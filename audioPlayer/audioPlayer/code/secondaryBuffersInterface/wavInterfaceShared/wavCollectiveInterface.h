#pragma once

#include "wavInterfaceDLLheader.h"
#include "wavInterfaceBase.h"

namespace SecondaryBuffer {
namespace Wav {
class wavInterface_API CollectiveInterface : public FileBase {
private:
  std::uint16_t
  initializeSamplingRateConversion(std::uint32_t originalFrameSize);
  std::int16_t getDataFromVector(std::uint8_t channelIndex);

public:
  CollectiveInterface(std::uint8_t deviceChannelQuantity,
                      std::uint16_t deviceSamlingRate,
                      std::uint32_t framesPerPeriod,
                      DataReadingMethod dataReadingMethodFlag, bool loop);

  std::uint16_t initialize(std::TSTRING &filePath);
  std::int16_t getBufferData(std::uint8_t channelIndex);
  std::uint16_t finalize();
};
} // namespace Wav
} // namespace SecondaryBuffer
