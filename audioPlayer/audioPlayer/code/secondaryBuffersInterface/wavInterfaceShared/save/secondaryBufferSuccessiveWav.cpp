#include "secondaryBufferSuccessiveWav.h"
#include <cstring>
// open
#include <sys/stat.h>
#include <fcntl.h>
// sysconf
#include <unistd.h>
// mmap
#include <sys/mman.h>
#include "powerOfTwoUtility.h"
#include "pcmSampleTypeEnumeration.h"
#include <cstring>
namespace SecondaryBuffer {
static constexpr uint32_t sampleByteSize = 2; // 16bit

SuccessiveWav::SuccessiveWav(DataReadingMethod dataReadingMethodFlag,
                             Endian endianFlag, bool loop,
                             std::uint32_t framesPerPeriod)
    : SuccessiveReading(dataReadingMethodFlag, loop, framesPerPeriod),
      WavBase(endianFlag), fileMapping(false), currentOffset(0) {}

std::uint16_t SuccessiveWav::setPcmDataSizeAndPrepareSamplingRateConversion(
    uint32_t bufferFrameCriterionSize) {
  if (samplingRate == 44100) {
    pcmInt16DataArray.resize(channels * bufferFrameCriterionSize);
  } else if (samplingRate == 22050) {
    switch (samplingRateConversionKind) {
    case SamplingRateConversionType::lowPass: {
      if (bufferFrameCriterionSize & 1)
        bufferFrameCriterionSize += 1;
      std::uint32_t dataArraySize = channels * bufferFrameCriterionSize;
      pcmInt16DataArray.resize(dataArraySize);
      pcmDoubleDataArray.resize(dataArraySize);
      FIRconvertorPtr.reset(new DigitalFilter::LowPass::FIRinterface(
          channels, pcmFloatDataArray, pcmInt16DataArray, pcmDoubleDataArray));
      FIRconvertorPtr->initialize(dataReadingMethodFlag,
                                  PcmSampleType::int16bit, samplingRate,
                                  usedSamplingRate, framesPerPeriod);
    } break;
    case SamplingRateConversionType::fourier: {
      /*  std::uint32_t dataArrayFrameSize =
            MyUtility::getTheNearestBiggerPowerOftwo(bufferFrameCriterionSize);
        std::uint32_t dataArraySize = dataArrayFrameSize * channels;
        pcmInt16DataArray.resize(dataArraySize);
        pcmDoubleDataArray.resize(dataArraySize);

        fastFourierConvertorPtr.reset(new FourierTransform::FastInterface(
            channels, pcmInt16DataArray, pcmFloatDataArray,
        pcmDoubleDataArray));
        fastFourierConvertorPtr->initialize(dataArrayFrameSize);*/
    } break;
    default:
      printf("programming miss\n");
      return 1;
      break;
    }
  }
  return 0;
}
std::uint16_t SuccessiveWav::readDataFromFile() {
  if (fileMapping)
    return 0;
  printf("readDataFromFile\n");
  currentReadCount = 0;
  // printf("current offset: %d", currentOffset);
  if ((currentOffset + bufferSize) >= dataSize) {
    if (!loop) {
      // printf("-----------file end\n");
      memset(&pcmInt16DataArray[0], 0, bufferSize);
      freadWrap(fp, pcmInt16DataArray, dataSize - currentOffset);
      playOn = false;
    } else {
      freadWrap(fp, pcmInt16DataArray, dataSize - currentOffset);
      fseek(fp, dataOffset, SEEK_SET);
      freadWrap(fp, pcmInt16DataArray, currentOffset + bufferSize - dataSize,
                dataSize - currentOffset);
      currentOffset += bufferSize - dataSize;
      // cout << currentOffset << endl;
    }
  } else {
    freadWrap(fp, pcmInt16DataArray, bufferSize);
    currentOffset += bufferSize;
  }
  return 0;
}

std::int16_t SuccessiveWav::getDataFromMmapMemory() {
  if (((currentReadCount + 1) >> (channels & 1)) == dataSize) {
    std::int16_t data =
        *(mmapDataTopPtr + (currentReadCount++ >> (channels & 1)));
    currentReadCount = 0;
    if (!loop) {
      printf("-----------wav play end\n");
      playOn = false;
    }
    return data;
  } else {
    return *(mmapDataTopPtr + (currentReadCount++ >> (channels & 1)));
  }
}

/*std::uint16_t SuccessiveWav::makePcmDataArrayForLowPassFilter() {
  std::memset(&pcmInt16DataArray[0], 0,
              sizeof(pcmInt16DataArray[0]) * pcmInt16DataArray.size());
  std::uint32_t arraySize = static_cast<uint32_t>(
      FIRconvertorPtr->getPrePcmDataFrameSize() * channels);
  std::uint32_t coefficient = FIRconvertorPtr->getPreCoefficient();

  if (fileMapping) {
    for (uint32_t u = arraySize; u != 0; --u)
      for (uint32_t v = 0; v < channels; ++v) {
        pcmInt16DataArray[(u * channels + v) * coefficient] =
            getDataFromMmapMemory();
      }
  } else {
    for (uint32_t u = arraySize; u != 0; --u)
      for (uint32_t v = 0; v < channels; ++v) {
        pcmInt16DataArray[(u * channels + v) * coefficient] =
            pcmInt16DataArray[u * channels + v];
        pcmInt16DataArray[u * channels + v] = 0;
      }
  }
  return 0;
}*/

std::uint16_t SuccessiveWav::runSamplingRateConvertor() {
  switch (samplingRateConversionKind) {
  case SamplingRateConversionType::lowPass: {
    makePcmDataArrayForLowPassFilter();
    FIRconvertorPtr->convert();
  } break;
  case SamplingRateConversionType::fourier: {
    /*     std::uint32_t dataArrayFrameSize =
             MyUtility::getTheNearestBiggerPowerOftwo(bufferFrameCriterionSize);
         std::uint32_t dataArraySize = dataArrayFrameSize * channels;
         pcmInt16DataArray.resize(dataArraySize);
         pcmDoubleDataArray.resize(dataArraySize);

         fastFourierConvertorPtr.reset(new FourierTransform::FastInterface(
             channels, pcmInt16DataArray, pcmFloatDataArray,
       pcmDoubleDataArray));
         fastFourierConvertorPtr->initialize(dataArrayFrameSize);*/
  } break;
  default:
    printf("programming miss\n");
    return 1;
    break;
  }
}

void SuccessiveWav::updateStatus() {}

std::int16_t SuccessiveWav::getBufferData() {

  if (!playOn)
    return 0;
  if (fileMapping)
    return getDataFromMmapMemory();
  else
    return getDataFromNormalMemory();
}
SuccessiveWav::~SuccessiveWav() {}

} // namespace SecondaryBuffer
