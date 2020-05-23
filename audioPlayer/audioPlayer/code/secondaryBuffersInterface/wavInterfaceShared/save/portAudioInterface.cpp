#include "stdio.h"
#include "searchAppropriateDevice.h"
#include "portAudioInterface.h"
#include "portAudioConstants.h"
#include <iostream>
#include "checkEndian.h"
#include "audioError.h"


int PortAudioInterface::paCallbackMethod(const void *inputBuffer, void *outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags)
{
	int16_t *out = (int16_t*)outputBuffer;

	(void)timeInfo; /* Prevent unused variable warnings. */
	(void)statusFlags;
	(void)inputBuffer;
	return primaryBufferPtr->playBack(out, framesPerBuffer);

}

/* This routine will be called by the PortAudio engine when audio is needed.
** It may called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
int  PortAudioInterface::paCallback(const void *inputBuffer, void *outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void *userData)
{
	//printf("using CPU timei: %lf", Pa_GetStreamCpuLoad(((PortAudioInterface*)userData)->stream));
	/* Here we cast userData to Sine* type so we can call the instance method paCallbackMethod, we can do that since
	we called Pa_OpenStream with 'this' for userData */
	return ((PortAudioInterface*)userData)->paCallbackMethod(inputBuffer, outputBuffer,
		framesPerBuffer,
		timeInfo,
		statusFlags);
}

void PortAudioInterface::paStreamFinishedMethod()
{
	printf("Stream Completed\n");
	statusCheck();
	primaryBufferPtr->paStreamFinishedMethod();
}

/*
* This routine is called by portaudio when playback is done.
*/
void  PortAudioInterface::paStreamFinished(void* userData)
{
	return ((PortAudioInterface*)userData)->paStreamFinishedMethod();
}

/*void controlStream(HANDLE event) {
	WaitForSingleObject(event, INFINITE);
	stopStream();
}*/
bool  PortAudioInterface::openStream(PaDeviceIndex index)
{
	PaStreamParameters outputParameters;

	outputParameters.device = index;
	if (outputParameters.device == paNoDevice) {
		return false;
	}

	const PaDeviceInfo* pInfo = Pa_GetDeviceInfo(index);
	if (pInfo != 0)
	{
		printf("Output device name: '%s'\r", pInfo->name);
	}

	outputParameters.channelCount = 2;       /* stereo output */
	outputParameters.sampleFormat = paInt16; /* 16 bit signed int32_t point output */
											 //outputParameters.sampleFormat = paFloat32;
	outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
	outputParameters.hostApiSpecificStreamInfo = NULL;

	PaError err = Pa_OpenStream(
		&stream,
		NULL, /* no input */
		&outputParameters,
		::samplingRate,
		::framesPerBuffer,// timer?
		paClipOff,      /* we won't output out of range samples so don't bother clipping them */
		&PortAudioInterface::paCallback,
		this            /* Using 'this' for userData so we can cast to Sine* in paCallback method */
	);

	if (err != paNoError)
	{

		std::cout << " Failed to open stream to device !!! " << std::endl;
		system("PAUSE");
		return false;
	}

	err = Pa_SetStreamFinishedCallback(stream, &PortAudioInterface::paStreamFinished);

	if (err != paNoError)
	{
		Pa_CloseStream(stream);
		stream = 0;
		std::cout << "Pa_SetStreamFinishedCallback" << std::endl;
		system("PAUSE");
		return false;
	}
	primaryBufferPtr.reset(new PrimaryBuffer(stream));

	return true;
}

bool  PortAudioInterface::closeStream()
{
	/*if (stream == 0)
	return false;*/

	PaError err = Pa_CloseStream(stream);
	stream = 0;
	primaryBufferPtr.reset();
	return (err == paNoError);
}

PortAudioInterface::PortAudioInterface() : endianFlag(checkEndian()), stream(0) {

	PaError  err = Pa_Initialize();
	if (err != paNoError) {
		fprintf(stderr, "An error occured while using the portaudio stream\n");
		fprintf(stderr, "Error number: %d\n", err);
		fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
		exit(1);//don't call Pa_terminate
	};

	int deviceIndex = searchAppropriateDevice();
	std::cout << "device index" << deviceIndex;

	//3: MME
	//7 , 8,: directsound
	//10, 13 : WDM-KS


	if (!openStream(deviceIndex))
	{
		system("pause");
		AudioError::exit(1);
	};

	//streamEvent = CreateEventA(NULL, FALSE, FALSE, NULL);

}

PortAudioInterface::~PortAudioInterface() {
	if (stream)
		closeStream();
	//CloseHandle(streamEvent);
	Pa_Terminate();
}


uint32_t  PortAudioInterface::add(std::TSTRING filePath,
	bool loop, ReadingMethod  method) {
	return 	primaryBufferPtr->addSecodaryBuffer(filePath, loop, method);
}
/*void  PortAudioInterface::startStream()
{
	PaError err;
	if (Pa_IsStreamStopped(stream) == 1) {
		err = Pa_StartStream(stream);
		AudioError::judgeError(err);
	}
}

void  PortAudioInterface::stopStream()
{

	PaError err;
	if (Pa_IsStreamStopped(stream) == 0) {
		err = Pa_StopStream(stream);
		AudioError::judgeError(err);
	}
}*/
void  PortAudioInterface::start(uint32_t id) {
	primaryBufferPtr->playBuffer(id);
}

uint16_t  PortAudioInterface::stop(uint32_t id) {
return 	primaryBufferPtr->stopBuffer(id);
}
void  PortAudioInterface::remove(uint32_t id) {
	primaryBufferPtr->removeBuffer(id);
}
void  PortAudioInterface::statusCheck() {
	if (Pa_IsStreamStopped(stream))
		printf("stream is stopped\n");
	else printf("stream is runing\n");

	if (Pa_IsStreamActive(stream))
		printf("stream is active\n");
	else printf("stream is not active\n");
}