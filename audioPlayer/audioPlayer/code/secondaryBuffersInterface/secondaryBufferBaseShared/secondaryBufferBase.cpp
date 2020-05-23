#include "secondaryBufferBase.h"

namespace SecondaryBuffer {
CommonBase::CommonBase(std::uint8_t deviceChannelQuantity,
                       std::uint16_t deviceSamlingRate,
                       std::uint32_t framesPerPeriod,
                       DataReadingMethod dataReadingMethodFlag, bool loop)
    : deviceChannelQuantity(deviceChannelQuantity),
      framesPerPeriod(framesPerPeriod), postSamplingRate(deviceSamlingRate),
      dataReadingMethodFlag(dataReadingMethodFlag), loop(loop) {}

std::uint16_t CommonBase::readDataFromFile() { return 0; }

DataReadingMethod CommonBase::getDataReadingMethod() const noexcept {
  return dataReadingMethodFlag;
}

bool CommonBase::getPlayOn() const noexcept { return playOn; }
void CommonBase::setPlayOn(bool playOnFlag) noexcept {
  this->playOn = playOnFlag;
}

} // namespace SecondaryBuffer
