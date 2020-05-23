#pragma once
#include "samplingRateConversionInterfaceDLLheader.h"
#include <cstdint>
#include <vector>
#include "dataReadingMethod.h"
#include "noCopyAndMoveTemplate.h"

namespace SamplingRate {
class samplingRateConversionInterface_API ConvertorBase
    : public NoCopyAndMove<ConvertorBase> {
private:
protected:
  DataReadingMethod readingMethod = DataReadingMethod::successive;
  std::uint8_t channelQuantity = 0;
  std::uint32_t prePcmDataFrameSize = 0;
  std::uint32_t postPcmDataFrameSize = 0;

public:
  virtual std::uint32_t getPrePcmDataArrayFrameSize() const noexcept = 0;
  std::uint32_t getPrePcmDataFrameSize() const noexcept;
  virtual std::uint16_t
  runConvertor(std::vector<std::int16_t> &pcmDataArray) = 0;
  virtual std::uint16_t runConvertor(std::vector<float> &pcmDataArray) = 0;
  virtual void convertDoubleDataOfInt16bitToInt16bit(
      std::vector<std::int16_t> &pcm16bitDataArray) = 0;
  virtual void convertDoubleDataOfFloatToInt16bit(
      std::vector<std::int16_t> &pcm16bitDataArray) = 0;
  virtual std::uint32_t getPostPcmDataFrameSize() const noexcept;

  virtual double getConvertedPcmOneSample(std::uint32_t frameIndex,
                                          std::uint8_t channelIndex) const = 0;
  virtual ~ConvertorBase() {}
};
} // namespace SamplingRate
