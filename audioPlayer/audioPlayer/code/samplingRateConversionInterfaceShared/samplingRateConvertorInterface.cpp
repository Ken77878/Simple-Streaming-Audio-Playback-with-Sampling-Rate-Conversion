#include "samplingRateConvertorInterface.h"
#include "lowPassFIRfilter.h"
#include "fastFourierTransform.h"
#include "discreteFourierTransform.h"
#include "debugMacro.h"

namespace SamplingRate

{
bool ConvertorInterface::suitableForFastFourier(std::uint32_t preSamplingRate,
                                                std::uint32_t postSamplingRate,
                                                std::uint32_t &quotient) {

  if (preSamplingRate > postSamplingRate) {
    if (!(preSamplingRate % postSamplingRate))
      quotient = preSamplingRate / postSamplingRate;
  } else if (preSamplingRate < postSamplingRate) {
    if (!(postSamplingRate % preSamplingRate))
      quotient = postSamplingRate / preSamplingRate;
  } else
    return false;
  std::uint32_t count = 0;
  std::uint32_t quotientTemp = quotient;
  for (;;) {
    if (!(quotientTemp >>= 1))
      break;
    ++count;
  }
  if ((1 << count) == quotient)
    return true;
  return false;
}

std::uint16_t ConvertorInterface::initialize(
    std::uint8_t channelQuantity, DataReadingMethod readingMethod,
    std::uint32_t preSamplingRate, std::uint32_t postSamplingRate,
    std::uint32_t criterionDataFrameSize) {
  if (readingMethod == DataReadingMethod::collective) {
    std::uint32_t quotient = 0;
    if (!suitableForFastFourier(preSamplingRate, postSamplingRate, quotient)) {
      DEBUG_PRINT(TEXT("FIR conversion\n"));
      convertorType = SamplingRateConvertorType::FIR;
      convertorPtr.reset(new LowPass::FIRfilter());
      if (dynamic_cast<LowPass::FIRfilter *>(convertorPtr.get())
              ->initialize(readingMethod, channelQuantity, preSamplingRate,
                           postSamplingRate, criterionDataFrameSize))
        return 1;

    } else {
      DEBUG_PRINT(TEXT("fast fourier conversion\n"));
      convertorType = SamplingRateConvertorType::FastFourier;
      convertorPtr.reset(new Fourier::FastTransform());
      if (dynamic_cast<Fourier::FastTransform *>(convertorPtr.get())
              ->initialize(readingMethod, channelQuantity, preSamplingRate,
                           postSamplingRate, criterionDataFrameSize, quotient))
        return 1;
    }

  } else if (readingMethod == DataReadingMethod::successive) {
    DEBUG_PRINT(TEXT("successive conversion\n"));
    convertorPtr.reset(new LowPass::FIRfilter());
    dynamic_cast<LowPass::FIRfilter *>(convertorPtr.get())
        ->initialize(readingMethod, channelQuantity, preSamplingRate,
                     postSamplingRate, criterionDataFrameSize);
  }
  return 0;
}
std::uint32_t ConvertorInterface::getPrePcmDataArrayFrameSize() const noexcept {
  return convertorPtr->getPrePcmDataArrayFrameSize();
}

std::uint32_t ConvertorInterface::getPrePcmDataFrameSize() const noexcept {
  return convertorPtr->getPrePcmDataFrameSize();
}

std::uint32_t ConvertorInterface::getPreExtraPcmDataFrameSize() const noexcept {
  if (convertorType == SamplingRateConvertorType::FIR) {
    return dynamic_cast<LowPass::FIRfilter *>(convertorPtr.get())
        ->getPreExtraPcmDataFrameSize();
  } else {
    printf("programming miss: getPreExtraPcmDataFrameSize() ");
    exit(701);
  }
}
std::uint32_t ConvertorInterface::getPostExtraPcmDataFrameSize() const
    noexcept {
  if (convertorType == SamplingRateConvertorType::FIR) {
    return dynamic_cast<LowPass::FIRfilter *>(convertorPtr.get())
        ->getPostExtraPcmDataFrameSize();
  } else {
    printf("programming miss: getPreExtraPcmDataFrameSize() ");
    exit(701);
  }
}

std::uint16_t
ConvertorInterface::runConvertor(std::vector<std::int16_t> &pcmDataArray) {
  return convertorPtr->runConvertor(pcmDataArray);
}

std::uint16_t
ConvertorInterface::runConvertor(std::vector<float> &pcmDataArray) {
  return convertorPtr->runConvertor(pcmDataArray);
}
std::uint32_t ConvertorInterface::getPostPcmDataFrameSize() const {
  return convertorPtr->getPostPcmDataFrameSize();
}
void ConvertorInterface::convertDoubleDataOfInt16bitToInt16bit(
    std::vector<std::int16_t> &pcm16bitDataArray) {
  convertorPtr->convertDoubleDataOfInt16bitToInt16bit(pcm16bitDataArray);
}

void ConvertorInterface::convertDoubleDataOfFloatToInt16bit(
    std::vector<std::int16_t> &pcm16bitDataArray) {
  convertorPtr->convertDoubleDataOfFloatToInt16bit(pcm16bitDataArray);
}

double ConvertorInterface::getConvertedPcmOneSample(std::uint32_t frameIndex,
                                                    std::uint8_t channelIndex) {
  return convertorPtr->getConvertedPcmOneSample(frameIndex, channelIndex);
}
} // namespace SamplingRate
