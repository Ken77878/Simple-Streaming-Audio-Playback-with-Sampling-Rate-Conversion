#pragma once


#include "oggVorbisBase.h"
#include <vector>
#include <atomic>
//#include <utility>
#include<array>
#include <limits>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>



class OggVorbisSuccessively :public OggVorbisBase {
private:
	bool loop;
	unsigned char & channels;
	std::array<std::vector<int16_t>, 2>& dataArray;
	int& fileIndex;
	std::atomic<bool>& ready;
	std::atomic<bool>&play;
	std::atomic<bool>&remove;
	std::array<HANDLE, 2>&  events;
	bool continuance;
	bool lastBuffer;
	uint16_t currentIndex;

	int headerExist();
	int readHeaders();
	int readConvertedData();
public:
	OggVorbisSuccessively(FILE* fp, bool loop, unsigned char & channels,
		std::array<std::vector<int16_t>, 2>& dataArray,
		int& fileIndex, std::atomic<bool>& ready, std::atomic<bool>&play,
		std::atomic<bool>&remove,
		std::array<HANDLE, 2>&  events
	);
	
	~OggVorbisSuccessively();
	void oggTask();
};