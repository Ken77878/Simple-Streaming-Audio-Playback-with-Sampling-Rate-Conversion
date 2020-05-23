#include "fourierTransformBase.h"
#include "powerOfTwoUtility.h"
#include <limits>
#include <cstring>
#include <cstdio>
#include <cmath>
namespace SamplingRate {
namespace Fourier {

std::uint16_t TransformBase::initialize(DataReadingMethod readingMethod,
                                        std::uint8_t channelQuantity,
                                        std::uint32_t preSamplingRate,
                                        std::uint32_t postSamplingRate,
                                        std::uint32_t criterionDataFrameSize,
                                        std::uint32_t quotient) {

  if (preSamplingRate > postSamplingRate)
    convertedDataCoefficient = 1.0 / quotient;
  else
    convertedDataCoefficient = quotient;

  this->readingMethod = readingMethod;
  this->channelQuantity = channelQuantity;

  if (readingMethod == DataReadingMethod::collective) {
    prePcmDataFrameSize =
        MyUtility::getNearestBiggerPowerOfTwo(criterionDataFrameSize);
    if (preSamplingRate > postSamplingRate) {
      realDataArray.resize(prePcmDataFrameSize * channelQuantity);
      imaginaryDataArray.resize(prePcmDataFrameSize * channelQuantity);

      postPcmDataFrameSize = prePcmDataFrameSize / quotient;
      std::uint32_t remainder = criterionDataFrameSize % quotient;
      if (!remainder)
        postPcmDataFrameSizeWithoutPadding = criterionDataFrameSize / quotient;
      else {
        postPcmDataFrameSizeWithoutPadding =
            (criterionDataFrameSize / quotient + 1) * quotient;
      }
    } else {
      postPcmDataFrameSize = prePcmDataFrameSize * quotient;
      postPcmDataFrameSizeWithoutPadding = criterionDataFrameSize * quotient;
      realDataArray.resize(postPcmDataFrameSize * channelQuantity);
      imaginaryDataArray.resize(postPcmDataFrameSize * channelQuantity);
      if (quotient > postPcmDataFrameSize) {
        printf("too big preSamplingRate!!");
        return 1;
      }
    }
  } else if (readingMethod == DataReadingMethod::successive) {
    postPcmDataFrameSize =
        MyUtility::getNearestBiggerPowerOfTwo(criterionDataFrameSize);
    if (preSamplingRate > postSamplingRate) {
      prePcmDataFrameSize = quotient * postPcmDataFrameSize;
      realDataArray.resize(prePcmDataFrameSize * channelQuantity);
      imaginaryDataArray.resize(prePcmDataFrameSize * channelQuantity);

    } else {

      prePcmDataFrameSize = postPcmDataFrameSize / quotient;
      realDataArray.resize(postPcmDataFrameSize * channelQuantity);
      imaginaryDataArray.resize(postPcmDataFrameSize * channelQuantity);
      if (quotient > postPcmDataFrameSize) {
        printf("too big preSamplingRate!!");
        return 1;
      }
    }
  }
  return 0;
}
std::uint32_t TransformBase::getPrePcmDataArrayFrameSize() const noexcept {
  if (prePcmDataFrameSize > postPcmDataFrameSize)
    return prePcmDataFrameSize;
  else
    return postPcmDataFrameSize;
}

std::uint16_t TransformBase::upOrDownSamplingTask() {
    if (postPcmDataFrameSize > prePcmDataFrameSize) {
    std::memcpy(
        &realDataArray[(postPcmDataFrameSize - (prePcmDataFrameSize >> 1)) *
                       channelQuantity],
        &realDataArray[(prePcmDataFrameSize >> 1) * channelQuantity],

        sizeof(double) * (prePcmDataFrameSize >> 1) * channelQuantity);


    std::memset(&realDataArray[(prePcmDataFrameSize >> 1) * channelQuantity], 0,
                sizeof(double) * (postPcmDataFrameSize - prePcmDataFrameSize) *
                    channelQuantity);

    std::memcpy(
        &imaginaryDataArray[(postPcmDataFrameSize -
                             (prePcmDataFrameSize >> 1)) *
                            channelQuantity],
        &imaginaryDataArray[(prePcmDataFrameSize >> 1) * channelQuantity],
        sizeof(double) * (prePcmDataFrameSize >> 1) * channelQuantity);

    std::memset(
        &imaginaryDataArray[(prePcmDataFrameSize >> 1) * channelQuantity], 0,
        sizeof(double) * (postPcmDataFrameSize - prePcmDataFrameSize) *
            channelQuantity);

  } else if (prePcmDataFrameSize > postPcmDataFrameSize) {
    std::memcpy(
        &realDataArray[(postPcmDataFrameSize >> 1) * channelQuantity],
        &realDataArray[(prePcmDataFrameSize - (postPcmDataFrameSize >> 1)) *
                       channelQuantity],
        sizeof(double) * (postPcmDataFrameSize >> 1) * channelQuantity);
    std::memcpy(
        &imaginaryDataArray[(postPcmDataFrameSize >> 1) * channelQuantity],
        &imaginaryDataArray[(prePcmDataFrameSize -
                             (postPcmDataFrameSize >> 1)) *
                            channelQuantity],
        sizeof(double) * (postPcmDataFrameSize >> 1) * channelQuantity);
  }
  return 0;
}
void TransformBase::convertDoubleDataOfInt16bitToInt16bit(
    std::vector<std::int16_t> &pcm16bitDataArray) {
  std::uint32_t size = pcm16bitDataArray.size();

  for (std::uint32_t u = 0; u < size; ++u) {
    if (realDataArray[u] > (std::numeric_limits<std::int16_t>::max)())
      pcm16bitDataArray[u] = (std::numeric_limits<std::int16_t>::max)();
    else if (realDataArray[u] < (std::numeric_limits<std::int16_t>::min)())
      pcm16bitDataArray[u] = (std::numeric_limits<std::int16_t>::min)();
    else
      pcm16bitDataArray[u] = realDataArray[u];
  }
}

void TransformBase::convertDoubleDataOfFloatToInt16bit(
    std::vector<std::int16_t> &pcm16bitDataArray) {
  std::uint32_t size = pcm16bitDataArray.size();
  std::int32_t tempValue = 0;
  for (std::uint32_t u = 0; u < size; ++u) {
    tempValue = std::floor(
        realDataArray[u] * (std::numeric_limits<std::int16_t>::max)() + 0.5);

    if (tempValue > (std::numeric_limits<std::int16_t>::max)())
      pcm16bitDataArray[u] = (std::numeric_limits<std::int16_t>::max)();
    else if (tempValue < (std::numeric_limits<std::int16_t>::min)())
      pcm16bitDataArray[u] = (std::numeric_limits<std::int16_t>::min)();
    else
      pcm16bitDataArray[u] = tempValue;
  }
}

std::uint32_t TransformBase::getPostPcmDataFrameSize() const noexcept {


  if (readingMethod == DataReadingMethod::collective) {
    return postPcmDataFrameSizeWithoutPadding;
  } else {
    if (readingMethod == DataReadingMethod::collective)
      return postPcmDataFrameSize;
  }
}
} // namespace Fourier
} // namespace SamplingRate
