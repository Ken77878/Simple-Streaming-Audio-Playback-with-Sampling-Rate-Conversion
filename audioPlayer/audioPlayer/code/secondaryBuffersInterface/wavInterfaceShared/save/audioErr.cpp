#include "audioError.h"
#include <cstdio>
#include <exception>

namespace AudioError {
	void AudioError::printError(PaError err) {
		fprintf(stderr, "An error occured while using the portaudio stream\n");
		fprintf(stderr, "Error number: %d\n", err);
		fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
		exit(1);

	}

	void AudioError::exit(int status) {
		Pa_Terminate();
		throw std::exception();
	}

	void AudioError::judgeError(PaError err) {
		if (err != paNoError)
			printError(err);
	}
}
