
#pragma once
#include <cstdint>
#include <vector>

namespace SamplingRate {
namespace Fourier {
namespace DiscreteTemplate {
template <typename T>
std::uint16_t
normalTransform(std::uint8_t channelQuantity, std::uint32_t prePcmDataFrameSize,
                double preCoefficient, const std::vector<T> &pcmDataArray,
                std::vector<double> &realDataArray,
                std::vector<double> &imaginaryDataArray) {
  std::uint32_t dataIndex1 = 0;
  std::uint32_t dataIndex2 = 0;

  for (uint32_t u = 0; u < channelQuantity; ++u) {
    {
      dataIndex1 = u;
      dataIndex2 = u;
      realDataArray[dataIndex1] += pcmDataArray[dataIndex2];
      for (std::uint32_t m = 1; m < prePcmDataFrameSize; ++m) {
        dataIndex2 = m * channelQuantity + u;
        realDataArray[dataIndex1] += pcmDataArray[dataIndex2];
      }
    }
    for (std::uint32_t k = 1; k < prePcmDataFrameSize; ++k) {
      dataIndex1 = k * channelQuantity + u;
      dataIndex2 = u;
      realDataArray[dataIndex1] += pcmDataArray[dataIndex2];
      for (std::uint32_t m = 1; m < prePcmDataFrameSize; ++m) {
        dataIndex2 = m * channelQuantity + u;
        realDataArray[dataIndex1] +=
            pcmDataArray[dataIndex2] * cos(preCoefficient * k * m);
        imaginaryDataArray[dataIndex1] +=
            -pcmDataArray[dataIndex2] * sin(preCoefficient * k * m);
      }
    }
  }
  return 0;
}
} // namespace DiscreteTemplate
} // namespace Fourier
} // namespace SamplingRate
