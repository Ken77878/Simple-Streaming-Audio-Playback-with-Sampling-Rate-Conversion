#pragma once
#include "wavInterfaceDLLheader.h"
#include "secondaryBufferBase.h"
#include "memoryMappingInterface.h"
namespace SecondaryBuffer {
namespace Wav {
class wavInterface_API FileBase : public CommonBase {
private:
protected:
  std::vector<std::int16_t> pcmInt16bitDataArray;
  std::uint32_t postPcmDataFrameSize = 0;
  std::uint32_t dataOffset = 0;
  std::uint32_t dataByteSize = 0;

  std::unique_ptr<MyUtility::MemoryMappingInterface> memoryMappingObj;
  std::int16_t *mappedWavDataTopPtr = nullptr;
  std::uint16_t initializeFreadMode(FILE *fp, std::uint32_t dataOffset);
  std::uint16_t initializeFileMapping(std::TSTRING &filePath,
                                      std::uint32_t &dataByteSize,
                                      std::uint32_t &dataOffset);

  std::uint16_t initialize(FILE *fp, std::uint32_t &dataByteSize,
                           std::uint32_t &dataOffset, std::uint8_t &channels,
                           std::uint32_t &preSamplingRate);
  std::uint16_t freadWrap(std::int16_t *bufferPtr, size_t readedbyteSize,
                          FILE *stream);
  std::int16_t getDataFromMmapMemory(std::uint8_t channelIndex);
  virtual std::int16_t getDataFromVector(std::uint8_t channelIndex) = 0;
  std::uint16_t finalizeMmap();

public:
  FileBase(std::uint8_t deviceChannelQuantity, std::uint16_t deviceSamlingRate,
           std::uint32_t framesPerPeriod,
           DataReadingMethod dataReadingMethodFlag, bool loop);
};
} // namespace Wav
} // namespace SecondaryBuffer
