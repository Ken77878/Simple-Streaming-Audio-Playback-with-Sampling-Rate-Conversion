#include "oggVorbisInterface.h"
#include <cmath>
#include <limits>
//#include <stdlib.h>
//#include "measureProcessingTime.h"

std::uint16_t OggVorbisInterface::playSuccessivelyWithoutLoopAtFileEnd() {
  for (uint32_t u = currentDataIndex; u < pcmDataArray.size(); ++u)
    for (uint32_t v = 0; v < channels; ++v) {
      pcmDataArray[u * channels + v] = 0;
    }
  playOn = false;
  continuance = false;
  // for next play
  fseek(fp, 0, SEEK_SET);
  return 0;
}

std::uint16_t OggVorbisInterface::firstPageExist() {
  printf("========headerExist() top==========\n");
  // grab some data at the head of the stream. We want the first page (which is
  // guaranteed to be small and only contain the Vorbis stream initial header)We
  // need the first page to get the stream serialno.

  // submit a 4k block to libvorbis' Ogg layer
  buffer = ogg_sync_buffer(&oggSyncState, 4096);
  bytes = fread(buffer, 1, 4096, fp);
  ogg_sync_wrote(&oggSyncState, bytes);

  /* Get the first page. */
  if (ogg_sync_pageout(&oggSyncState, &oggPage) != 1) {
    printf("=========ogg header don't exist.=========\n");
    /* have we simply run out of data?  If so, we're done. */
    // if the following logical bitstream is not exist, then we're done,
    if (bytes < 4096) {
      // reached file end
      printf("\n============play end: readed byte size "
             "%d=========\n",
             bytes);
      if (dataReadingMethod == DataReadingMethod::collective) {
        // All data was read collectively.
        return 0;
      }
      if (!loopSuccessively) {
        playSuccessivelyWithoutLoopAtFileEnd();
        return 0;
      }
      printf("=========to play from the first=========\n");
      fseek(fp, 0, SEEK_SET);
      return firstPageExist();
    } else {
      /* error case.  Must not be Vorbis data */
      fprintf(stderr, "Input does not appear to be an Ogg bitstream.\n");
      return 1;
    }
  }
  printf("\n=====first page exist!!!=======\n");
  return 0;
}

std::uint16_t OggVorbisInterface::readInfoHeader() {
  if (ogg_stream_pagein(&oggStreamState, &oggPage) < 0) {
    /* error; stream version mismatch perhaps */
    fprintf(stderr, "Error reading first page of Ogg bitstream data.\n");
    return 1;
  }

  if (ogg_stream_packetout(&oggStreamState, &oggPacket) != 1) {
    /* no page? must not be vorbis */
    fprintf(stderr, "Error reading initial header packet.\n");
    return 1;
  }

  if (!vorbis_synthesis_idheader(&oggPacket)) {
    fprintf(stderr, "the packet is not a Vorbis header packet.\n");
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

std::uint16_t OggVorbisInterface::readCommentsAndCodebooksHeadersPacketPart(
    std::uint32_t &headerCount) {
  while (headerCount < 2) {
    returnValue = ogg_stream_packetout(&oggStreamState, &oggPacket);
    // need more pages to make the packet
    if (returnValue == 0)
      break;

    if (returnValue < 0) {
      /* Uh oh; data at some point was corrupted or missing!
         We can't tolerate that in a header.  Die. */
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
}
std::uint16_t OggVorbisInterface::readCommentsAndCodebooksHeadersPagePart(
    std::uint32_t &headerCount) {
  while (headerCount < 2) {
    returnValue = ogg_sync_pageout(&oggSyncState, &oggPage);
    /* Need more data to make one page*/
    if (returnValue == 0)
      break;
    /* Don't complain about missing or corrupt data yet. We'll
       catch it at the packet output phase */
    if (returnValue == 1) {
      ogg_stream_pagein(&oggStreamState, &oggPage);
      /* we can ignore any errors here
      as they'll also become apparent
      at packetout */

      if (readCommentsAndCodebooksHeadersPacketPart(headerCount))
        return 1;
    }
  }
  return 0;
}

std::uint16_t OggVorbisInterface::readCommentsAndCodebooksHeaders() {
  std::uint32_t headerCount = 0;
  while (headerCount < 2) {
    if (readCommentsAndCodebooksHeadersPagePart(headerCount))
      return 1;
    // no harm in not checking before adding more
    buffer = ogg_sync_buffer(&oggSyncState, bytesPerCompressedPeriod);
    bytes = fread(buffer, 1, bytesPerCompressedPeriod, fp);
    if (bytes == 0 && headerCount < 2) {
      fprintf(stderr, "End of file before finding all Vorbis headers!\n");
      return 1;
    }
    ogg_sync_wrote(&oggSyncState, bytes);
  }
}

std::uint16_t
OggVorbisInterface::initializeOggStreamAndVorbisInfoAndVorbisComment() {
  // Get the serial number and set up the rest of decode.
  // serialno first; use it to set up a logical stream
  ogg_stream_init(&oggStreamState, ogg_page_serialno(&oggPage));

  // extract the initial header from the first page and verify that the
  //  Ogg bitstream is in fact Vorbis data

  // I handle the initial header first instead of just having the code
  //      read all three Vorbis headers at once because reading the initial
  //    header is an easy way to identify a Vorbis bitstream and it's
  //  useful to see that functionality seperated out.

  vorbis_info_init(&vorbisInfo);

  vorbis_comment_init(&vorbisComment);
  return 0;
}

std::uint16_t OggVorbisInterface::readHeaders() {

  printf("bitrate_upper : %d\n", vorbisInfo.bitrate_upper);
  printf("bitrate_nominal : %d\n", vorbisInfo.bitrate_nominal);
  printf("bitrate_lower : %d\n", vorbisInfo.bitrate_lower);

  if (readInfoHeader())
    return 1;
  /*  long rate;

    long bitrate_upper;
    long bitrate_nominal;
    long bitrate_lower;
    long bitrate_window;*/

  printf("channels: %d\n", vorbisInfo.channels);
  printf("sampling rate : %d\n", vorbisInfo.rate);
  printf("bitrate_upper : %d\n", vorbisInfo.bitrate_upper);
  printf("bitrate_nominal : %d\n", vorbisInfo.bitrate_nominal);
  printf("bitrate_lower : %d\n", vorbisInfo.bitrate_lower);

  channels = vorbisInfo.channels;
  if (channels > 2) {
    printf("Possible Channel size is one or two.");
    return 1;
  }
  pcmDataArray.resize(framesPerPeriod * channels);
  // get byte size of compressed sample
  if (vorbisInfo.bitrate_upper) {
    float sampleByteSize =
        vorbisInfo.bitrate_upper / 8.0f / 44100.0f / channels;
    if ((sampleByteSize - static_cast<std::uint32_t>(sampleByteSize)) > 0)
      bytesPerCompressedPeriod =
          (static_cast<std::uint32_t>(sampleByteSize) + 1) *
          pcmDataArray.size();
    else
      bytesPerCompressedPeriod =
          static_cast<std::uint32_t>(sampleByteSize) * pcmDataArray.size();
  } else {
    bytesPerCompressedPeriod = sizeof(std::int16_t) * pcmDataArray.size();
  }

  // At this point, we're sure we're Vorbis. We've set up the logical (Ogg)
  // bitstream decoder.Get the comment and codebook headers and set up the
  // Vorbis decoder

  // The next two packets in order are the comment and codebook headers. They're
  // likely large and may span multiple pages. Thus we read and submit data
  // until we get our two packets, watching that no pages are missing. If a page
  // is missing, error out;  loggStreamStateing a header page is the only place
  // where missing data is fatal.

  if (readCommentsAndCodebooksHeaders())
    return 1;

  /* Throw the comments plus a few lines about the bitstream we're
     decoding */
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

  /*3.Initialize a vorbis_dsp_state for decoding based on the parameters in the
  vorbis_info by using vorbis_synthesis_init. 4.Initialize a vorbis_block
  structure using vorbis_block_init. 5.While there are more packets to decode :
  a.Decode the next packet into a block using vorbis_synthesis.
  b.Submit the block to the reassembly layer using vorbis_synthesis_blockin.
  c.Obtain some decoded audio using vorbis_synthesis_pcmout and
  vorbis_synthesis_read. Any audio data returned but not marked as consumed
  using vorbis_synthesis_read carries over to the next call to
  vorbis_synthesis_pcmout.

  6.Destroy the structures using the appropriate vorbis_*_clear routines.*/
  return 0;
}

std::uint16_t OggVorbisInterface::getPcmData() {
  //**pcm is a multichannel float vector.  In stereo, for example, pcm[0] is
  // left, and pcm[1] is right.  samples is the size of each channel.  Convert
  // the float values (-1.<=range<=1.) to whatever PCM format and write it out
  float **pcm;
  std::int32_t samples;

  // The application is not required to make use of all of the samples made
  // available to it by one call to vorbis_synthesis_pcmout before it
  // continues to decode. Use vorbis_synthesis_read to inform the decoder of
  // how many samples were actually used. Any unused samples will be included
  // in the buffers output by the next call to this function.

  int32_t val = 5;
  bool clipflag = false;
  while ((samples = vorbis_synthesis_pcmout(&vorbisDspState, &pcm)) > 0) {

    //	printf("samples: %d\n", samples);
    // convert floats to 16 bit signed ints (host order) and interleave
    if (dataReadingMethod == DataReadingMethod::collective) {
      pcmDataArray.reserve(pcmDataArray.size() + samples * channels);
    }

    for (std::int32_t i = 0; i < samples; ++i) {
      for (std::int32_t j = 0; j < vorbisInfo.channels; ++j) {
        val = std::floor(
            pcm[j][i] * (std::numeric_limits<std::int16_t>::max)() + .5f);

        /* might as well guard against clipping */
        if (val > (std::numeric_limits<std::int16_t>::max)()) {
          //	printf("val: %d\n", val);
          val = (std::numeric_limits<std::int16_t>::max)();
          clipflag = true;
        }
        if (val < (std::numeric_limits<std::int16_t>::min)()) {
          val = (std::numeric_limits<std::int16_t>::min)();
          clipflag = true;
        }

        if (dataReadingMethod == DataReadingMethod::collective)
          pcmDataArray.push_back(static_cast<std::int16_t>(val));
        if (dataReadingMethod == DataReadingMethod::successive)
          pcmDataArray[currentDataIndex] = static_cast<std::int16_t>(val);
      }
      if (dataReadingMethod == DataReadingMethod::successive) {
        ++currentDataIndex;
        if (currentDataIndex == pcmDataArray.size()) {
          // printf("currentIndex: %d\n", currentIndex);
          currentDataIndex = 0;
          // informs the Vorbis decoder of how many samples the
          // application used from the last buffer output by
          // vorbis_synthesis_pcmout.
          vorbis_synthesis_read(
              &vorbisDspState,
              i + 1); // tell libvorbis how many samples we actually consumed
          continuance = true;
          return 0;
        }
      }
    }

    //	if (clipflag)
    //	fprintf(stderr, "Clipping in frame %ld\n",

    // fwrite(convbuffer, 2 * vorbisInfo.channels, bout, stdout);
    // tell libvorbis how many samples we actually consumed

    vorbis_synthesis_read(&vorbisDspState, samples);
  }
  return 0;
}

std::uint16_t OggVorbisInterface::readOggPacketPart() {
  while (1) {
    // printf("=============================test2");
    if (!continuance) {
      returnValue = ogg_stream_packetout(&oggStreamState, &oggPacket);

      if (returnValue == 0)
        break;               /* need more data */
      if (returnValue < 0) { /* missing or corrupt data at this page position */
        /* no reason to complain; already complained above */
        break;
      }
      // we have a packet.  Decode it
      if (vorbis_synthesis(&vorbisBlock, &oggPacket) ==
          0) // test for success! //
        vorbis_synthesis_blockin(&vorbisDspState, &vorbisBlock);
    }
    if (continuance)
      continuance = false;

    if (getPcmData())
      return 1;
    if (continuance)
      break;
  }
  return 0;
}

std::uint16_t OggVorbisInterface::readOggDataPagePart() {
  while (!endOfStream) {
    // printf("=============================test1");
    if (!continuance) {
      returnValue = ogg_sync_pageout(&oggSyncState, &oggPage);
      if (returnValue == 0)
        break;               /* need more data */
      if (returnValue < 0) { /* missing or corrupt data at this page position */
        fprintf(stderr, "Corrupt or missing data in bitstream; "
                        "continuing...\n");
        if (ogg_page_eos(&oggPage)) {
          endOfStream = 1;
          // logicalStreamEnd = true;
        }
        break;
      }

      ogg_stream_pagein(&oggStreamState, &oggPage);
      /* can safely ignore errors at this point */
    }
    if (readOggPacketPart())
      return 1;

    if (continuance)
      break;

    // Indicates whether this page is at the end of the logical bitstream.
    if (ogg_page_eos(&oggPage)) {
      endOfStream = 1;
      // logicalStreamEnd = true;
    }
  }
  return 0;
}

std::uint16_t OggVorbisInterface::readOggData() {
  endOfStream = 0;
  /* The rest is just a straight decode loop until end of stream */
  while (!endOfStream) {
    if (readOggDataPagePart())
      return 1;
    if (continuance)
      break;

    if (!endOfStream) {
      buffer = ogg_sync_buffer(&oggSyncState, bytesPerCompressedPeriod);
      bytes = fread(buffer, 1, bytesPerCompressedPeriod, fp);
      // printf("bytes: %d\n", bytes);
      ogg_sync_wrote(&oggSyncState, bytes);
      if (bytes == 0)
        endOfStream = 1;
    }
  }
  return 0;
}

void OggVorbisInterface::setLoopSuccessively(bool loopFlag) {
  loopSuccessively = loopFlag;
  if (loopSuccessively)
    printf("=======loop is true=======\n");
  else
    printf("\n======loop is false=======\n");
}

OggVorbisInterface::OggVorbisInterface(std::uint8_t &channels,
                                       std::vector<int16_t> &pcmDataArray,
                                       bool &playOn)
    : playOn(playOn), continuance(false), endOfStream(0), channels(channels),
      framesPerPeriod(0), pcmDataArray(pcmDataArray), currentDataIndex(0),
      loopSuccessively(false), firstPageExistFlag(false),
      bytesPerCompressedPeriod(0) {}

OggVorbisInterface::~OggVorbisInterface() {}
std::uint16_t
OggVorbisInterface::initialize(FILE *fp, DataReadingMethod dataReadingMethod) {
  this->fp = fp;
  this->dataReadingMethod = dataReadingMethod;
  ogg_sync_init(&oggSyncState); /* Now we can read pages */
  return 0;
}

std::uint16_t OggVorbisInterface::finalize() {
  if (continuance) {
    if (clearVorbisDspAndBlock())
      return 1;
    if (clearOggStreamAndVorbisInfoAndVorbisComment())
      return 1;
  }
  ogg_sync_clear(&oggSyncState);
  return 0;
}

std::uint16_t OggVorbisInterface::initializeVorbisDspAndBlock() {
  /* OK, got and parsed all three headers. Initialize the Vorbis
   packet->PCM decoder. */
  if (vorbis_synthesis_init(&vorbisDspState, &vorbisInfo) != 0) {
    fprintf(stderr, "Error: Corrupt header during playback initialization.\n");
    return 1;
  }

  /* central decode state */
  vorbis_block_init(&vorbisDspState, &vorbisBlock);
  /* local state for most of the decode
  so multiple block decodes can
  proceed in parallel. We could init
  multiple vorbis_block structures
  for vorbisDespState here */
  return 0;
}

std::uint16_t OggVorbisInterface::clearVorbisDspAndBlock() {
  /* ogg_page and ogg_packet structs always point to storage in
  libvorbis.  They're never freed or manipulated directly */
  vorbis_block_clear(&vorbisBlock);
  vorbis_dsp_clear(&vorbisDspState);
  return 0;
}
std::uint16_t
OggVorbisInterface::clearOggStreamAndVorbisInfoAndVorbisComment() {
  /* clean up this logical bitstream; before exit we see if we're
  followed by another [chained] */
  ogg_stream_clear(&oggStreamState);
  vorbis_comment_clear(&vorbisComment);
  vorbis_info_clear(&vorbisInfo); /* must be called last */
  return 0;
}
std::uint16_t OggVorbisInterface::oggDecodeTask() {
  if (playOn)
    printf("play: true\n");
  else
    printf("play: false\n");

  while (1) { /* we repeat if the bitstream is chained */
              // int eoggStreamState = 0;
              //
    if (!continuance) {
      if (firstPageExist(firstPageExistFlag))
        return 1;

      if (!playOn)
        break;
      if (initializeOggStreamAndVorbisInfoAndVorbisComment())
        return 1;
      if (readHeaders()) {
        playOn = false;
        fseek(fp, 0, SEEK_SET);
        break;
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

void OggVorbisInterface::setFramesPerPeriod(std::uint32_t frames) {
  framesPerPeriod = frames;
}
