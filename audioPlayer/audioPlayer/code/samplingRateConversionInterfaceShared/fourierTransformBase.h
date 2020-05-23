#pragma once
#include "samplingRateConvertorBase.h"

namespace SamplingRate {
namespace Fourier {

class TransformBase : public ConvertorBase {
protected:
  std::uint32_t postPcmDataFrameSizeWithoutPadding = 0;
  std::vector<double> realDataArray;
  std::vector<double> imaginaryDataArray;
  double convertedDataCoefficient = 0;
  virtual std::uint16_t
  initialize(DataReadingMethod readingMethod, std::uint8_t channelQuantity,
             std::uint32_t preSamplingRate, std::uint32_t postSamplingRate,
             std::uint32_t criterionDataFrameSize, std::uint32_t quotient);
  std::uint16_t upOrDownSamplingTask();
  void convertDoubleDataOfInt16bitToInt16bit(
      std::vector<std::int16_t> &pcm16bitDataArray);

public:
  std::uint32_t getPrePcmDataArrayFrameSize() const noexcept;
  std::uint32_t getPostPcmDataFrameSize() const noexcept;
  void convertDoubleDataOfFloatToInt16bit(
      std::vector<std::int16_t> &pcm16bitDataArray);
};

} // namespace Fourier
} // namespace SamplingRate
