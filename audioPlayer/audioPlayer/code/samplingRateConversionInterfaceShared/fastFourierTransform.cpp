#include "fastFourierTransform.h"
#include "m_piConstants.h"
#include <cmath>
#include <limits>
#include <cstdio>
#include <cstring>

namespace SamplingRate {
namespace Fourier {
FastTransform::FastTransform() {}

std::uint16_t FastTransform::initialize(DataReadingMethod readingMethod,
                                        std::uint8_t channelQuantity,
                                        std::uint32_t preSamplingRate,
                                        std::uint32_t postSamplingRate,
                                        std::uint32_t criterionDataFrameSize,
                                        std::uint32_t quotient) {

  if (TransformBase::initialize(readingMethod, channelQuantity, preSamplingRate,
                                postSamplingRate, criterionDataFrameSize,
                                quotient)) {

    printf("error occurred.\n");

    return 1;
  }

  thetaAtNormal = -2 * M_PI / prePcmDataFrameSize;
  thetaAtInverse = 2 * M_PI / postPcmDataFrameSize;

  uint32_t loopCounterNormal = prePcmDataFrameSize >> 1;
  normalSpecialIndex = {loopCounterNormal >> 2, loopCounterNormal >> 1,
                        loopCounterNormal - (loopCounterNormal >> 2)};
  wRealNormalValueArray.reserve(loopCounterNormal);
  wImaginaryNormalValueArray.reserve(loopCounterNormal);

  for (uint32_t u = 0; u < loopCounterNormal; ++u) {
    wRealNormalValueArray.push_back(std::cos(thetaAtNormal * u));
    wImaginaryNormalValueArray.push_back(std::sin(thetaAtNormal * u));
  }
  uint32_t loopCounterInverse = postPcmDataFrameSize >> 1;
  normalSpecialIndex = {loopCounterInverse >> 2, loopCounterInverse >> 1,
                        loopCounterInverse - (loopCounterInverse >> 2)};
  wRealInverseValueArray.reserve(loopCounterInverse);
  wImaginaryInverseValueArray.reserve(loopCounterInverse);

  for (uint32_t u = 0; u < loopCounterInverse; ++u) {
    wRealInverseValueArray.push_back(std::cos(thetaAtInverse * u));
    wImaginaryInverseValueArray.push_back(std::sin(thetaAtInverse * u));
  }

  return 0;
}

std::uint16_t FastTransform::normalTransform(std::uint32_t bufferFrameSize) {
  double realTemp = 0, imaginaryTemp = 0;

  std::uint32_t coefficient = 0;
  std::uint32_t wIndex = 0;
  std::uint32_t dataIndex1 = 0;
  std::uint32_t dataIndex2 = 0;
  std::uint32_t vHalfValue = 0;

  for (uint32_t u = 0; u < channelQuantity; ++u) {
    coefficient = 0;
    for (std::uint32_t v = bufferFrameSize; (vHalfValue = v >> 1) >= 1;
         v = vHalfValue) {
      for (std::uint32_t x = 0; x < vHalfValue; ++x) {
        wIndex = x << coefficient;
        for (std::uint32_t y = x; y < bufferFrameSize; y += v) {
          dataIndex1 = y * channelQuantity + u;
          dataIndex2 = (y + vHalfValue) * channelQuantity + u;
          realTemp = realDataArray[dataIndex1] - realDataArray[dataIndex2];
          imaginaryTemp =
              imaginaryDataArray[dataIndex1] - imaginaryDataArray[dataIndex2];
          realDataArray[dataIndex1] += realDataArray[dataIndex2];
          imaginaryDataArray[dataIndex1] += imaginaryDataArray[dataIndex2];
          {
            realDataArray[dataIndex2] =
                wRealNormalValueArray[wIndex] * realTemp -
                wImaginaryNormalValueArray[wIndex] * imaginaryTemp;
            imaginaryDataArray[dataIndex2] =
                wRealNormalValueArray[wIndex] * imaginaryTemp +
                wImaginaryNormalValueArray[wIndex] * realTemp;
          }
        }
      }
      ++coefficient;
    }
  }
  return 0;
}

std::uint16_t FastTransform::normalTransform2(std::uint32_t bufferFrameSize) {

  double realTemp = 0, imaginaryTemp = 0;

  std::uint32_t coefficient = 0;
  std::uint32_t wIndex = 0;
  std::uint32_t dataIndex1 = 0;
  std::uint32_t dataIndex2 = 0;
  std::uint32_t vHalfValue = 0;

  for (uint32_t u = 0; u < channelQuantity; ++u) {
    coefficient = 0;
    for (std::uint32_t v = bufferFrameSize; (vHalfValue = v >> 1) >= 1;
         v = vHalfValue) {

      for (std::uint32_t x = 0; x < vHalfValue; ++x) {
        wIndex = x << coefficient;
        if (wIndex == 0) {
          for (std::uint32_t y = x; y < bufferFrameSize; y += v) {
            dataIndex1 = y * channelQuantity + u;
            dataIndex2 = (y + vHalfValue) * channelQuantity + u;
            realTemp = realDataArray[dataIndex1] - realDataArray[dataIndex2];
            imaginaryTemp =
                imaginaryDataArray[dataIndex1] - imaginaryDataArray[dataIndex2];
            realDataArray[dataIndex1] += realDataArray[dataIndex2];
            imaginaryDataArray[dataIndex1] += imaginaryDataArray[dataIndex2];
            {
              realDataArray[dataIndex2] = realTemp;
              imaginaryDataArray[dataIndex2] = imaginaryTemp;
            }
          }
        } else if (wIndex == normalSpecialIndex[0]) {
          for (std::uint32_t y = x; y < bufferFrameSize; y += v) {
            dataIndex1 = y * channelQuantity + u;
            dataIndex2 = (y + vHalfValue) * channelQuantity + u;
            realTemp = realDataArray[dataIndex1] - realDataArray[dataIndex2];
            imaginaryTemp =
                imaginaryDataArray[dataIndex1] - imaginaryDataArray[dataIndex2];
            realDataArray[dataIndex1] += realDataArray[dataIndex2];
            imaginaryDataArray[dataIndex1] += imaginaryDataArray[dataIndex2];
            {
              realDataArray[dataIndex2] =
                  wRealNormalValueArray[wIndex] * (realTemp + imaginaryTemp);
              imaginaryDataArray[dataIndex2] =
                  wRealNormalValueArray[wIndex] * (imaginaryTemp - realTemp);
            }
          }

        } else if (wIndex == normalSpecialIndex[1]) {
          for (std::uint32_t y = x; y < bufferFrameSize; y += v) {
            dataIndex1 = y * channelQuantity + u;
            dataIndex2 = (y + vHalfValue) * channelQuantity + u;
            realTemp = realDataArray[dataIndex1] - realDataArray[dataIndex2];
            imaginaryTemp =
                imaginaryDataArray[dataIndex1] - imaginaryDataArray[dataIndex2];
            realDataArray[dataIndex1] += realDataArray[dataIndex2];
            imaginaryDataArray[dataIndex1] += imaginaryDataArray[dataIndex2];
            {
              realDataArray[dataIndex2] = -imaginaryTemp;
              imaginaryDataArray[dataIndex2] = -realTemp;
            }
          }

        } else if (wIndex == normalSpecialIndex[2]) {
          for (std::uint32_t y = x; y < bufferFrameSize; y += v) {
            dataIndex1 = y * channelQuantity + u;
            dataIndex2 = (y + vHalfValue) * channelQuantity + u;
            realTemp = realDataArray[dataIndex1] - realDataArray[dataIndex2];
            imaginaryTemp =
                imaginaryDataArray[dataIndex1] - imaginaryDataArray[dataIndex2];
            realDataArray[dataIndex1] += realDataArray[dataIndex2];
            imaginaryDataArray[dataIndex1] += imaginaryDataArray[dataIndex2];
            {
              realDataArray[dataIndex2] =
                  wRealNormalValueArray[wIndex] * (realTemp - imaginaryTemp);
              imaginaryDataArray[dataIndex2] =
                  wRealNormalValueArray[wIndex] * (imaginaryTemp + realTemp);
            }
          }
        } else {
          for (std::uint32_t y = x; y < bufferFrameSize; y += v) {
            dataIndex1 = y * channelQuantity + u;
            dataIndex2 = (y + vHalfValue) * channelQuantity + u;
            realTemp = realDataArray[dataIndex1] - realDataArray[dataIndex2];
            imaginaryTemp =
                imaginaryDataArray[dataIndex1] - imaginaryDataArray[dataIndex2];
            realDataArray[dataIndex1] += realDataArray[dataIndex2];
            imaginaryDataArray[dataIndex1] += imaginaryDataArray[dataIndex2];
            {
              realDataArray[dataIndex2] =
                  wRealNormalValueArray[wIndex] * realTemp -
                  wImaginaryNormalValueArray[wIndex] * imaginaryTemp;
              imaginaryDataArray[dataIndex2] =
                  wRealNormalValueArray[wIndex] * imaginaryTemp +
                  wImaginaryNormalValueArray[wIndex] * realTemp;
            }
          }
        }
      }
      ++coefficient;
    }
  }
  return 0;
}

std::uint16_t
FastTransform::sortingAfterNormalTransform(std::uint32_t bufferFrameSize) {
  std::uint32_t halfBufferSize = bufferFrameSize >> 1;
  std::uint32_t halfBufferSizePlusOne = halfBufferSize + 1;
  double tempValue = 0;

  std::uint32_t u0 = 0;
  std::uint32_t dataIndex1 = 0;
  std::uint32_t dataIndex2 = 0;
  std::uint32_t dataIndex3 = 0;
  std::uint32_t dataIndex4 = 0;
  std::uint32_t dataIndex5 = 0;
  std::uint32_t dataIndex6 = 0;

  for (std::uint32_t v = 0; v < channelQuantity; ++v) {
    u0 = 0;
    for (std::uint32_t u1 = 0; u1 < halfBufferSize; u1 += 2) {
      if (u1 < u0) {
        dataIndex1 = u1 * channelQuantity + v;

        dataIndex2 = u0 * channelQuantity + v;
        dataIndex3 = (u1 + halfBufferSizePlusOne) * channelQuantity + v;

        dataIndex4 = (u0 + halfBufferSizePlusOne) * channelQuantity + v;

        tempValue = realDataArray[dataIndex1];
        realDataArray[dataIndex1] = realDataArray[dataIndex2];
        realDataArray[dataIndex2] = tempValue;
        tempValue = realDataArray[dataIndex3];
        realDataArray[dataIndex3] = realDataArray[dataIndex4];
        realDataArray[dataIndex4] = tempValue;

        tempValue = imaginaryDataArray[dataIndex1];
        imaginaryDataArray[dataIndex1] = imaginaryDataArray[dataIndex2];
        imaginaryDataArray[dataIndex2] = tempValue;
        tempValue = imaginaryDataArray[dataIndex3];
        imaginaryDataArray[dataIndex3] = imaginaryDataArray[dataIndex4];
        imaginaryDataArray[dataIndex4] = tempValue;
      }
      dataIndex5 = (u1 + halfBufferSize) * channelQuantity + v;
      dataIndex6 = (u0 + 1) * channelQuantity + v;

      tempValue = realDataArray[dataIndex5];
      realDataArray[dataIndex5] = realDataArray[dataIndex6];
      realDataArray[dataIndex6] = tempValue;

      tempValue = imaginaryDataArray[dataIndex5];
      imaginaryDataArray[dataIndex5] = imaginaryDataArray[dataIndex6];
      imaginaryDataArray[dataIndex6] = tempValue;

      for (std::uint32_t u2 = halfBufferSize >> 1; u2 > (u0 ^= u2); u2 >>= 1) {
      }
    }
  }
  return 0;
}

std::uint16_t FastTransform::inverseTransform(std::uint32_t bufferFrameSize) {

  double realTemp, imaginaryTemp;

  std::uint32_t coefficient = 0;
  std::uint32_t wIndex = 0;
  std::uint32_t dataIndex1 = 0;
  std::uint32_t dataIndex2 = 0;
  std::uint32_t vHalfValue = 0;

  for (uint32_t u = 0; u < channelQuantity; ++u) {
    coefficient = 0;
    for (std::uint32_t v = bufferFrameSize; (vHalfValue = v >> 1) >= 1;
         v = vHalfValue) {
      for (std::uint32_t x = 0; x < vHalfValue; ++x) {
        wIndex = x << coefficient;
        for (std::uint32_t y = x; y < bufferFrameSize; y += v) {
          dataIndex1 = y * channelQuantity + u;
          dataIndex2 = (y + vHalfValue) * channelQuantity + u;

          realTemp = realDataArray[dataIndex1] - realDataArray[dataIndex2];
          imaginaryTemp =
              imaginaryDataArray[dataIndex1] - imaginaryDataArray[dataIndex2];
          realDataArray[dataIndex1] += realDataArray[dataIndex2];
          imaginaryDataArray[dataIndex1] += imaginaryDataArray[dataIndex2];

          {
            realDataArray[dataIndex2] =
                wRealInverseValueArray[wIndex] * realTemp -
                wImaginaryInverseValueArray[wIndex] * imaginaryTemp;
            imaginaryDataArray[dataIndex2] =
                wRealInverseValueArray[wIndex] * imaginaryTemp +
                wImaginaryInverseValueArray[wIndex] * realTemp;
          }
        }
      }
      ++coefficient;
    }
  }
  // printf("inverse() end\n");
  // std::uint32_t  exponents
  for (auto &data : realDataArray)
    data = data / bufferFrameSize * convertedDataCoefficient;
  return 0;
}
std::uint16_t
FastTransform::sortingAfterInverseTransform(std::vector<double> &pcmDataArray,
                                            std::uint32_t bufferFrameSize) {
  std::uint32_t halfBufferSize = bufferFrameSize >> 1;
  std::uint32_t halfBufferSizePlusOne = halfBufferSize + 1;
  double tempValue = 0;
  std::uint32_t u0 = 0;
  std::uint32_t dataIndex1 = 0;
  std::uint32_t dataIndex2 = 0;
  std::uint32_t dataIndex3 = 0;
  std::uint32_t dataIndex4 = 0;
  std::uint32_t dataIndex5 = 0;
  std::uint32_t dataIndex6 = 0;

  for (std::uint32_t v = 0; v < channelQuantity; ++v) {
    u0 = 0;
    for (std::uint32_t u1 = 0; u1 < halfBufferSize; u1 += 2) {
      if (u1 < u0) {
        dataIndex1 = u1 * channelQuantity + v;
        dataIndex2 = u0 * channelQuantity + v;
        dataIndex3 = (u1 + halfBufferSizePlusOne) * channelQuantity + v;
        dataIndex4 = (u0 + halfBufferSizePlusOne) * channelQuantity + v;

        tempValue = pcmDataArray[dataIndex1];
        pcmDataArray[dataIndex1] = pcmDataArray[dataIndex2];
        pcmDataArray[dataIndex2] = tempValue;
        tempValue = pcmDataArray[dataIndex3];
        pcmDataArray[dataIndex3] = pcmDataArray[dataIndex4];
        pcmDataArray[dataIndex4] = tempValue;
      }
      dataIndex5 = (u1 + halfBufferSize) * channelQuantity + v;
      dataIndex6 = (u0 + 1) * channelQuantity + v;

      tempValue = pcmDataArray[dataIndex5];
      pcmDataArray[dataIndex5] = pcmDataArray[dataIndex6];
      pcmDataArray[dataIndex6] = tempValue;
      for (std::uint32_t u2 = halfBufferSize >> 1; u2 > (u0 ^= u2); u2 >>= 1)
        ;
    }
  }
  return 0;
}

std::uint16_t FastTransform::conversionTask() {
  normalTransform(prePcmDataFrameSize);
  sortingAfterNormalTransform(prePcmDataFrameSize);

  upOrDownSamplingTask();
  inverseTransform(postPcmDataFrameSize);
  sortingAfterInverseTransform(realDataArray, postPcmDataFrameSize);
  return 0;
}

std::uint16_t
FastTransform::runConvertor(std::vector<std::int16_t> &pcmDataArray) {
  std::memset(&imaginaryDataArray[0], 0,
              sizeof(double) * imaginaryDataArray.size());
  for (uint32_t u = 0; u < prePcmDataFrameSize; ++u)
    realDataArray[u] = pcmDataArray[u];
  conversionTask();
  return 0;
}

std::uint16_t FastTransform::runConvertor(std::vector<float> &pcmDataArray) {
  std::fill(imaginaryDataArray.begin(), imaginaryDataArray.end(), 0);
  for (uint32_t u = 0; u < prePcmDataFrameSize; ++u)
    realDataArray[u] = pcmDataArray[u];

  conversionTask();
  return 0;
}
double
FastTransform::getConvertedPcmOneSample(std::uint32_t frameIndex,
                                        std::uint8_t channelIndex) const {
  return realDataArray[frameIndex * channelQuantity + channelIndex];
}
} // namespace Fourier
} // namespace SamplingRate
