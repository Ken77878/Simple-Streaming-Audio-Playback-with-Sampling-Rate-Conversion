#pragma once


#include "primaryBuffer.h"
#include "portAudioInterfaceAbstract.h"


//#define FRAMES_PER_BUFFER  (64)

class PortAudioInterface : public PortAudioInterfaceAbstract{
private:
	Endian endianFlag;
	PaStream * stream;
	std::unique_ptr<PrimaryBuffer> primaryBufferPtr;
	//unique_ptr<thread> controlThread;
	//HANDLE streamEvent;
	/* The instance callback, where we have access to every method/variable in object of class Sine */
	int paCallbackMethod(const void *inputBuffer, void *outputBuffer,
		unsigned long framesPerBuffer,
		const PaStreamCallbackTimeInfo* timeInfo,
		PaStreamCallbackFlags statusFlags);
	/* This routine will be called by the PortAudio engine when audio is needed.
	** It may called at interrupt level on some machines so don't do anything
	** that could mess up the system like calling malloc() or free().
	*/
	static int32_t paCallback(const void *inputBuffer, void *outputBuffer,
		unsigned long framesPerBuffer,
		const PaStreamCallbackTimeInfo* timeInfo,
		PaStreamCallbackFlags statusFlags,
		void *userData);
	void paStreamFinishedMethod();
	/*
	* This routine is called by portaudio when playback is done.
	*/
	static void paStreamFinished(void* userData);
	//void controlStream(HANDLE event);
	bool openStream(PaDeviceIndex index);
	bool closeStream();

public:
	PortAudioInterface();
	~PortAudioInterface();
	uint32_t add(std::TSTRING filePath,
		bool loop, ReadingMethod  method);
	//void startStream();
	//void stopStream();
	void start(uint32_t id);
	uint16_t stop(uint32_t id);
	void remove(uint32_t id);
	void statusCheck();
};

