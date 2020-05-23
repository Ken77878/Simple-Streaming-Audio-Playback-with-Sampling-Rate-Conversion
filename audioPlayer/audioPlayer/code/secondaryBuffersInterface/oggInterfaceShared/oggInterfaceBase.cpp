#include "oggInterfaceBase.h"
#include "oggInterfaceConstants.h"
#include "systemErrorInterface.h"
#include "debugMacro.h"

namespace SecondaryBuffer {
namespace Ogg {
FileBase::FileBase(std::uint8_t deviceChannelQuantity,
                   std::uint16_t deviceSamlingRate,
                   std::uint32_t framesPerPeriod,
                   DataReadingMethod dataReadingMethodFlag, bool loop)
    : CommonBase(deviceChannelQuantity, deviceSamlingRate, framesPerPeriod,
                 dataReadingMethodFlag, loop) {}

std::uint16_t FileBase::initialize() {
  ogg_sync_init(&oggSyncState); /* Now we can read pages */
  return 0;
}

std::uint16_t FileBase::readInfoHeader() {
  if (ogg_stream_pagein(&oggStreamState, &oggPage) < 0) {
    // stream version mismatch perhaps
    fprintf(stderr, "Failed reading first page of ogg bitstream data.\n");
    return 1;
  }

  if (ogg_stream_packetout(&oggStreamState, &oggPacket) != 1) {
    // must not be vorbis
    fprintf(stderr, "Failed reading initial header packet.\n");
    return 1;
  }

  if (!vorbis_synthesis_idheader(&oggPacket)) {
    fprintf(stderr, "The packet is not a Vorbis header packet.\n");
    return 1;
  }

  // info header
  // decodes a header packet from a Vorbis stream and applies the contents to
  // the given vorbis_info structure (to provide codec parameters to the
  // decoder) and vorbis_comment structure (to provide access to the embedded
  // Vorbis comments).

  if ((returnValue = vorbis_synthesis_headerin(&vorbisInfo, &vorbisComment,
                                               &oggPacket)) < 0) {
    switch (returnValue) {
    case OV_ENOTVORBIS:
      fprintf(stderr, "the packet is not a Vorbis header packet.\n");
      break;
    case OV_EBADHEADER:
      fprintf(stderr, "there was an error interpreting the packet.\n");
      break;
    case OV_EFAULT:
      fprintf(stderr, "internal error\n");
      break;
    default:
      fprintf(stderr, "unknown error\n");
      break;
    }
    return 1;
  }
  return 0;
}

std::uint16_t FileBase::readCommentsAndCodebooksHeadersPacketPart(
    std::uint32_t &headerCount) {
  while (headerCount < 2) {
    returnValue = ogg_stream_packetout(&oggStreamState, &oggPacket);
    // need more pages to make the packet
    if (returnValue == 0)
      break;

    if (returnValue < 0) {
      // data at some point was corrupted or missing!
      fprintf(stderr, "Corrupt header:%d.  Exiting.\n", headerCount + 2);
      return 1;
    }
    // read header from one packet(i = 0:comment header, i = 1:codebook
    // header)
    returnValue =
        vorbis_synthesis_headerin(&vorbisInfo, &vorbisComment, &oggPacket);
    if (returnValue < 0) {
      fprintf(stderr, "Corrupt secondary header:%d.  Exiting.\n",
              headerCount + 2);
      return 1;
    }
    ++headerCount;
  }
  return 0;
}
std::uint16_t
FileBase::readCommentsAndCodebooksHeadersPagePart(std::uint32_t &headerCount) {
  while (headerCount < 2) {
    returnValue = ogg_sync_pageout(&oggSyncState, &oggPage);
    // Need more data to make one page
    if (returnValue == 0)
      break;
    if (returnValue == 1) {
      ogg_stream_pagein(&oggStreamState, &oggPage);
      // ignore any errors here as they'll also become apparent at packetout
      if (readCommentsAndCodebooksHeadersPacketPart(headerCount))
        return 1;
    }
  }
  return 0;
}

std::uint16_t FileBase::readCommentsAndCodebooksHeaders() {
  std::uint32_t headerCount = 0;
  while (headerCount < 2) {
    if (readCommentsAndCodebooksHeadersPagePart(headerCount))
      return 1;
    // no harm in not checking before adding more
    buffer = ogg_sync_buffer(&oggSyncState, oggWrittenBufferSize);
    bytes = fread(buffer, 1, oggWrittenBufferSize, fp);
    if (oggWrittenBufferSize > bytes)
      if (ferror(fp)) {
        SystemErrorInterface::systemErrorHandlingTask(TEXT("fread"));
        return 1;
      }

    if (bytes == 0 && headerCount < 2) {
      fprintf(stderr, "End of file before finding all Vorbis headers!\n");
      return 1;
    }
    ogg_sync_wrote(&oggSyncState, bytes);
  }
  return 0;
}

std::uint16_t FileBase::initializeOggStreamAndVorbisInfoAndVorbisComment() {
  // Get the serial number and set up the rest of decode.
  // serialno first; use it to set up a logical stream
  ogg_stream_init(&oggStreamState, ogg_page_serialno(&oggPage));

  // extract the initial header from the first page and verify that the
  //  Ogg bitstream is in fact Vorbis data

  vorbis_info_init(&vorbisInfo);

  vorbis_comment_init(&vorbisComment);
  return 0;
}

std::uint16_t FileBase::readHeaders() {
  if (readInfoHeader())
    return 1;
  DEBUG_PRINT_ARGS(TEXT("channels: %d\n"), vorbisInfo.channels);
  DEBUG_PRINT_ARGS(TEXT("sampling rate : %ld\n"), vorbisInfo.rate);
  DEBUG_PRINT_ARGS(TEXT("bitrate_upper : %ld\n"), vorbisInfo.bitrate_upper);
  DEBUG_PRINT_ARGS(TEXT("bitrate_nominal : %ld\n"), vorbisInfo.bitrate_nominal);
  DEBUG_PRINT_ARGS(TEXT("bitrate_lower : %ld\n"), vorbisInfo.bitrate_lower);

  fileChannelQuantity = vorbisInfo.channels;
  preSamplingRate = vorbisInfo.rate;

  // set up the logical (Ogg) bitstream decoder.Get the comment and codebook
  // headers and set up the Vorbis decoder

  if (readCommentsAndCodebooksHeaders())
    return 1;

#ifdef _DEBUG
  // throw the comments plus a few lines about the bitstream we're decoding
  {
    char **ptr = vorbisComment.user_comments;
    while (*ptr) {
      fprintf(stderr, "%s\n", *ptr);
      ++ptr;
    }
    fprintf(stderr, "\nBitstream is %d channel, %ldHz\n", vorbisInfo.channels,
            vorbisInfo.rate);
    fprintf(stderr, "Encoded by: %s\n\n", vorbisComment.vendor);
  }
#endif

  return 0;
}

std::uint16_t FileBase::initializeVorbisDspAndBlock() {
  // initialize the Vorbis  decoder from packet to PCM. */
  if (vorbis_synthesis_init(&vorbisDspState, &vorbisInfo) != 0) {
    fprintf(stderr, "Error: Corrupt header during playback initialization.\n");
    return 1;
  }

  // central decode state
  vorbis_block_init(&vorbisDspState, &vorbisBlock);
  return 0;
}

std::uint16_t FileBase::clearVorbisDspAndBlock() {
  // ogg_page and ogg_packet structs always point to storage in :w
  // libvorbis.  They're never freed or manipulated directly.
  vorbis_block_clear(&vorbisBlock);
  vorbis_dsp_clear(&vorbisDspState);
  return 0;
}
std::uint16_t FileBase::clearOggStreamAndVorbisInfoAndVorbisComment() {
  // clean up this logical bitstream
  ogg_stream_clear(&oggStreamState);
  vorbis_comment_clear(&vorbisComment);
  vorbis_info_clear(&vorbisInfo); /* must be called last */
  return 0;
}

FileBase::~FileBase() {}
} // namespace Ogg
} // namespace SecondaryBuffer
