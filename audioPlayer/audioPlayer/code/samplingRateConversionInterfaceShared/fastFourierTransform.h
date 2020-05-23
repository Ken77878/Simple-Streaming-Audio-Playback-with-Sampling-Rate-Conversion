#pragma once
#include "fourierTransformBase.h"
#include <array>
//
namespace SamplingRate {
namespace Fourier {
class FastTransform : public TransformBase {
private:
  double thetaAtNormal = 0;
  double thetaAtInverse = 0;
  std::array<std::uint32_t, 3> normalSpecialIndex = {0, 0, 0};
  std::array<std::uint32_t, 3> InverseSpecialIndex = {0, 0, 0};

  std::vector<double> wRealNormalValueArray;
  std::vector<double> wImaginaryNormalValueArray;
  std::vector<double> wRealInverseValueArray;
  std::vector<double> wImaginaryInverseValueArray;
  std::uint16_t normalTransform(std::uint32_t oneChannelBufferSize);

  std::uint16_t normalTransform2(std::uint32_t bufferFrameSize);
  std::uint16_t sortingAfterNormalTransform(std::uint32_t oneChannelBufferSize);

  std::uint16_t inverseTransform(std::uint32_t oneChannelBufferSize);
  std::uint16_t
  sortingAfterInverseTransform(std::vector<double> &pcmDataArray,
                               std::uint32_t oneChannelBufferSize);

  std::uint16_t conversionTask();

public:
  FastTransform();

  std::uint16_t
  initialize(DataReadingMethod readingMethod, std::uint8_t channelQuantity,
             std::uint32_t preSamplingRate, std::uint32_t postSamplingRate,
             std::uint32_t criterionDataFrameSize, std::uint32_t quotient);

  std::uint16_t runConvertor(std::vector<std::int16_t> &pcmDataArray);
  std::uint16_t runConvertor(std::vector<float> &pcmDataArray);

  double getConvertedPcmOneSample(std::uint32_t index,
                                  std::uint8_t channelQuantity) const;
};
} // namespace Fourier
} // namespace SamplingRate
