#include "oggCollectiveInterface.h"
#include <cmath>
#include <limits>
#include "oggInterfaceConstants.h"
#include "fileManagement.h"
#include "systemErrorInterface.h"
#include "debugMacro.h"

namespace SecondaryBuffer {
namespace Ogg {
CollectiveInterface::CollectiveInterface(
    std::uint8_t deviceChannelQuantity, std::uint16_t deviceSamlingRate,
    std::uint32_t framesPerPeriod, DataReadingMethod dataReadingMethodFlag,
    bool loop)
    : FileBase(deviceChannelQuantity, deviceSamlingRate, framesPerPeriod,
               dataReadingMethodFlag, loop) {}

std::uint16_t CollectiveInterface::firstPageExist() {
  DEBUG_PRINT(TEXT("firstPageExist() start\n"));

  buffer = ogg_sync_buffer(&oggSyncState, 4096);

  if (!buffer) {
    std::fprintf(stderr, "ogg_sync_buffer failed.\n");
    return 1;
  }

  bytes = fread(buffer, 1, 4096, fp);

  ogg_sync_wrote(&oggSyncState, bytes);

  // get the first page.
  if (ogg_sync_pageout(&oggSyncState, &oggPage) != 1) {
    DEBUG_PRINT(TEXT("ogg header don't exist.\n"));
    if (bytes < 4096) {
      if (ferror(fp)) {
        SystemErrorInterface::systemErrorHandlingTask(TEXT("fread"));
        return 1;
      }
      // reached file end
      // All data was read collectively.
      fileEnd = true;
      return 0;
    } else {
      fprintf(stderr, "Input does not appear to be an ogg bitstream.\n");
      return 1;
    }
  }
  DEBUG_PRINT(TEXT("First page exist.\n"));
  return 0;
}

std::uint16_t CollectiveInterface::getPcmInt16bitData() {
  float **pcm;
  std::int32_t samples = 0;
  std::int32_t val = 5;
  bool clipflag = false;

  while ((samples = vorbis_synthesis_pcmout(&vorbisDspState, &pcm)) > 0) {
    // convert floats to 16 bit signed ints (host order) and interleave
    pcm16bitDataArray.reserve(pcm16bitDataArray.size() +
                              samples * fileChannelQuantity);

    for (std::int32_t i = 0; i < samples; ++i) {
      for (std::int32_t j = 0; j < fileChannelQuantity; ++j) {
        val = static_cast<std::int32_t>(std::floor(
            pcm[j][i] * (std::numeric_limits<std::int16_t>::max)() + .5f));

        // guard against clipping
        if (val > (std::numeric_limits<std::int16_t>::max)()) {
          pcm16bitDataArray.push_back(
              (std::numeric_limits<std::int16_t>::max)());
          clipflag = true;
        } else if (val < (std::numeric_limits<std::int16_t>::min)()) {
          pcm16bitDataArray.push_back(
              (std::numeric_limits<std::int16_t>::min)());
          clipflag = true;
        } else
          pcm16bitDataArray.push_back(static_cast<std::int16_t>(val));
      }
    }

#ifdef _DEBUG
    if (clipflag)
      fprintf(stderr, "Clipping in frame\n");
#endif
    // tell libvorbis how many samples we actually consumed
    vorbis_synthesis_read(&vorbisDspState, samples);
  }
  return 0;
}
std::uint16_t CollectiveInterface::getPcmFloat32bitData() {
  float **pcm;
  std::int32_t samples = 0;

  while ((samples = vorbis_synthesis_pcmout(&vorbisDspState, &pcm)) > 0) {
    // convert floats to 16 bit signed ints (host order) and interleave
    pcm32bitDataArray.reserve(pcm32bitDataArray.size() +
                              ((samples * fileChannelQuantity) << 1));

    for (std::int32_t i = 0; i < samples; ++i) {
      for (std::int32_t j = 0; j < fileChannelQuantity; ++j) {
        pcm32bitDataArray.push_back(pcm[j][i]);
      }
    }
    // tell libvorbis how many samples we actually consumed
    vorbis_synthesis_read(&vorbisDspState, samples);
  }
  return 0;
}

std::uint16_t CollectiveInterface::readOggPacketPart() {
  while (1) {
    returnValue = ogg_stream_packetout(&oggStreamState, &oggPacket);
    // need more data
    if (returnValue == 0)
      break;
    if (returnValue < 0) {
      // missing or corrupt data at this page position
      // already complained above
      break;
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
    if (preSamplingRate == postSamplingRate) {
      if (getPcmInt16bitData())
        return 1;
    } else {
      if (getPcmFloat32bitData())
        return 1;
    }
  }
  return 0;
}

std::uint16_t CollectiveInterface::readOggDataPagePart() {
  while (!endOfStream) {
    returnValue = ogg_sync_pageout(&oggSyncState, &oggPage);
    // need more data
    if (returnValue == 0)
      break;
    if (returnValue < 0) {
      // missing or corrupt data at this page position
#ifdef _DEBUG
      fprintf(stderr, "Corrupt or missing data in bitstream; "
                      "continuing...\n");
#endif
      if (ogg_page_eos(&oggPage)) {
        endOfStream = 1;
      }
      break;
    }

    ogg_stream_pagein(&oggStreamState, &oggPage);
    // we can safely ignore errors at this point
    if (readOggPacketPart())
      return 1;

    // Indicates whether this page is at the end of the logical bitstream.
    if (ogg_page_eos(&oggPage)) {
      endOfStream = 1;
    }
  }
  return 0;
}

std::uint16_t CollectiveInterface::readOggData() {
  endOfStream = 0;
  // The rest is just a straight decode loop until end of stream
  while (!endOfStream) {
    if (readOggDataPagePart())
      return 1;
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

std::uint16_t CollectiveInterface::oggDecodeTask() {
  while (1) {
    // repeat if the bitstream is chained
    if (firstPageExist())
      return 1;
    if (fileEnd)
      break;
    if (initializeOggStreamAndVorbisInfoAndVorbisComment())
      return 1;

    if (readHeaders())
      return 1;

    if (initializeVorbisDspAndBlock())
      return 1;

    if (readOggData())
      return 1;

    if (clearVorbisDspAndBlock())
      return 1;
    if (clearOggStreamAndVorbisInfoAndVorbisComment())
      return 1;
  }

  return 0;
}

std::uint16_t CollectiveInterface::initializeSamplingRateConversion(
    std::uint32_t originalFrameSize) {
  if (samplingRateConvertor.initialize(
          fileChannelQuantity, dataReadingMethodFlag,

          preSamplingRate, postSamplingRate, originalFrameSize))
    return 1;
  pcm32bitDataArray.resize(samplingRateConvertor.getPrePcmDataArrayFrameSize() *
                           fileChannelQuantity);
  pcm16bitDataArray.resize(samplingRateConvertor.getPostPcmDataFrameSize() *
                           fileChannelQuantity);
  return 0;
}

std::uint16_t CollectiveInterface::finalizeOggDecode() {
  if (clearVorbisDspAndBlock())
    return 1;
  if (clearOggStreamAndVorbisInfoAndVorbisComment())
    return 1;
  ogg_sync_clear(&oggSyncState);
  return 0;
}

std::uint16_t CollectiveInterface::initialize(std::TSTRING &filePath) {
  if (MyUtility::fopenWrap(fp, filePath))
    return 1;

  if (FileBase::initialize())
    return 1;

  if (oggDecodeTask())
    return 1;
  if (finalizeOggDecode())
    return 1;

  if (MyUtility::fcloseWrap(fp))
    return 1;

  if (preSamplingRate == postSamplingRate) {
    postPcmDataFrameSize = pcm16bitDataArray.size() / fileChannelQuantity;

  } else if (preSamplingRate != postSamplingRate) {
    if (initializeSamplingRateConversion(pcm32bitDataArray.size() /
                                         fileChannelQuantity))
      return 1;
    if (samplingRateConvertor.runConvertor(pcm32bitDataArray))
      return 1;

    samplingRateConvertor.convertDoubleDataOfFloatToInt16bit(pcm16bitDataArray);
    postPcmDataFrameSize = samplingRateConvertor.getPostPcmDataFrameSize();
  }
  DEBUG_PRINT(TEXT("CollectiveInterface::initialize() nomarllyEnd()\n"));
  return 0;
}

std::int16_t CollectiveInterface::getDataFromVector(std::uint8_t channelIndex) {
  if (!playOn)
    return 0;

  std::uint32_t currentReadFrameIndexTemp = currentReadFrameIndex;
  if (channelIndex == (deviceChannelQuantity - 1))
    ++currentReadFrameIndex;
  if (currentReadFrameIndex == postPcmDataFrameSize) {
    currentReadFrameIndex = 0;
    if (!loop) {
      DEBUG_PRINT(TEXT("Audio File was all read. To play end\n"));
      playOn = false;
    }
  }
  if (channelIndex <= (fileChannelQuantity - 1))
    return pcm16bitDataArray[currentReadFrameIndexTemp * fileChannelQuantity +
                             channelIndex];
  else {
    if (channelIndex == 1)
      return pcm16bitDataArray[currentReadFrameIndexTemp * fileChannelQuantity];
    else
      return 0;
  }
  return 0;
}
std::int16_t CollectiveInterface::getBufferData(std::uint8_t channelIndex) {
  return getDataFromVector(channelIndex);
}

std::uint16_t CollectiveInterface::finalize() { return 0; }

CollectiveInterface::~CollectiveInterface() {}
} // namespace Ogg
} // namespace SecondaryBuffer
