#pragma once
#include "oggVorbisBase.h"
#include <vector>
#include <utility>

class OggVorbisCollectively :public OggVorbisBase {
private:
	std::vector<std::pair<unsigned char, std::vector<int16_t>>>& oggSamplesArray;
	int readHeaders();
	void readConvertedData();
public:
	OggVorbisCollectively(FILE *fp, std::vector<std::pair<unsigned char, std::vector<int16_t>>>& oggSamplesArray) :OggVorbisBase(fp), oggSamplesArray(oggSamplesArray) {}
	void oggTask();
};
