#include "searchAppropriateDevice.h"
#include "stdio.h"
#include "portaudio.h"
#include <stdlib.h>

void printError(PaError err) {
	Pa_Terminate();
	fprintf(stderr, "Error number: %d\n", err);
	fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
	exit(1);
}

int searchAppropriateDevice() {
	int      numDevices;
	const   PaDeviceInfo *deviceInfo;
	PaError err;
	
	numDevices = Pa_GetDeviceCount();
	if (numDevices < 0)
	{
		printf("ERROR: Pa_GetDeviceCount returned 0x%x\n", numDevices);
		err = numDevices;
		printError(err);
	}

	printf("Number of devices = %d\n", numDevices);
	for (int i = 0; i < numDevices; i++)
	{
		deviceInfo = Pa_GetDeviceInfo(i);
		printf("--------------------------------------- device #%d\n", i);

#ifdef _WIN32
		if (Pa_GetHostApiInfo(deviceInfo->hostApi)->type == PaHostApiTypeId::paDirectSound &&
			Pa_GetHostApiInfo(deviceInfo->hostApi)->defaultOutputDevice == i)
			return i;
#else
	if (Pa_GetHostApiInfo(deviceInfo->hostApi)->type == PaHostApiTypeId::paALSA &&
			Pa_GetHostApiInfo(deviceInfo->hostApi)->defaultOutputDevice == i)
			return i;
#endif


	}
#ifdef _WIN32
	printf("device error: DirectSound output device don't exist.");
	Pa_Terminate();
	exit(1);
#else
printf("device error: ALSA output device don't exist.");
	Pa_Terminate();
	exit(1);
#endif

}
