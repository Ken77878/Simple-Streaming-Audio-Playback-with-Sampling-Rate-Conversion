#pragma once

#include <type_traits>
#include <cstring>

namespace SamplingRate {
namespace LowPass {
namespace FIRtemplate {
template <class T>
std::uint16_t upSamplingTask(std::uint8_t channelQuantity,
                             std::vector<T> &pcmDataArray,
                             std::uint32_t preCoefficient,

                             std::uint32_t prePcmDataFrameSize,
                             std::uint32_t middlePcmDataFrameSize

) {
  static_assert(std::is_integral<T>::value || std::is_floating_point<T>::value,
                "template parameter T must be integral or floating_point type");
  if (preCoefficient == 1)
    return 0;
  for (std::uint32_t u = prePcmDataFrameSize; u > 0; --u)
    for (std::uint32_t v = 0; v < channelQuantity; ++v) {
      pcmDataArray[preCoefficient * ((u - 1) * channelQuantity) + v] =
          pcmDataArray[(u - 1) * channelQuantity + v];
      for (std::uint32_t w = 1; w < preCoefficient; ++w)
        pcmDataArray[preCoefficient * ((u - 1) * channelQuantity) + v +
                     channelQuantity * w] = 0;
    }
  return 0;
}

template <class T>
std::uint16_t
runFilter(std::uint8_t channelQuantity, std::uint32_t middlePcmDataFrameSize,
          std::uint32_t delayUnitQuantity,
          std::vector<double> &coefficientArray, std::vector<T> &pcmDataArray,
          std::vector<double> &finalPcmDataArray,
          std::uint32_t preCoefficient) {

  static_assert(std::is_integral<T>::value || std::is_floating_point<T>::value,
                "template parameter T must be integral or floating_point type");

  for (std::uint32_t u = 0; u < channelQuantity; ++u)
    for (std::uint32_t v = delayUnitQuantity; v < middlePcmDataFrameSize; ++v) {
      for (std::uint32_t w = 0; w <= delayUnitQuantity; ++w) {
        if (v >= w) {
          finalPcmDataArray[v * channelQuantity + u] +=
              (coefficientArray[w] *
               pcmDataArray[(v - w) * channelQuantity + u]);
        }
      }
      finalPcmDataArray[v * channelQuantity + u] *= preCoefficient;
    }
  return 0;
}

} // namespace FIRtemplate
} // namespace LowPass
} // namespace SamplingRate
