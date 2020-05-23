#pragma once
#include "oggVorbisCollectively.h"
#include "secondaryBuffer.h"

class SecondaryBufferCollectivelyOgg : public SecondaryBuffer {
private:
	std::vector < std::pair<unsigned char, std::vector<int16_t>>> oggSamplesArray;
	uint16_t currentStream;
	uint32_t currentIndex;
	std::unique_ptr<OggVorbisCollectively> oggVorbisInstance;
	//uint32_t sampleIndex;
	//uint32_t allSamples;

	void initialize();

	HANDLE getEvent(unsigned char i);
public:
	SecondaryBufferCollectivelyOgg(ReadingMethod method, Endian endianFlag, std::TSTRING filePath,
		PaStream * stream, bool loop, std::mutex* startStreamMutexPt);

	int16_t getBufferData(DataChannel channelSide, uint32_t frame);
	void startLoopThread() {}
	void stop() {}
	void updateStatus() {}
};

