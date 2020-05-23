#pragma once
#include "samplingRateConvertorBase.h"
#include "dataReadingMethod.h"

namespace SamplingRate {
namespace LowPass {
class FIRfilter : public ConvertorBase {
private:
  std::uint32_t preExtraPcmDataFrameSize = 0;
  std::uint32_t postExtraPcmDataFrameSize = 0;
  std::vector<double> finalPcmDataArray;
  std::vector<double> coefficientArray;
  double edgeFrequencyNomalization = 0;
  // must be multiple of four
  std::uint32_t delayUnitQuantity = 84;
  std::uint32_t middlePcmDataFrameSize = 0;
  std::uint32_t postCoefficient = 1;
  std::uint32_t preCoefficient = 1;
  void setDelayUnitQuantity(double transitionBandPassNomalization);
  std::uint16_t initializeCommonTask();
  std::uint16_t downSamplingTask(std::vector<double> &pcmDataArray);
  void convertDoubleDataOfInt16bitToInt16bit(
      std::vector<std::int16_t> &pcm16bitDataArray);

public:
  FIRfilter();
  std::uint16_t initialize(DataReadingMethod readingMethod,
                           std::uint8_t channelQuantity,
                           std::uint32_t preSamplingRate,
                           std::uint32_t postSamplingRate,
                           std::uint32_t criterionDataFrameSize);

  std::uint32_t getPrePcmDataArrayFrameSize() const noexcept;
  std::uint32_t getPreExtraPcmDataFrameSize() const noexcept;
  std::uint32_t getMiddlePcmDataFrameSize() const noexcept;

  std::uint32_t getPostExtraPcmDataFrameSize() const noexcept;
  void convertDoubleDataOfFloatToInt16bit(
      std::vector<std::int16_t> &pcm16bitDataArray);
  std::uint16_t runConvertor(std::vector<std::int16_t> &pcmDataArray);
  std::uint16_t runConvertor(std::vector<float> &pcmDataArray);
  double getConvertedPcmOneSample(std::uint32_t frameIndex,
                                  std::uint8_t channelIndex) const;
};
} // namespace LowPass

} // namespace SamplingRate
