#pragma once

#include "portaudio.h"

namespace AudioError  {
	
	void printError(PaError err);

	void exit(int status);

	void judgeError(PaError err);
}
