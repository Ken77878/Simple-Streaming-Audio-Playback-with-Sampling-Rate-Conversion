#pragma once
#include "characterCodeMacro.h"
#include <string>
#include "audioReadingMethod.h"

class PortAudioInterfaceAbstract {
public:
	virtual	uint32_t add(std::TSTRING filePath,
		bool loop, ReadingMethod  method) = 0;
	virtual	void start(uint32_t id) = 0;
	virtual uint16_t stop(uint32_t id) = 0;
	virtual	void remove(uint32_t id) = 0;
	virtual	void statusCheck() = 0;
};
