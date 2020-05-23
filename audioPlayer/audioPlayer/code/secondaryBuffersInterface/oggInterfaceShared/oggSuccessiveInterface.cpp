#include "oggSuccessiveInterface.h"
#include <cmath>
#include <limits>
#include <cstring>
#include "oggInterfaceConstants.h"
#include "powerOfTwoUtility.h"
#include "fileManagement.h"
#include "systemErrorInterface.h"
#include "debugMacro.h"

namespace SecondaryBuffer {
namespace Ogg {
SuccessiveInterface::SuccessiveInterface(

    std::uint8_t deviceChannelQuantity, std::uint16_t deviceSamplingRate,
    std::uint32_t framesPerPeriod, DataReadingMethod dataReadingMethodFlag,
    bool loop)
    : FileBase(deviceChannelQuantity, deviceSamplingRate, framesPerPeriod,
               dataReadingMethodFlag, loop) {
  preSamplingRate = deviceSamplingRate;
}

std::uint16_t SuccessiveInterface::playEndWithoutLoopAtFileEnd() {

  std::uint32_t pcmDataArrayFrameSize = 0;
  if (preSamplingRate == postSamplingRate) {
    pcmDataArrayFrameSize = framesPerPeriod;
  } else {
    pcmDataArrayFrameSize = samplingRateConvertor.getPrePcmDataFrameSize();
  }

  for (uint32_t u = currentReadFrameIndex; u < pcmDataArrayFrameSize; ++u)
    for (uint32_t v = 0; v < fileChannelQuantity; ++v) {
      if (preSamplingRate == postSamplingRate) {
        pcm16bitDataArray[u * fileChannelQuantity + v] = 0;
      } else {
        pcm32bitDataArray[u * fileChannelQuantity + v] = 0;
      }
    }
  playOn = false;
  continuance = false;
  // for next play
  fseek(fp, 0, SEEK_SET);
  return 0;
}

std::uint16_t SuccessiveInterface::firstPageExist() {
  DEBUG_PRINT(TEXT("firstPageExist() start\n"));
  // get first page which is
  // guaranteed to be small and only contain the Vorbis stream initial header
  buffer = ogg_sync_buffer(&oggSyncState, 4096);
  bytes = fread(buffer, 1, 4096, fp);
  ogg_sync_wrote(&oggSyncState, bytes);

  if (ogg_sync_pageout(&oggSyncState, &oggPage) != 1) {
    DEBUG_PRINT(TEXT("=========ogg header don't exist.=========\n"));
    if (bytes < 4096) {
      if (ferror(fp)) {
        SystemErrorInterface::systemErrorHandlingTask(TEXT("fread"));
        return 1;
      }

      // reached file end
      DEBUG_PRINT_ARGS(TEXT("play end: readed byte size "
                            "%d\n"),
                       bytes);
      if (!loop) {
        playEndWithoutLoopAtFileEnd();
        return 0;
      }
      DEBUG_PRINT(TEXT("to play from the first\n"));
      fseek(fp, 0, SEEK_SET);
      return firstPageExist();
    } else {
      // must not be Vorbis data
      fprintf(stderr, "Input does not appear to be an Ogg bitstream.\n");
      return 1;
    }
  }
  DEBUG_PRINT(TEXT("first page exist!!!\n"));
  return 0;
}
std::uint16_t SuccessiveInterface::getPcmInt16bitData() {
  float **pcm;
  std::int32_t samples = 0;
  int32_t val = 5;
  bool clipflag = false;

  while ((samples = vorbis_synthesis_pcmout(&vorbisDspState, &pcm)) > 0) {

    // convert floats to 16 bit signed ints (host order) and interleave
    for (std::int32_t i = 0; i < samples; ++i) {
      for (std::int32_t j = 0; j < fileChannelQuantity; ++j) {

        val = static_cast<std::int32_t>(std::floor(
            pcm[j][i] * (std::numeric_limits<std::int16_t>::max)() + .5f));

        // guard against clipping
        if (val > (std::numeric_limits<std::int16_t>::max)()) {
          pcm16bitDataArray[currentReadFrameIndex * fileChannelQuantity + j] =
              (std::numeric_limits<std::int16_t>::max)();
          clipflag = true;
        } else if (val < (std::numeric_limits<std::int16_t>::min)()) {
          pcm16bitDataArray[currentReadFrameIndex * fileChannelQuantity + j] =
              (std::numeric_limits<std::int16_t>::min)();
          clipflag = true;
        } else
          pcm16bitDataArray[currentReadFrameIndex * fileChannelQuantity + j] =
              static_cast<std::int16_t>(val);
      }
      ++currentReadFrameIndex;
      if (currentReadFrameIndex == framesPerPeriod) {

        // informs the Vorbis decoder of how many samples the
        // application used from the last buffer output by
        // vorbis_synthesis_pcmout.
        vorbis_synthesis_read(&vorbisDspState, i + 1);
        continuance = true;
        return 0;
      }
    }
#ifdef _DEBUG
    if (clipflag)
      fprintf(stderr, "Clipping in frame");
#endif
    vorbis_synthesis_read(&vorbisDspState, samples);
  }
  return 0;
}

std::uint16_t SuccessiveInterface::getPcmFloat32bitData() {
  float **pcm;
  std::int32_t samples = 0;
  std::uint32_t endFrameSize = 0;
  while ((samples = vorbis_synthesis_pcmout(&vorbisDspState, &pcm)) > 0) {
    for (std::int32_t i = 0; i < samples; ++i) {
      for (std::int32_t j = 0; j < fileChannelQuantity; ++j) {
        pcm32bitDataArray[currentReadFrameIndex * fileChannelQuantity + j] =
            pcm[j][i];
      }
      ++currentReadFrameIndex;
      if (currentReadFrameIndex ==
          samplingRateConvertor.getPrePcmDataFrameSize()) {
        // informs the Vorbis decoder of how many samples the
        // application used from the last buffer output by
        // vorbis_synthesis_pcmout.
        vorbis_synthesis_read(&vorbisDspState, i + 1);
        continuance = true;

        return 0;
      }
    }
    vorbis_synthesis_read(&vorbisDspState, samples);
  }
  return 0;
}

std::uint16_t SuccessiveInterface::readOggPacketPart() {
  while (1) {
    if (!continuance) {
      returnValue = ogg_stream_packetout(&oggStreamState, &oggPacket);
      // need more data
      if (returnValue == 0)
        break;
      if (returnValue < 0) {
        // missing or corrupt data at this page position
        // break;
      }
      // decodes a Vorbis packet into a block of data.
      if ((returnValue = vorbis_synthesis(&vorbisBlock, &oggPacket))) {
        switch (returnValue) {
        case OV_ENOTAUDIO:
          fprintf(stderr, "the packet is not an audio packet.\n");
          break;
        case OV_EBADPACKET:
          fprintf(stderr, "there was an error in the packet.\n");
          break;
        default:
          fprintf(stderr, "unknown error\n");
          break;
        }
        continue;
      }
      // submits a vorbis_block for assembly into the final decoded audio
      if ((returnValue =
               vorbis_synthesis_blockin(&vorbisDspState, &vorbisBlock))) {
        switch (returnValue) {
        case OV_EINVAL:
          fprintf(stderr,
                  "the decoder is in an invalid state to accept blocks.\n");
          break;
        default:
          fprintf(stderr, "unknown error\n");
          break;
        }
        return 1;
      }
    }
    if (continuance)
      continuance = false;
    if (preSamplingRate == postSamplingRate) {
      if (getPcmInt16bitData())
        return 1;
    } else {
      if (getPcmFloat32bitData())
        return 1;
    }

    if (continuance)
      break;
  }
  return 0;
}

std::uint16_t SuccessiveInterface::readOggDataPagePart() {
  while (!endOfStream) {
    if (!continuance) {
      returnValue = ogg_sync_pageout(&oggSyncState, &oggPage);
      // need more data
      if (returnValue == 0)
        break;
      if (returnValue < 0) {
        // missing or corrupt data at this page position
        fprintf(stderr, "Corrupt or missing data in bitstream; "
                        "continuing...\n");
        if (ogg_page_eos(&oggPage)) {
          endOfStream = 1;
        }
        break;
      }

      ogg_stream_pagein(&oggStreamState, &oggPage);
      // can safely ignore errors at this point
    }
    if (readOggPacketPart())
      return 1;

    if (continuance)
      break;

    // Indicates whether this page is at the end of the logical bitstream.
    if (ogg_page_eos(&oggPage)) {
      endOfStream = 1;
    }
  }
  return 0;
}

std::uint16_t SuccessiveInterface::readOggData() {
  endOfStream = 0;
  // a straight decode loop until end of stream
  while (!endOfStream) {
    if (readOggDataPagePart())
      return 1;
    if (continuance)
      break;

    if (!endOfStream) {
      buffer = ogg_sync_buffer(&oggSyncState, oggWrittenBufferSize);
      bytes = fread(buffer, 1, oggWrittenBufferSize, fp);
      if (oggWrittenBufferSize > bytes)
        if (ferror(fp)) {
          SystemErrorInterface::systemErrorHandlingTask(TEXT("fread"));
          return 1;
        }

      ogg_sync_wrote(&oggSyncState, bytes);
      if (bytes == 0)
        endOfStream = 1;
    }
  }
  return 0;
}

std::uint16_t SuccessiveInterface::initializeSamplingRateConversion() {
  if (samplingRateConvertor.initialize(
          fileChannelQuantity, dataReadingMethodFlag,

          preSamplingRate, postSamplingRate, framesPerPeriod))
    return 1;
  pcm32bitDataArray.resize(samplingRateConvertor.getPrePcmDataArrayFrameSize() *
                           fileChannelQuantity);
  pcm16bitDataArray.resize(samplingRateConvertor.getPostPcmDataFrameSize() *
                           fileChannelQuantity);

  preExtraPcmDataFrameSize =
      samplingRateConvertor.getPreExtraPcmDataFrameSize();
  preExtraPcmDataArray.resize(preExtraPcmDataFrameSize * fileChannelQuantity);
  return 0;
}

std::uint16_t SuccessiveInterface::oggDecodeTask() {

  while (1) {
    // repeat if the bitstream is chained
    if (!continuance) {
      if (firstPageExist())
        return 1;

      if (!playOn)
        break;
      if (initializeOggStreamAndVorbisInfoAndVorbisComment())
        return 1;
      if (readHeaders())
        return 1;

      if (preSamplingRate == postSamplingRate) {
        pcm16bitDataArray.resize(framesPerPeriod * fileChannelQuantity);
        currentReadFrameIndex = 0;
      } else {
        if (initializeSamplingRateConversion())
          return 1;
        currentReadFrameIndex = preExtraPcmDataFrameSize;
      }

      if (initializeVorbisDspAndBlock())
        return 1;
    }
    if (readOggData())
      return 1;
    if (continuance)
      break;
    if (clearVorbisDspAndBlock())
      return 1;
    if (clearOggStreamAndVorbisInfoAndVorbisComment())
      return 1;
  }
  return 0;
}

std::uint16_t SuccessiveInterface::initialize(std::TSTRING &filePath) {
  DEBUG_PRINT_ARGS(TEXT("filePath: %s\n"), filePath.c_str());
  if (MyUtility::fopenWrap(fp, filePath))
    return 1;

  if (FileBase::initialize())
    return 1;

  return 0;
}

std::uint16_t SuccessiveInterface::readDataFromFile() {
  if (!playOn)
    return 0;
  // inialized presamplingRate with the value of postSamplingRate in constructor
  // because preSamplingRate is unknown  for the first time.
  if (preSamplingRate == postSamplingRate)
    currentReadFrameIndex = 0;
  else {
    currentReadFrameIndex = preExtraPcmDataFrameSize;
    std::memcpy(&pcm32bitDataArray[0], &preExtraPcmDataArray[0],
                preExtraPcmDataFrameSize * fileChannelQuantity * sizeof(float));
  }
  if (oggDecodeTask())
    return 1;

  if (preSamplingRate != postSamplingRate) {
    std::memcpy(
        &preExtraPcmDataArray[0],
        &pcm32bitDataArray[(samplingRateConvertor.getPrePcmDataFrameSize() -
                            preExtraPcmDataFrameSize) *
                           fileChannelQuantity],
        preExtraPcmDataFrameSize * fileChannelQuantity * sizeof(float));
    if (samplingRateConvertor.runConvertor(pcm32bitDataArray))
      return 1;
    samplingRateConvertor.convertDoubleDataOfFloatToInt16bit(pcm16bitDataArray);
  }
  if (preSamplingRate == postSamplingRate)
    currentReadFrameIndex = 0;
  else
    currentReadFrameIndex =
        samplingRateConvertor.getPostExtraPcmDataFrameSize();
  return 0;
}

std::int16_t SuccessiveInterface::getDataFromVector(std::uint8_t channelIndex) {
  std::uint32_t currentReadFrameIndexTemp = currentReadFrameIndex;
  if (channelIndex == (deviceChannelQuantity - 1))
    ++currentReadFrameIndex;
  if (channelIndex <= (fileChannelQuantity - 1))
    return pcm16bitDataArray[currentReadFrameIndexTemp * fileChannelQuantity +
                             channelIndex];
  else {
    if (channelIndex == 1)
      return pcm16bitDataArray[currentReadFrameIndexTemp * fileChannelQuantity];
    else
      return 0;
  }
}

std::int16_t SuccessiveInterface::getBufferData(std::uint8_t channelIndex) {
  return getDataFromVector(channelIndex);
}

std::uint16_t SuccessiveInterface::finalize() {
  if (continuance) {
    if (clearVorbisDspAndBlock())
      return 1;
    if (clearOggStreamAndVorbisInfoAndVorbisComment())
      return 1;
  }
  ogg_sync_clear(&oggSyncState);

  if (MyUtility::fcloseWrap(fp))
    return 1;
  return 0;
}

SuccessiveInterface::~SuccessiveInterface() {}
} // namespace Ogg
} // namespace SecondaryBuffer
