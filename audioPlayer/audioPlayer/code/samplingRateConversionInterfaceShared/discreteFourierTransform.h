#pragma once
#include "fourierTransformBase.h"


namespace SamplingRate {
namespace Fourier {
class DiscreteTransform : public TransformBase {
private:
  double preCoefficient = 0;
  double postCoefficient = 0;
  std::vector<double> finalPcmDataArray;
  std::uint16_t inverseTransform(std::uint32_t postPcmDataFrameSize,
                                 const std::vector<double> &realInputArray,
                                 const std::vector<double> &imaginaryInputArray,
                                 std::vector<double> &realOutputArray);

public:
  std::uint16_t
  initialize(DataReadingMethod readingMethod, std::uint8_t channelQuantity,
             std::uint32_t preSamplingRate, std::uint32_t postSamplingRate,
             std::uint32_t criterionDataFrameSize, std::uint32_t quotient);
  std::uint16_t runConvertor(std::vector<std::int16_t> &pcmDataArray);
  std::uint16_t runConvertor(std::vector<float> &pcmDataArray);

  double getConvertedPcmOneSample(std::uint32_t frameIndex,
                                  std::uint8_t channelIndex) const;
};
} // namespace Fourier
} // namespace SamplingRate
