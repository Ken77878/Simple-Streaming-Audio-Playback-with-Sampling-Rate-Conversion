#pragma once
#pragma once
//#include <array>
//#define WIN32_LEAN_AND_MEAN
//#include <Windows.h>
//#include <vector>
//#include <atomic>
#include "secondaryBuffer.h"
#include "oggVorbisSuccessively.h"
#include "endianInfo.h"
#include "characterCodeMacro.h"
#include <string>


class SecondaryBufferSuccessivelyOgg :public SecondaryBuffer {
private:
	bool lastBuffer;
	uint32_t currentOffset;
	//size_t bufferSize;
	std::array<std::vector<int16_t>, 2> dataArray;
	int fileIndex;
	FILE  *fp;
	std::array<HANDLE, 2>  events;
	//0 emit in Secondary
	//1 emit in primary


	//HANDLE EventToSecondary;
	//atomic<bool>* playBack;

	std::unique_ptr<std::thread> loopThread;
	std::unique_ptr<OggVorbisSuccessively> oggVorbisInstance;
	void initialize();

	void threadTask();

	void startLoopThread();

	void stop();
public:
	SecondaryBufferSuccessivelyOgg(ReadingMethod method, Endian endianFlag, std::TSTRING filePath,
		PaStream * stream, bool loop, std::mutex* startStreamMutexPt);

	~SecondaryBufferSuccessivelyOgg();

	void updateStatus();

	int16_t getBufferData(DataChannel channel, uint32_t frame);

	HANDLE getEvent(unsigned char i) { return events[i]; }
};