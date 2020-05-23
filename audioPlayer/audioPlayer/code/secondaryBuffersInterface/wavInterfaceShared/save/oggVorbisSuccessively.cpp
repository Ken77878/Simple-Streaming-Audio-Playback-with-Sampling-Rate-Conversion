#include "oggVorbisSuccessively.h"
//#include <stdlib.h>
//#include "measureProcessingTime.h"
#include "portAudioConstants.h"

using namespace std;





int OggVorbisSuccessively::headerExist() {
//	static Mpt stopWatch;
	printf("========analyze headerExist==========\n");
	
	
	//when stop was executed waiting thread-ending at last time 
	if (continuance)
	{
		continuance = false;
		play = false;
		lastBuffer = false;
		fseek(fp, 0, SEEK_SET);
		SetEvent(events[0]);
		printf("thread end A\n");
		return 1;
	}
	
	
	
	/* grab some data at the head of the stream. We want the first page
	(which is guaranteed to be small and only contain the Vorbis
	stream initial header) We need the first page to get the stream
	serialno. */

	/* submit a 4k block to libvorbis' Ogg layer */
	buffer = ogg_sync_buffer(&oggSyncState, 4096);
	bytes = fread(buffer, 1, 4096, fp);
	ogg_sync_wrote(&oggSyncState, bytes);

	/* Get the first page. */
	if (ogg_sync_pageout(&oggSyncState, &oggPage) != 1) {
	//	stopWatch.end();
		printf("=========Header don't exist.=========\n");
		/* have we simply run out of data?  If so, we're done. */
		//if the following logical bitstream is not exist, then we're done,
		if (bytes < 4096) {
			if (!loop)
				printf("no loop\n");
			else printf("loop on\n");
			
			//file end
			if (!loop){
				lastBuffer = true;
				printf("\n============play end for no loop: readed byte size %d=========\n", bytes);
				for (uint16_t u = currentIndex; u < dataArray[fileIndex].size(); ++u) {
					dataArray[fileIndex][currentIndex] = 0;
				}
				
				if (!ready) {
					//fileIndex = ++fileIndex % 2;
					ready = true;
				}
				
				SetEvent(events[0]);

				DWORD res = WaitForSingleObject(events[1], INFINITE);
				if (!play) {
					continuance = true;
					return 1;
				}
				play = false;
				lastBuffer = false;
				//for next play
				fseek(fp, 0, SEEK_SET);
				SetEvent(events[0]);
				printf("thread end B\n");
				return 1;
				
			}
			else {
				fseek(fp, 0, SEEK_SET);
				printf("=========to  play from the first=========\n");
				return headerExist();


			}
		}
		else {
			/* error case.  Must not be Vorbis data */
			fprintf(stderr, "Input does not appear to be an Ogg bitstream.\n");
			exit(1);
		}
	}
//	stopWatch.start();

	return 0;

}


int OggVorbisSuccessively::readHeaders() {

	if (headerExist())
		return 1;

	printf("\n=====header exist!!!=======\n");
	/* Get the serial number and set up the rest of decode. */
	/* serialno first; use it to set up a logical stream */
	ogg_stream_init(&oggStreamState, ogg_page_serialno(&oggPage));

	/* extract the initial header from the first page and verify that the
	   Ogg bitstream is in fact Vorbis data */

	   /* I handle the initial header first instead of just having the code
		  read all three Vorbis headers at once because reading the initial
		  header is an easy way to identify a Vorbis bitstream and it's
		  useful to see that functionality seperated out. */

	vorbis_info_init(&vorbisInfo);
	vorbis_comment_init(&vorbisComment);
	if (ogg_stream_pagein(&oggStreamState, &oggPage) < 0) {
		/* error; stream version mismatch perhaps */
		fprintf(stderr, "Error reading first page of Ogg bitstream data.\n");
		exit(1);
	}

	if (ogg_stream_packetout(&oggStreamState, &oggPacket) != 1) {
		/* no page? must not be vorbis */
		fprintf(stderr, "Error reading initial header packet.\n");
		exit(1);
	}
	//info header
	//vorbis_synthesis_idheader
	if (vorbis_synthesis_headerin(&vorbisInfo, &vorbisComment, &oggPacket) < 0) {
		/* error case; not a vorbis header */
		fprintf(stderr, "This Ogg bitstream does not contain Vorbis "
			"audio data.\n");
		exit(1);
	}
	/*int channels;
	long rate;

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
	for (auto &data : dataArray)
			data.resize(::framesPerBuffer * vorbisInfo.channels);

	float size = vorbisInfo.bitrate_nominal / 8.0f / 44100.0f * dataArray[fileIndex].size();
	if ((uint32_t(size) + 1) > (std::numeric_limits<uint16_t>::max)())
	{
		printf("samples size is too big.\n");
	}
	if ((size - uint16_t(size)) > 0)
		bufferSize = uint16_t(size) + 1;
	else bufferSize = uint16_t(size);

	/* At this point, we're sure we're Vorbis. We've set up the logical
	   (Ogg) bitstream decoder. Get the comment and codebook headers and
	   set up the Vorbis decoder */

	   /* The next two packets in order are the comment and codebook headers.
		  They're likely large and may span multiple pages. Thus we read
		  and submit data until we get our two packets, watching that no
		  pages are missing. If a page is missing, error out; loggStreamStateing a
		  header page is the only place where missing data is fatal. */
	int i;
	i = 0;
	while (i < 2) {
		while (i < 2) {
			int result = ogg_sync_pageout(&oggSyncState, &oggPage);
			/* Need more data to make one page*/
			if (result == 0)break;
			/* Don't complain about missing or corrupt data yet. We'll
			   catch it at the packet output phase */
			if (result == 1) {
				ogg_stream_pagein(&oggStreamState, &oggPage);
				/* we can ignore any errors here
				as they'll also become apparent
				at packetout */
				while (i < 2) {
					result = ogg_stream_packetout(&oggStreamState, &oggPacket);
					//need more pages to make the packet
					if (result == 0)break;

					if (result < 0) {
						/* Uh oh; data at some point was corrupted or missing!
						   We can't tolerate that in a header.  Die. */
						fprintf(stderr, "Corrupt header:%d.  Exiting.\n", i + 2);
						exit(1);
					}
					//read header from one packet(i = 0:comment header, i = 1:codebook header)
					result = vorbis_synthesis_headerin(&vorbisInfo, &vorbisComment, &oggPacket);
					if (result < 0) {
						fprintf(stderr, "Corrupt secondary header:%d.  Exiting.\n", i + 2);
						exit(1);
					}
					i++;
				}
			}
		}
		/* no harm in not checking before adding more */
		buffer = ogg_sync_buffer(&oggSyncState, bufferSize);
		bytes = fread(buffer, 1, bufferSize, fp);
		if (bytes == 0 && i < 2) {
			fprintf(stderr, "End of file before finding all Vorbis headers!\n");
			exit(1);
		}
		ogg_sync_wrote(&oggSyncState, bytes);
	}

	/* Throw the comments plus a few lines about the bitstream we're
	   decoding */
	{
		char **ptr = vorbisComment.user_comments;
		while (*ptr) {
			fprintf(stderr, "%s\n", *ptr);
			++ptr;
		}
		fprintf(stderr, "\nBitstream is %d channel, %ldHz\n", vorbisInfo.channels, vorbisInfo.rate);
		fprintf(stderr, "Encoded by: %s\n\n", vorbisComment.vendor);
	}

	/*3.Initialize a vorbis_dsp_state for decoding based on the parameters in the vorbis_info by using vorbis_synthesis_init.
	4.Initialize a vorbis_block structure using vorbis_block_init.
	5.While there are more packets to decode :
	1.Decode the next packet into a block using vorbis_synthesis.
	2.Submit the block to the reassembly layer using vorbis_synthesis_blockin.
	3.Obtain some decoded audio using vorbis_synthesis_pcmout and vorbis_synthesis_read.
	Any audio data returned but not marked as consumed using vorbis_synthesis_read carries over to the next call to vorbis_synthesis_pcmout.

	6.Destroy the structures using the appropriate vorbis_*_clear routines.*/
	/* OK, got and parsed all three headers. Initialize the Vorbis
	packet->PCM decoder. */
	if (vorbis_synthesis_init(&vorbisDspState, &vorbisInfo) == 0) {
		/* central decode state */
		vorbis_block_init(&vorbisDspState, &vorbisBlock);
		/* local state for most of the decode
		so multiple block decodes can
		proceed in parallel. We could init
		multiple vorbis_block structures
		for vorbisDespState here */
	}
	else {
		fprintf(stderr, "Error: Corrupt header during playback initialization.\n");
		system("pause");
		exit(1);
	}
	return 0;
}


int OggVorbisSuccessively::readConvertedData() {


	int result = 0;
	if (play)
		printf("play: true\n");
	else
		printf("play: false\n");
	if (remove)
		printf("remove: true\n");
	else
		printf("remove: false\n");

	if (ready)
		printf("ready: true\n");
	else
		printf("ready: false\n");

	while ((play && !remove) || !ready) { /* we repeat if the bitstream is chained */
				//int eoggStreamState = 0;
		if (state == VorbisState::header)
		{
			if (readHeaders())
			{
				play = false;
				fseek(fp, 0, SEEK_SET);
				break;
			}
		}
		//printf("=============================test0");
		state = VorbisState::body;
		int endOfStream = 0;
		/* The rest is just a straight decode loop until end of stream */
	
		int bufferSum = 0;
		while (!endOfStream) {
			while (!endOfStream) {
				//printf("=============================test1");
				if (!continuance) {
					result = ogg_sync_pageout(&oggSyncState, &oggPage);
					if (result == 0)break; /* need more data */
					if (result < 0) { /* missing or corrupt data at this page position */
						fprintf(stderr, "Corrupt or missing data in bitstream; "
							"continuing...\n");
						if (ogg_page_eos(&oggPage)) {
							endOfStream = 1;
							//logicalStreamEnd = true;
						}
						break;
					}

					ogg_stream_pagein(&oggStreamState, &oggPage);
					/* can safely ignore errors at this point */
				}
				while (1) {
					//printf("=============================test2");
					if (!continuance) {
						result = ogg_stream_packetout(&oggStreamState, &oggPacket);

						if (result == 0)break; /* need more data */
						if (result < 0) { /* missing or corrupt data at this page position */
										  /* no reason to complain; already complained above */

							break;
						}

						/* we have a packet.  Decode it */


						if (vorbis_synthesis(&vorbisBlock, &oggPacket) == 0) /* test for success! */
							vorbis_synthesis_blockin(&vorbisDspState, &vorbisBlock);
					}

					/*
					**pcm is a multichannel float vector.  In stereo, for
					example, pcm[0] is left, and pcm[1] is right.  samples is
					the size of each channel.  Convert the float values
					(-1.<=range<=1.) to whatever PCM format and write it out */
					if (continuance)
						continuance = false;
					float **pcm;
					int samples;
					if ((samples = vorbis_synthesis_pcmout(&vorbisDspState, &pcm)) > 0) {
						int clipflag = 0;
					//	printf("samples: %d\n", samples);

		
						/* convert floats to 16 bit signed ints (host order) and
						interleave */
						//float  *mono = pcm[i];
					//	static long long count = 0;

						int channels = vorbisInfo.channels;
						for (int i = 0; i < samples; ++i) {

							for (int j = 0; j < channels; ++j) {
								//printf("=============================one sample");
								
								int32_t val = 5;
								val = floor(pcm[j][i] * 32767.f + .5f);
							/*	if (vorbisInfo.channels == 2) {
								//	if (pcm[j][i] > 1.0f)
									//	printf("pcm : %f\n", pcm);
									
									val = floor(pcm[j][i] * 32767.f + .5f);
								//	printf("val: %d\n", val);
								}
								else if (vorbisInfo.channels == 1) {
									//count++;

									
								//	printf("count: %ld, pcm : %.20f\n",count,  pcm[j][i]);
									val = floor(pcm[0][i] * 32767.f + .5f);

								//	printf("val: %d\n", val);
								}
								else {

									printf("Size of channels is wrong.\n");
									throw std::exception();
								}*/
								/* might as well guard against clipping */
								if (val > 32767) {
								//	printf("val: %d\n", val);
									val = 32767;
									clipflag = 1;
								}
								if (val < -32768) {
									val = -32768;
									clipflag = 1;
								}
							//	printf("fileIndex : %d, oggdata: %d\n",fileIndex, val );
								dataArray[fileIndex][currentIndex++] = val;

							}


							if (currentIndex == dataArray[fileIndex].size())
							{
								//printf("currentIndex: %d\n", currentIndex);

							//	printf("samples: %d, i: %d, fileIndex : %d\n", samples, i, fileIndex);
								
								currentIndex = 0;
							//	if (clipflag)
								//	fprintf(stderr, "Clipping in frame %ld\n", (long)(vorbisDspState.sequence));
								
								if (!ready) {
									printf("=============================ready\n");
									//fileIndex = ++fileIndex % 2;
									ready = true;
								}
								

								SetEvent(events[0]);
								printf("=========iWaitForSingleObject start ==============");
								
								
								DWORD res = WaitForSingleObject(events[1], INFINITE);
								printf("=========WaitForSingleObject end ==============");

								if (!play) {
									printf("=============================stop\n");
									vorbis_synthesis_read(&vorbisDspState, i + 1); /* tell libvorbis how
																			   many samples we
																			   actually consumed */
									continuance = true;
									return 1;
								}
							}

						}

					//	if (clipflag)
						//	fprintf(stderr, "Clipping in frame %ld\n", (long)(vorbisDspState.sequence));

						//fwrite(convbuffer, 2 * vorbisInfo.channels, bout, stdout);

						vorbis_synthesis_read(&vorbisDspState, samples); /* tell libvorbis how
																		 many samples we
																		 actually consumed */
					}
				}


				//Indicates whether this page is at the end of the logical bitstream. 
				if (ogg_page_eos(&oggPage)) {
					endOfStream = 1;
					//logicalStreamEnd = true;
				}
			}

			if (!endOfStream) {
				buffer = ogg_sync_buffer(&oggSyncState, bufferSize);
				bytes = fread(buffer, 1, bufferSize, fp);
				//printf("bytes: %d\n", bytes);
				ogg_sync_wrote(&oggSyncState, bytes);
				if (bytes == 0)endOfStream = 1;
			}
		}

		/* ogg_page and ogg_packet structs always point to storage in
		libvorbis.  They're never freed or manipulated directly */

		vorbis_block_clear(&vorbisBlock);
		vorbis_dsp_clear(&vorbisDspState);

		/* clean up this logical bitstream; before exit we see if we're
		followed by another [chained] */

		ogg_stream_clear(&oggStreamState);
		vorbis_comment_clear(&vorbisComment);
		vorbis_info_clear(&vorbisInfo);  /* must be called last */
		state = VorbisState::header;
	}
	return 0;
}

OggVorbisSuccessively::OggVorbisSuccessively(FILE* fp, bool loop, unsigned char & channels,
	std::array<std::vector<int16_t>, 2>& dataArray,
	int& fileIndex, std::atomic<bool>& ready, std::atomic<bool>&play,
	std::atomic<bool>&remove,
	std::array<HANDLE, 2>&  events
) :OggVorbisBase(fp), loop(loop), channels(channels), dataArray(dataArray), fileIndex(fileIndex), ready(ready), play(play), remove(remove), events(events), lastBuffer(false), currentIndex(0), continuance(false)
{
	if (loop)printf("=======loop is true=======\n");
	else printf("\n======loop is false=======\n");

}

OggVorbisSuccessively::~OggVorbisSuccessively() {
	/* ogg_page and ogg_packet structs always point to storage in
	libvorbis.  They're never freed or manipulated directly */

	vorbis_block_clear(&vorbisBlock);
	vorbis_dsp_clear(&vorbisDspState);

	/* clean up this logical bitstream; before exit we see if we're
	followed by another [chained] */

	ogg_stream_clear(&oggStreamState);
	vorbis_comment_clear(&vorbisComment);
	vorbis_info_clear(&vorbisInfo);  /* must be called last */
	ogg_sync_clear(&oggSyncState);
}


void OggVorbisSuccessively::oggTask() {
	
	if(!ready)
	ogg_sync_init(&oggSyncState); /* Now we can read pages */

	if (readConvertedData())
		return;
	if (remove) {
		ogg_sync_clear(&oggSyncState);
		printf("\n=============================ogg remove true===========================\n");

	}
	fprintf(stderr, "Done.\n");
}
