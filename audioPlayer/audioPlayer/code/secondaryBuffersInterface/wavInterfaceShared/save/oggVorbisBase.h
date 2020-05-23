#pragma once


#include <cstdio>
#include <vorbis/codec.h>

enum class VorbisState {
	header = 0,
	body = 1
};


class OggVorbisBase {
protected:
	FILE * fp;
	ogg_sync_state   oggSyncState; /* sync and verify incoming physical bitstream */
	ogg_page         oggPage; /* one Ogg bitstream page. Vorbis packets are inside */
	ogg_stream_state oggStreamState; /* take physical pages, weld into a logical stream of packets */
	ogg_packet       oggPacket; /* one raw packet of data for decode */
	vorbis_info      vorbisInfo; /* struct that stores all the static vorbis bitstream settings */
	vorbis_comment   vorbisComment; /* struct that stores all the bitstream user comments */
	vorbis_dsp_state vorbisDspState; /* central working state for the packet->PCM decoder */
	vorbis_block     vorbisBlock; /* local working space for packet->PCM decode */

	char *buffer;
	uint16_t bufferSize;
	int  bytes;

	bool play;
	VorbisState state;
	OggVorbisBase(FILE* fp) :fp(fp), play(false), state(VorbisState::header) {};
	virtual ~OggVorbisBase(){}
};
