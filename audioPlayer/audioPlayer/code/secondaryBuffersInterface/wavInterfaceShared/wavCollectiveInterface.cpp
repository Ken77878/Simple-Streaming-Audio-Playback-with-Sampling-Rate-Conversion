#include "wavCollectiveInterface.h"
#include "fileManagement.h"
#include "debugMacro.h"

namespace SecondaryBuffer {
namespace Wav {
CollectiveInterface::CollectiveInterface(
    std::uint8_t deviceChannelQuantity, std::uint16_t deviceSamlingRate,
    std::uint32_t framesPerPeriod, DataReadingMethod dataReadingMethodFlag,
    bool loop)
    : FileBase(deviceChannelQuantity, deviceSamlingRate, framesPerPeriod,
               dataReadingMethodFlag, loop) {}

std::uint16_t CollectiveInterface::initializeSamplingRateConversion(
    std::uint32_t originalPcmDataFrameSize) {
  if (samplingRateConvertor.initialize(
          fileChannelQuantity, dataReadingMethodFlag,

          preSamplingRate, postSamplingRate, originalPcmDataFrameSize))
    return 1;
  pcmInt16bitDataArray.resize(
      samplingRateConvertor.getPrePcmDataArrayFrameSize() *
      fileChannelQuantity);
  return 0;
}

std::uint16_t CollectiveInterface::initialize(std::TSTRING &filePath) {
  FILE *fp = nullptr;

  if (MyUtility::fopenWrap(fp, filePath))
    return 1;
  if (FileBase::initialize(fp, dataByteSize, dataOffset, fileChannelQuantity,
                           preSamplingRate))
    return 1;
  // wavDataSize < 4GB
  std::uint32_t originalPcmDataFrameSize =
      (dataByteSize >> 1) / fileChannelQuantity;

  if (preSamplingRate == postSamplingRate) {
    if (initializeFileMapping(filePath, dataByteSize, dataOffset))
      return 1;
    postPcmDataFrameSize = originalPcmDataFrameSize;

  } else

  {
    if (initializeFreadMode(fp, dataOffset))
      return 1;

    if (initializeSamplingRateConversion(originalPcmDataFrameSize)) {
      fprintf(stderr, "initializeSamplingRateConversion: error occurred.\n");
      return 1;
    }

    freadWrap(&pcmInt16bitDataArray[0], dataByteSize, fp);
    if (samplingRateConvertor.runConvertor(pcmInt16bitDataArray))
      return 1;
    samplingRateConvertor.convertDoubleDataOfInt16bitToInt16bit(
        pcmInt16bitDataArray);
    postPcmDataFrameSize = samplingRateConvertor.getPostPcmDataFrameSize();
  }

  if (MyUtility::fcloseWrap(fp))
    return 1;
  DEBUG_PRINT(TEXT("CollectiveWav::initialize normally ended.\n"));

  return 0;
}

std::int16_t CollectiveInterface::getDataFromVector(std::uint8_t channelIndex) {
  if (!playOn)
    return 0;

  std::uint32_t currentReadFrameIndexTemp = currentReadFrameIndex;
  if (channelIndex == (deviceChannelQuantity - 1))
    ++currentReadFrameIndex;
  if (currentReadFrameIndex == postPcmDataFrameSize) {
    currentReadFrameIndex = 0;
    if (!loop) {
      DEBUG_PRINT(TEXT("Audio File was all read. To play end\n"));
      playOn = false;
    }
  }
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
std::int16_t CollectiveInterface::getBufferData(std::uint8_t channelIndex) {
  if (!playOn)
    return 0;
  if (preSamplingRate == postSamplingRate)
    return getDataFromMmapMemory(channelIndex);
  else
    return getDataFromVector(channelIndex);
}
std::uint16_t CollectiveInterface::finalize() {
  if (preSamplingRate == postSamplingRate)
    if (finalizeMmap())
      return 1;
  return 0;
}
} // namespace Wav
} // namespace SecondaryBuffer
