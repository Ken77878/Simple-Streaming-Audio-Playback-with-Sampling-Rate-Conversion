#include "samplingRateConvertorBase.h"

namespace SamplingRate {

std::uint32_t ConvertorBase::getPrePcmDataFrameSize() const noexcept {
  return prePcmDataFrameSize;
}
std::uint32_t ConvertorBase::getPostPcmDataFrameSize() const noexcept {
  return postPcmDataFrameSize;
}

} // namespace SamplingRate
