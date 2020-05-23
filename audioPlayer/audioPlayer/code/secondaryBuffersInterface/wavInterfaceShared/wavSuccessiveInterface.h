#pragma once

#include "wavInterfaceDLLheader.h"
#include "wavInterfaceBase.h"

namespace SecondaryBuffer {
namespace Wav {
class wavInterface_API SuccessiveInterface : public FileBase {
private:
  FILE *fp = nullptr;
  // for low pass filter
  std::uint32_t preExtraPcmDataByteSize = 0;
  std::uint32_t postExtraPcmDataFrameSize = 0;
  std::uint32_t postPcmDataFrameSizeWithExtra = 0;
  std::uint32_t bufferByteSize = 0;
  std::uint32_t currentOffset = 0;
  std::uint16_t initializeSamplingRateConversion();
  std::uint16_t readFileInRecursionCall(std::uint32_t bufferStartIndex,
                                        std::uint32_t allReadByteSize,
                                        FILE *stream);
  std::uint16_t readPreExtraPcmDataAndMainData();
  std::int16_t getDataFromVector(std::uint8_t channelIndex);

public:
  SuccessiveInterface(std::uint8_t deviceChannelQuantity,
                      std::uint16_t deviceSamlingRate,
                      std::uint32_t framesPerPeriod,
                      DataReadingMethod dataReadingMethodFlag, bool loop);

  std::uint16_t initialize(std::TSTRING &filePath);
  std::uint16_t readDataFromFile();
  std::int16_t getBufferData(std::uint8_t channelIndex);
  std::uint16_t finalize();
};
} // namespace Wav
} // namespace SecondaryBuffer
