#include "lowPassFIRfilter.h"
#include "m_piConstants.h"
#include <cmath>
#include <limits>
#include <cstdio>
#include "greatestCommonDevisorCalculation.h"
#include "lowPassFIRfilterTemplate.h"
#include "debugMacro.h"

namespace SamplingRate {
namespace LowPass {
FIRfilter::FIRfilter() {}
static std::uint16_t
hanningWindowTask1(std::vector<double> &coefficientArray,
                   std::uint32_t delayUnitQuantityPlusOne) {
  coefficientArray.clear();
  coefficientArray.reserve(delayUnitQuantityPlusOne);
  for (std::uint32_t u = 0; u < delayUnitQuantityPlusOne; ++u)
    coefficientArray.push_back(
        0.5 - 0.5 * std::cos(2 * M_PI * u / (delayUnitQuantityPlusOne - 1)));
  return 0;
}

static std::uint16_t
hanningWindowTask2(std::vector<double> &coefficientArray,
                   std::uint32_t delayUnitQuantityPlusOne) {
  coefficientArray.clear();
  coefficientArray.reserve(delayUnitQuantityPlusOne);
  double tempValue = 2 * M_PI / delayUnitQuantityPlusOne;
  if (!(delayUnitQuantityPlusOne & 1)) {
    for (std::uint32_t u = 0; u < delayUnitQuantityPlusOne; ++u)
      coefficientArray.push_back(0.5 - 0.5 * std::cos(u * tempValue));
  } else {
    for (std::uint32_t u = 0; u < delayUnitQuantityPlusOne; ++u)
      coefficientArray.push_back(0.5 - 0.5 * std::cos((u + 0.5) * tempValue));
  }
  return 0;
}

static double denormalizedSinc(double x) noexcept {
  if (x)
    return std::sin(x) / (x);
  else
    return 1;
}

static double normalizedSinc(double x) noexcept {
  if (x)
    return std::sin(M_PI * x) / (M_PI * x);
  else
    return 1;
}

static std::uint16_t makeCoefficientArray(std::vector<double> &coefficientArray,
                                          double edgeFrequencyNomalization,
                                          std::uint32_t delayUnitQuantity) {
  double tempValue1 = 2 * edgeFrequencyNomalization * M_PI;
  double tempValue2 = edgeFrequencyNomalization * 2;

  for (std::uint32_t u = 0; u <= delayUnitQuantity; ++u) {

    coefficientArray[u] *=
        tempValue2 *
        denormalizedSinc(tempValue1 *
                         (static_cast<int32_t>(u) -
                          static_cast<std::int32_t>((delayUnitQuantity >> 1))));
  }
  return 0;
}

// transitionBandPassNomalization: 0.025 - 0.05
void FIRfilter::setDelayUnitQuantity(double transitionBandPassNomalization) {
  if (!transitionBandPassNomalization) {
    delayUnitQuantity = 0;
    return;
  }

  delayUnitQuantity = static_cast<uint32_t>(
      std::floor((3.1 / transitionBandPassNomalization) + 0.5) - 1);
  if (delayUnitQuantity & 1) {
    // make delayUnitQuantityPlusOne odd number
    ++delayUnitQuantity;
  }
}

std::uint16_t FIRfilter::initializeCommonTask() {
  hanningWindowTask2(coefficientArray, delayUnitQuantity + 1);
  makeCoefficientArray(coefficientArray, edgeFrequencyNomalization,
                       delayUnitQuantity);
  return 0;
}

std::uint16_t FIRfilter::initialize(DataReadingMethod readingMethod,
                                    std::uint8_t channelQuantity,
                                    std::uint32_t preSamplingRate,
                                    std::uint32_t postSamplingRate,
                                    std::uint32_t criterionDataFrameSize) {
  this->readingMethod = readingMethod;
  this->channelQuantity = channelQuantity;
  std::uint32_t greatestCommonDevisor =
      getGreatestCommonDevisorInRecursion(preSamplingRate, postSamplingRate);
  std::uint32_t greatestCommonMultiple =
      postSamplingRate * preSamplingRate / greatestCommonDevisor;

  preCoefficient = postSamplingRate / greatestCommonDevisor;
  postCoefficient = preSamplingRate / greatestCommonDevisor;

  if (preSamplingRate > postSamplingRate)
    edgeFrequencyNomalization =
        static_cast<double>(postSamplingRate) / (2 * greatestCommonMultiple);
  else {
    edgeFrequencyNomalization =
        static_cast<double>(preSamplingRate) / (2 * greatestCommonMultiple);
  }
  DEBUG_PRINT_ARGS(TEXT("greatestCommonDevisor: %u\n"), greatestCommonDevisor);
  DEBUG_PRINT_ARGS(TEXT("greatestCommonMultiple: %u\n"),
                   greatestCommonMultiple);
  DEBUG_PRINT_ARGS(TEXT("edgeFrequencyNomalization: %f\n"),
                   edgeFrequencyNomalization);
  std::uint32_t quotientTemp = 100 / (preCoefficient * postCoefficient);
  if (!quotientTemp) {
    printf("This sampling rate isn't supported.");
    return 1;
  }
  delayUnitQuantity = (quotientTemp + 1) * (preCoefficient * postCoefficient);

  preExtraPcmDataFrameSize = delayUnitQuantity / preCoefficient;
  postExtraPcmDataFrameSize = delayUnitQuantity / postCoefficient;
  if (readingMethod == DataReadingMethod::successive) {
    if ((criterionDataFrameSize % preCoefficient)) {
      printf("This samling rate %u isn't supported.", preSamplingRate);
      return 1;
    }
    postPcmDataFrameSize = criterionDataFrameSize + postExtraPcmDataFrameSize;
    middlePcmDataFrameSize = postPcmDataFrameSize * postCoefficient;
    prePcmDataFrameSize = middlePcmDataFrameSize / preCoefficient;
  } else if (readingMethod == DataReadingMethod::collective) {
    if (criterionDataFrameSize % postCoefficient)
      prePcmDataFrameSize =
          ((criterionDataFrameSize / postCoefficient) + 1) * postCoefficient +
          preExtraPcmDataFrameSize;
    else
      prePcmDataFrameSize = criterionDataFrameSize + preExtraPcmDataFrameSize;

    middlePcmDataFrameSize = prePcmDataFrameSize * preCoefficient;
    postPcmDataFrameSize = middlePcmDataFrameSize / postCoefficient;
  }
  finalPcmDataArray.resize(middlePcmDataFrameSize * channelQuantity);
  initializeCommonTask();

  DEBUG_PRINT_ARGS(TEXT("prePcmDataFrameSize:%u\n"), prePcmDataFrameSize);
  DEBUG_PRINT_ARGS(TEXT("middlePcmDataFrameSize:%u\n"), middlePcmDataFrameSize);
  DEBUG_PRINT_ARGS(TEXT("postPcmDataFrameSize : %u\n"), postPcmDataFrameSize);
  return 0;
}

std::uint32_t FIRfilter::getPrePcmDataArrayFrameSize() const noexcept {
  return middlePcmDataFrameSize;
}

std::uint32_t FIRfilter::getPreExtraPcmDataFrameSize() const noexcept {
  return preExtraPcmDataFrameSize;
}

std::uint32_t FIRfilter::getPostExtraPcmDataFrameSize() const noexcept {
  return postExtraPcmDataFrameSize;
}

std::uint16_t FIRfilter::downSamplingTask(std::vector<double> &pcmDataArray) {
  if (postCoefficient == 1)
    return 0;

  for (std::uint32_t u = 0; u < middlePcmDataFrameSize; ++u)
    for (std::uint32_t v = 0; v < channelQuantity; ++v) {

      pcmDataArray[u * channelQuantity + v] =
          pcmDataArray[(u * channelQuantity) / postCoefficient + v];
    }
  return 0;
}

void FIRfilter::convertDoubleDataOfInt16bitToInt16bit(
    std::vector<std::int16_t> &pcm16bitDataArray) {
  std::uint32_t size = postPcmDataFrameSize * channelQuantity;

  for (std::uint32_t u = 0; u < size; ++u) {
    if (finalPcmDataArray[u] > (std::numeric_limits<std::int16_t>::max)())
      pcm16bitDataArray[u] = (std::numeric_limits<std::int16_t>::max)();
    else if (finalPcmDataArray[u] < (std::numeric_limits<std::int16_t>::min)())
      pcm16bitDataArray[u] = (std::numeric_limits<std::int16_t>::min)();
    else {
      pcm16bitDataArray[u] = std::floor(finalPcmDataArray[u] + 0.5);
    }
  }
}

void FIRfilter::convertDoubleDataOfFloatToInt16bit(
    std::vector<std::int16_t> &pcm16bitDataArray) {

  std::uint32_t size = pcm16bitDataArray.size();
  std::int32_t tempValue = 0;
  for (std::uint32_t u = preExtraPcmDataFrameSize * channelQuantity; u < size;
       ++u) {
    tempValue = std::floor(
        finalPcmDataArray[u] * (std::numeric_limits<std::int16_t>::max)() + .5);
    if (tempValue > (std::numeric_limits<std::int16_t>::max)())
      pcm16bitDataArray[u] = (std::numeric_limits<std::int16_t>::max)();
    else if (tempValue < (std::numeric_limits<std::int16_t>::min)())
      pcm16bitDataArray[u] = (std::numeric_limits<std::int16_t>::min)();
    else
      pcm16bitDataArray[u] = static_cast<std::int16_t>(tempValue);
  }
}

std::uint16_t FIRfilter::runConvertor(std::vector<std::int16_t> &pcmDataArray) {

  std::memset(&finalPcmDataArray[0], 0,
              sizeof(double) * middlePcmDataFrameSize * channelQuantity);

  if (FIRtemplate::upSamplingTask(channelQuantity, pcmDataArray, preCoefficient,
                                  prePcmDataFrameSize, middlePcmDataFrameSize))
    return 0;

  FIRtemplate::runFilter(channelQuantity, middlePcmDataFrameSize,
                         delayUnitQuantity, coefficientArray, pcmDataArray,
                         finalPcmDataArray, preCoefficient);

  downSamplingTask(finalPcmDataArray);
  convertDoubleDataOfInt16bitToInt16bit(pcmDataArray);
  return 0;
}

std::uint16_t FIRfilter::runConvertor(std::vector<float> &pcmDataArray) {
  std::memset(&finalPcmDataArray[0], 0,
              sizeof(double) * middlePcmDataFrameSize * channelQuantity);

  if (FIRtemplate::upSamplingTask(channelQuantity, pcmDataArray, preCoefficient,
                                  prePcmDataFrameSize, middlePcmDataFrameSize))
    return 0;

  FIRtemplate::runFilter(channelQuantity, middlePcmDataFrameSize,
                         delayUnitQuantity, coefficientArray, pcmDataArray,
                         finalPcmDataArray, preCoefficient);

  downSamplingTask(finalPcmDataArray);
  return 0;
}

double FIRfilter::getConvertedPcmOneSample(std::uint32_t frameIndex,
                                           std::uint8_t channelIndex) const {
  return finalPcmDataArray[frameIndex * channelQuantity + channelIndex];
}
} // namespace LowPass
} // namespace SamplingRate
