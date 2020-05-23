#pragma once
#include "samplingRateConversionInterfaceDLLheader.h"
#include <memory>
#include "samplingRateConvertorBase.h"
#include "pcmSampleTypeEnumeration.h"
#include "dataReadingMethod.h"
#include "noCopyAndMoveTemplate.h"

enum class SamplingRateConvertorType { FIR, FastFourier };

namespace SamplingRate {
class samplingRateConversionInterface_API ConvertorInterface
    : public NoCopyAndMove<ConvertorInterface> {
private:
  SamplingRateConvertorType convertorType = SamplingRateConvertorType::FIR;
  std::unique_ptr<ConvertorBase> convertorPtr;
  bool suitableForFastFourier(std::uint32_t preSamplingRate,
                              std::uint32_t postSamplingRate,
                              std::uint32_t &quotient);

public:
  std::uint16_t initialize(std::uint8_t channelQuantity,
                           DataReadingMethod readingMethod,

                           std::uint32_t preSamplingRate,
                           std::uint32_t postSamplingRate,
                           std::uint32_t criterionDataFrameSize);
  std::uint32_t getPrePcmDataArrayFrameSize() const noexcept;
  std::uint32_t getPrePcmDataFrameSize() const noexcept;
  std::uint32_t getPreExtraPcmDataFrameSize() const noexcept;
  std::uint32_t getPostExtraPcmDataFrameSize() const noexcept;

  std::uint16_t runConvertor(std::vector<std::int16_t> &pcmDataArray);
  std::uint16_t runConvertor(std::vector<float> &pcmDataArray);

  std::uint32_t getPostPcmDataFrameSize() const;
  void convertDoubleDataOfInt16bitToInt16bit(
      std::vector<std::int16_t> &pcm16bitDataArray);

  void convertDoubleDataOfFloatToInt16bit(
      std::vector<std::int16_t> &pcm16bitDataArray);

  double getConvertedPcmOneSample(std::uint32_t frameIndex,
                                  std::uint8_t channelIndex);
};

} // namespace SamplingRate
