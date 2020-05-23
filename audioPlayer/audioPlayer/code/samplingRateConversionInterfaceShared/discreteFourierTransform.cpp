#include "discreteFourierTransform.h"
#include "m_piConstants.h"
#include <cmath>
#include <cstdio>
#include "discreteFourierTransformTemplate.h"
#include <cstring>

namespace SamplingRate {
namespace Fourier {

std::uint16_t DiscreteTransform::initialize(
    DataReadingMethod readingMethod, std::uint8_t channelQuantity,
    std::uint32_t preSamplingRate, std::uint32_t postSamplingRate,
    std::uint32_t criterionDataFrameSize, std::uint32_t quotient) {

  if (TransformBase::initialize(readingMethod, channelQuantity, preSamplingRate,
                                postSamplingRate, criterionDataFrameSize,
                                quotient))
    return 1;

  prePcmDataFrameSize = 8;
  postPcmDataFrameSize = 8;
  preCoefficient = 2 * M_PI / static_cast<double>(prePcmDataFrameSize);
  postCoefficient = 2 * M_PI / (double)postPcmDataFrameSize;
  finalPcmDataArray.resize(postPcmDataFrameSize * channelQuantity);
  return 0;
}

std::uint16_t DiscreteTransform::inverseTransform(
    std::uint32_t postPcmDataFrameSize,
    const std::vector<double> &realInputArray,
    const std::vector<double> &imaginaryInputArray,
    std::vector<double> &realOutputArray) {

  std::uint32_t dataIndex1 = 0;

  double tempValue = 0;
  for (std::uint32_t u = 0; u < channelQuantity; ++u)
    for (std::uint32_t m = 0; m < postPcmDataFrameSize; ++m) {
      tempValue = 0;
      for (std::uint32_t k = 0; k < postPcmDataFrameSize; ++k) {
        dataIndex1 = k * channelQuantity + u;
        tempValue +=
            realInputArray[dataIndex1] * cos(postCoefficient * k * m) -
            imaginaryInputArray[dataIndex1] * sin(postCoefficient * k * m);
      }
      realOutputArray[m * channelQuantity + u] =
          tempValue / postPcmDataFrameSize * convertedDataCoefficient;
    }
  return 0;
}

std::uint16_t
DiscreteTransform::runConvertor(std::vector<std::int16_t> &pcmDataArray) {
  printf("discreteTransform  normal start");
  std::memset(&realDataArray[0], 0, sizeof(double) * realDataArray.size());
  std::memset(&imaginaryDataArray[0], 0,
              sizeof(double) * imaginaryDataArray.size());
  if (DiscreteTemplate::normalTransform(channelQuantity, prePcmDataFrameSize,
                                        preCoefficient, pcmDataArray,
                                        realDataArray, imaginaryDataArray))
    return 1;

  upOrDownSamplingTask();

  if (inverseTransform(postPcmDataFrameSize, realDataArray, imaginaryDataArray,
                       finalPcmDataArray))
    return 1;

  return 0;
}

std::uint16_t
DiscreteTransform::runConvertor(std::vector<float> &pcmDataArray) {

  if (DiscreteTemplate::normalTransform(channelQuantity, prePcmDataFrameSize,
                                        preCoefficient, pcmDataArray,
                                        realDataArray, imaginaryDataArray))
    return 1;
  upOrDownSamplingTask();
  if (inverseTransform(postPcmDataFrameSize, realDataArray, imaginaryDataArray,
                       finalPcmDataArray))
    return 1;
  return 0;
}

double
DiscreteTransform::getConvertedPcmOneSample(std::uint32_t frameIndex,
                                            std::uint8_t channelIndex) const {
  return finalPcmDataArray[frameIndex * channelQuantity + channelIndex];
}

} // namespace Fourier
} // namespace SamplingRate
