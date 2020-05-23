
#include "wavSuccessiveInterface.h"
#include "fileManagement.h"
#include <cstring>
#include "debugMacro.h"

namespace SecondaryBuffer {
namespace Wav {
SuccessiveInterface::SuccessiveInterface(
    std::uint8_t deviceChannelQuantity, std::uint16_t deviceSamlingRate,
    std::uint32_t framesPerPeriod, DataReadingMethod dataReadingMethodFlag,
    bool loop)
    : FileBase(deviceChannelQuantity, deviceSamlingRate, framesPerPeriod,
               dataReadingMethodFlag, loop) {}

std::uint16_t SuccessiveInterface::initializeSamplingRateConversion() {
  if (samplingRateConvertor.initialize(
          fileChannelQuantity, dataReadingMethodFlag,

          preSamplingRate, postSamplingRate, framesPerPeriod))
    return 1;

  pcmInt16bitDataArray.resize(
      samplingRateConvertor.getPrePcmDataArrayFrameSize() *
      fileChannelQuantity);

  return 0;
}

std::uint16_t SuccessiveInterface::initialize(std::TSTRING &filePath) {

  if (MyUtility::fopenWrap(fp, filePath))
    return 1;

  if (FileBase::initialize(fp, dataByteSize, dataOffset, fileChannelQuantity,
                           preSamplingRate))
    return 1;
  // wavDataSize < 4GB

  if (preSamplingRate == postSamplingRate) {
    if (MyUtility::fcloseWrap(fp))
      return 1;
    if (initializeFileMapping(filePath, dataByteSize, dataOffset))
      return 1;
    postPcmDataFrameSize = (dataByteSize >> 1) / fileChannelQuantity;
  } else {
    if (initializeFreadMode(fp, dataOffset))
      return 1;

    printf("initializeSamplingRateConversion");

    if (initializeSamplingRateConversion())
      return 1;

    bufferByteSize = sizeof(std::int16_t) *
                     samplingRateConvertor.getPrePcmDataFrameSize() *
                     fileChannelQuantity;
    preExtraPcmDataByteSize =
        samplingRateConvertor.getPreExtraPcmDataFrameSize() *
        sizeof(std::int16_t) * fileChannelQuantity;
    postExtraPcmDataFrameSize =
        samplingRateConvertor.getPostExtraPcmDataFrameSize();

    currentReadFrameIndex = postExtraPcmDataFrameSize;
    postPcmDataFrameSizeWithExtra = framesPerPeriod + postExtraPcmDataFrameSize;
  }

  DEBUG_PRINT(TEXT("SuccessiveInterface::initialize normally ended.\n"));

  return 0;
}

std::uint16_t
SuccessiveInterface::readFileInRecursionCall(std::uint32_t bufferStartIndex,
                                             std::uint32_t allReadByteSize,
                                             FILE *stream) {
  DEBUG_PRINT_ARGS(TEXT("allReadByteSize: %u\n"), allReadByteSize);
  DEBUG_PRINT_ARGS(TEXT("currentOffset: %u\n"), currentOffset);
  DEBUG_PRINT_ARGS(TEXT("bufferStartIndex: %u\n"), bufferStartIndex);

  if ((currentOffset + allReadByteSize) >= dataByteSize) {
    std::uint32_t remainingDataByteSize = dataByteSize - currentOffset;
    if (loop) {

      if (freadWrap(&pcmInt16bitDataArray[bufferStartIndex],
                    remainingDataByteSize, fp))
        return 1;

      if (fseek(fp, dataOffset, SEEK_SET))
        return 1;
      currentOffset = 0;
      if (readFileInRecursionCall(bufferStartIndex +
                                      (remainingDataByteSize >> 1),
                                  allReadByteSize - remainingDataByteSize, fp))
        return 1;
    } else {
      if (freadWrap(&pcmInt16bitDataArray[bufferStartIndex],
                    remainingDataByteSize, stream))
        return 1;

      memset(&pcmInt16bitDataArray[bufferStartIndex + remainingDataByteSize /
                                                          sizeof(std::int16_t)],
             0, bufferByteSize - remainingDataByteSize);
      playOn = false;
      currentOffset = 0;
    }
  } else {
    if (freadWrap(&pcmInt16bitDataArray[bufferStartIndex], allReadByteSize,
                  stream))
      return 1;
    currentOffset += allReadByteSize;
  }
  return 0;
}

std::uint16_t SuccessiveInterface::readPreExtraPcmDataAndMainData() {
  DEBUG_PRINT_ARGS(TEXT("file offset: %d\n"), ftell(fp));
  DEBUG_PRINT_ARGS(TEXT("preExtraPcmDataByteSize: %u\n"),
                   preExtraPcmDataByteSize);
  DEBUG_PRINT_ARGS(TEXT("bufferByteSize: %u\n"), bufferByteSize);

  if (currentOffset >= preExtraPcmDataByteSize) {
    if (fseek(fp, -static_cast<int32_t>(preExtraPcmDataByteSize), SEEK_CUR)) {

      return 1;
    }

    currentOffset -= preExtraPcmDataByteSize;
    if (readFileInRecursionCall(0, bufferByteSize, fp))
      return 1;
  } else {
    std::uint32_t zeroPaddingByteSize = preExtraPcmDataByteSize - currentOffset;
    std::memset(&pcmInt16bitDataArray[0], 0, zeroPaddingByteSize);
    if (fseek(fp, dataOffset, SEEK_SET))
      return 1;
    currentOffset = 0;

    if (readFileInRecursionCall((zeroPaddingByteSize >> 1),
                                bufferByteSize - zeroPaddingByteSize, fp))
      return 1;
  }
  return 0;
}

std::uint16_t SuccessiveInterface::readDataFromFile() {
  if (preSamplingRate == postSamplingRate)
    return 0;
  if (readPreExtraPcmDataAndMainData())
    return 1;
  if (samplingRateConvertor.runConvertor(pcmInt16bitDataArray))
    return 1;
  currentReadFrameIndex = postExtraPcmDataFrameSize;
  return 0;
}

std::int16_t SuccessiveInterface::getDataFromVector(std::uint8_t channelIndex) {

  std::uint32_t currentReadFrameIndexTemp = currentReadFrameIndex;

  if (channelIndex == (deviceChannelQuantity - 1))
    ++currentReadFrameIndex;
  if (channelIndex <= (fileChannelQuantity - 1)) {
    return pcmInt16bitDataArray[currentReadFrameIndexTemp *
                                    fileChannelQuantity +
                                channelIndex];
  } else {
    if (channelIndex == 1)
      return pcmInt16bitDataArray[currentReadFrameIndexTemp *
                                  fileChannelQuantity];
    else
      return 0;
  }
}

std::int16_t SuccessiveInterface::getBufferData(std::uint8_t channelIndex) {
  if (preSamplingRate == postSamplingRate)
    return getDataFromMmapMemory(channelIndex);
  else
    return getDataFromVector(channelIndex);
}

std::uint16_t SuccessiveInterface::finalize() {
  if (preSamplingRate == postSamplingRate) {
    if (finalizeMmap())
      return 1;
  } else {
    if (MyUtility::fcloseWrap(fp))
      return 1;
  }
  return 0;
}

} // namespace Wav
} // namespace SecondaryBuffer
