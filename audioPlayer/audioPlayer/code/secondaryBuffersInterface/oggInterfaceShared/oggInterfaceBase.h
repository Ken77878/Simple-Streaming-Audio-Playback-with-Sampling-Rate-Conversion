#pragma once

#include "oggInterfaceDLLheader.h"
#include <cstdio>
#include "vorbis/codec.h"
#include "secondaryBufferBase.h"
namespace SecondaryBuffer {

namespace Ogg {
class  oggInterface_API FileBase : public CommonBase {
protected:
  FILE *fp = nullptr;
  std::int32_t endOfStream = 0;
  std::int32_t returnValue = 0;
  std::vector<std::int16_t> pcm16bitDataArray;
  std::vector<float> pcm32bitDataArray;
  char *buffer = nullptr;
  uint16_t bufferSize = 0;
  std::uint32_t bytes = 0;

  ogg_sync_state oggSyncState; // sync and verify incoming physical bitstream
  ogg_page oggPage; // one Ogg bitstream page. Vorbis packets are inside
  ogg_stream_state oggStreamState; // take physical pages, weld into a logical
                                   // stream of packets
  ogg_packet oggPacket;            // one raw packet of data for decode
  vorbis_info
      vorbisInfo; // struct that stores all the static vorbis bitstream settings
  vorbis_comment
      vorbisComment; // struct that stores all the bitstream user comments
  vorbis_dsp_state
      vorbisDspState;       // central working state for the packet->PCM decoder
  vorbis_block vorbisBlock; // local working space for packet->PCM decode

  virtual std::uint16_t firstPageExist() = 0;
  std::uint16_t readInfoHeader();
  std::uint16_t
  readCommentsAndCodebooksHeadersPacketPart(std::uint32_t &headerCount);
  std::uint16_t
  readCommentsAndCodebooksHeadersPagePart(std::uint32_t &headerCount);
  std::uint16_t readCommentsAndCodebooksHeaders();
  std::uint16_t initializeOggStreamAndVorbisInfoAndVorbisComment();
  std::uint16_t readHeaders();
  virtual std::uint16_t getPcmInt16bitData() = 0;
  virtual std::uint16_t getPcmFloat32bitData() = 0;

  virtual std::uint16_t readOggPacketPart() = 0;
  virtual std::uint16_t readOggDataPagePart() = 0;
  virtual std::uint16_t readOggData() = 0;
  std::uint16_t initializeVorbisDspAndBlock();
  std::uint16_t clearVorbisDspAndBlock();
  std::uint16_t clearOggStreamAndVorbisInfoAndVorbisComment();

public:
  FileBase(std::uint8_t deviceChannelQuantity, std::uint16_t deviceSamlingRate,
           std::uint32_t framesPerPeriod,
           DataReadingMethod dataReadingMethodFlag, bool loop);
  virtual std::uint16_t initialize();
  virtual std::uint16_t oggDecodeTask() = 0;
  virtual ~FileBase();
};
} // namespace Ogg
} // namespace SecondaryBuffer
