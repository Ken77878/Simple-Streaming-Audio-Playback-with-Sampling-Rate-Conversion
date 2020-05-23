#include "wavInterfaceBase.h"
#include "checkWav.h"

#include "fileManagement.h"
#include "systemErrorInterface.h"
#include "debugMacro.h"

namespace SecondaryBuffer {
namespace Wav {
FileBase::FileBase(std::uint8_t deviceChannelQuantity,
                   std::uint16_t deviceSamlingRate,
                   std::uint32_t framesPerPeriod,
                   DataReadingMethod dataReadingMethodFlag, bool loop)
    : CommonBase(deviceChannelQuantity, deviceSamlingRate, framesPerPeriod,
                 dataReadingMethodFlag, loop) {}

std::uint16_t FileBase::initializeFreadMode(FILE *fp,
                                            std::uint32_t dataOffset) {
  DEBUG_PRINT(TEXT("Wav: initializeFreadMode\n"));
  if (MyUtility::fseekWrap(fp, dataOffset, SEEK_SET))
    return 1;
  return 0;
}

std::uint16_t FileBase::initializeFileMapping(std::TSTRING &filePath,
                                              std::uint32_t &dataByteSize,
                                              std::uint32_t &dataOffset) {
  DEBUG_PRINT(TEXT("fileMapping mode\n"));
  memoryMappingObj.reset(new MyUtility::MemoryMappingInterface);

  if (memoryMappingObj->initialize(filePath))
    return 1;

  mappedWavDataTopPtr = reinterpret_cast<std::int16_t *>(const_cast<void *>(
                            memoryMappingObj->getMmapDataTopPtr())) +
                        (dataOffset >> 1);
  return 0;
}

std::uint16_t FileBase::initialize(FILE *fp, std::uint32_t &dataByteSize,
                                   std::uint32_t &dataOffset,
                                   std::uint8_t &channels,
                                   std::uint32_t &samplingRate) {
  if (checkWav(fp, dataByteSize, dataOffset, channels, samplingRate))
    return 1;
  return 0;
}
std::uint16_t FileBase::freadWrap(std::int16_t *bufferPtr,
                                  size_t readedByteSize, FILE *stream) {
  if (MyUtility::freadWrap(bufferPtr, 1, readedByteSize, stream))
    return 1;
  return 0;
}

std::int16_t FileBase::getDataFromMmapMemory(std::uint8_t channelIndex) {
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
  if (channelIndex <= (fileChannelQuantity - 1))
    return *(mappedWavDataTopPtr +
             currentReadFrameIndexTemp * fileChannelQuantity + channelIndex);

  else {
    if (channelIndex == 1)
      return *(mappedWavDataTopPtr +
               currentReadFrameIndexTemp * fileChannelQuantity);

    else
      return 0;
  }
}
std::uint16_t FileBase::finalizeMmap() {
  DEBUG_PRINT(TEXT("finalizeMmap()\n"));
  if (memoryMappingObj->finalize())
    return 1;

  return 0;
}

} // namespace Wav
} // namespace SecondaryBuffer
