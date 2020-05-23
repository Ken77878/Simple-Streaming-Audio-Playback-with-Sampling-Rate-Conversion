#include "secondaryBufferCollectiveWav.h"
#include <iostream>

namespace SecondaryBuffer {


std::uint16_t SuccessiveWav::setPcmDataSizeAndPrepareSamplingRateConversion(
    uint32_t bufferFrameCriterionSize) {
  if (samplingRate == 44100) {
    pcmInt16DataArray.resize(channels * bufferFrameCriterionSize);
  } else if (samplingRate == 22050) {
    switch (samplingRateConversionKind) {
    case SamplingRateConversionType::lowPass: {
      std::uint32_t dataArraySize = channels * bufferFrameCriterionSize;
      pcmInt16DataArray.resize(dataArraySize);
      pcmFloatDataArray.resize(dataArraySize);
      pcmInt16DataArray.resize(dataArraySize);
      FIRconvertorPtr.reset(new DigitalFilter::LowPass::FIRinterface(
          channels, pcmFloatDataArray, pcmDoubleDataArray, pcmInt16DataArray));
    } break;
    case SamplingRateConversionType::fourier: {
      sts::uint32_t dataArrayFrameSize =
          MyUtility::getTheNearestBiggerPowerOftwo(bufferFrameCriterionSize);
      std::uint32_t dataArraySize = dataArrayFrameSize * channels;
      pcmInt16DataArray.resize(dataArraySize);
      pcmFloatDataArray.resize(dataArraySize);
      pcmInt16DataArray.resize(dataArraySize);

      fastFourierConvertorPtr.reset(new FourierTransform::FastInterface(
          channels, pcmInt16DataArray, pcmFloatDataArray, pcmDoubleDataArray));
      fastFourierConvertorPtr->initialize(dataArrayFrameSize);
    }
    case default:
      printf("programming miss\n");
      return 1;
      break;
    }
  }
}

std::uint16_t CollectiveWav::initialize(std::string &&filePath) {
  FILE *fp = nullptr;
  std::uint32_t dataOffset = 0;
  std::uint32_t dataSize = 0;

  if (fopenWrap(fp, std::move(filePath)))
    return 1;
  if (WavBase::initialize(fp, endianFlag, dataSize, dataOffset, channels,
                          samplingRate))
    return 1;

  printf("wav dataSize : %lu\n", dataSize);
  fseek(fp, dataOffset, SEEK_SET);
  pcmInt16DataArray.resize(dataSize >> 1);
  printf("wav frame size %zu\n", pcmInt16DataArray.size());
  freadWrap(fp, pcmInt16DataArray, dataSize);
  fclose(fp);
  return 0;
} // namespace SecondaryBuffer
std::uint16_t CollectiveWav::finalize() { return 0; }
CollectiveWav::CollectiveWav(DataReadingMethod dataReadingMethodFlag,
                             Endian endianFlag, bool loop,
                             std::uint32_t framesPerPeriod)
    : CollectiveReading(dataReadingMethodFlag, loop, framesPerPeriod),
      WavBase(endianFlag) {}

} // namespace SecondaryBuffer
