#include "audioInterface.h"
#include <process.h>
#include "win32ApiWrapper/eventApi.h"
#include "windows/timeMeasurement.h"
#include "debugMacro.h"

static 	const LPCTSTR  threadTaskNameConstants = TEXT("");

AudioInterface & AudioInterface::getInstance() {
	static AudioInterface instance;
	return instance;
}

std::uint16_t AudioInterface::initialize() {
	if (audioPlayThreadStartEvent.initialize(nullptr, false, false, nullptr))
		return 1;
	if (audioBufferPoolThreadStartEvent.initialize(nullptr, false, false, nullptr))
		return 1;

	if (bufferPoolThreadReadyEvent.initialize(nullptr, false, false, nullptr))
		return 1;

	if (bufferPlayThreadEndEvent.initialize(nullptr, false, false, nullptr))
		return 1;

	if (win32AudioApiInterface.initialize())
		return 1;

	if (audioBufferPoolInterface.initialize(win32AudioApiInterface.getDeviceChannelQuantity(),
		win32AudioApiInterface.getDeviceSamplingRate(),
		win32AudioApiInterface.getFramesPerPeriod()))
		return 1;

	if (threadsState == AudioThreadsState::always)
	{
		if (makeThreads())
			return 1;
	}
	return 0;
}

std::uint16_t AudioInterface::audioBufferPoolThreadFuncMainContent() {
	DEBUG_PRINT(TEXT("audioBufferPoolThread start\n"));
	for (;;) {
		DEBUG_PRINT(TEXT("audioBufferPoolThread for main loop start point\n"));
		bool writeContinueFlag = false;
		if (audioBufferPoolInterface.fillAudioBufferPool(writeContinueFlag))
			return 1;
		if (bufferPoolThreadReadyEvent.setEventOn())
			return 1;
		DEBUG_PRINT(TEXT("audioBufferPoolThread:bufferPoolThreadReadyEvent was set.\n"));

		if (audioBufferPoolInterface.poolThreadTask())
			return 1;

		DEBUG_PRINT(TEXT("audioBufferPoolThread:pool thread task ended.\n"));


		if (bufferPlayThreadEndEvent.waitEvent())
			return 1;

		DEBUG_PRINT(TEXT("audioBufferPoolThread:bufferPlayThreadEndEvent came.\n"));

		if (audioBufferPoolInterface.getPlayContinueFlag())
			continue;
		else
			break;
	}
	DEBUG_PRINT(TEXT("audioBufferPoolThread end"));
	return 0;
}

static unsigned __stdcall audioBufferPoolTemporaryThreadFunc(void *audioInterfaceObjPtr) {
	return reinterpret_cast<AudioInterface *>(audioInterfaceObjPtr)
		->audioBufferPoolThreadFuncMainContent();
}

std::uint16_t AudioInterface::audioBufferPoolUnendingThreadFuncContent() {
	for (;;) {
		if (audioBufferPoolThreadStartEvent.waitEvent())
			return 1;
		if (threadEndFlag)
		{
			DEBUG_PRINT(TEXT("Audio thread ended successfully."));
			break;
		}
		if (audioBufferPoolThreadFuncMainContent())
			return 1;
	}
	return 0;
}

static unsigned __stdcall audioBufferPoolUnendingThreadFunc(void *audioInterfaceObjPtr) {
	return reinterpret_cast<AudioInterface *>(audioInterfaceObjPtr)
		->audioBufferPoolUnendingThreadFuncContent();
}

Win32AudioSystem::ApiInterface& AudioInterface::getWin32AudioApiInterfaceRef() noexcept {
	return win32AudioApiInterface;
}

static std::uint16_t
writeDataToAudioDeviceBuffer(void *audioInterfaceObjectPtr,
	std::int16_t *audioBufferPtr) {
	return reinterpret_cast<AudioInterface*>(audioInterfaceObjectPtr)->
		getWin32AudioApiInterfaceRef().writeDataToAudioDeviceBuffer(audioBufferPtr);
}


std::uint16_t AudioInterface::writeInitialDataToAudioDevice(bool & poolEndFlag) {
	std::uint32_t u = 0;
	std::uint32_t count = win32AudioApiInterface.getAudioBufferQuantity();
	for (u = 0; u < count; ++u) {
		if (audioBufferPoolInterface.readTask(writeDataToAudioDeviceBuffer,
			this, poolEndFlag))
			return 1;
		win32AudioApiInterface.updateAudioDeviceBufferCurrentIndex();
		if (poolEndFlag)
			break;
	}
	win32AudioApiInterface.setAudioDeviceBufferEndMaxCount(u);
	return 0;
}

MSG&	AudioInterface::getMessageInfo() noexcept { return messageInfo; }

static std::uint16_t messageNotificationWriteTask(void *audioInterfaceObjectPtr,
	std::int16_t *audioBufferPtr) {
	return reinterpret_cast<AudioInterface*>(audioInterfaceObjectPtr)->
		getWin32AudioApiInterfaceRef().writeDataToAudioDeviceBufferWrap(
			reinterpret_cast<AudioInterface*>(audioInterfaceObjectPtr)->getMessageInfo().wParam,
			reinterpret_cast<AudioInterface*>(audioInterfaceObjectPtr)->getMessageInfo().lParam,
			audioBufferPtr
		);
}

std::uint16_t AudioInterface::messageNotificationThreadFuncMainContent(bool& poolEndFlag) {
	for (;;) {
		if (win32AudioApiInterface.preparePlayByThreadNotification(audioPlayThread.getThreadId()))
			return 1;

		if (bufferPoolThreadReadyEvent.waitEvent())
			return 1;
		poolEndFlag = false;
		if (writeInitialDataToAudioDevice(poolEndFlag))
			return 1;

		for (;;) {
			if (win32AudioApiInterface.audioMessageTask(
				messageInfo))
				return 1;
			if (!poolEndFlag) {
				if (audioBufferPoolInterface.readTask(messageNotificationWriteTask,
					this, poolEndFlag))
					return 1;
			}
			else {
				if (win32AudioApiInterface.checkPlayEndAfterPoolEnd())
					break;
			}
		}
		if (win32AudioApiInterface.taskAfterPlayEnd())
			return 1;
		if (bufferPlayThreadEndEvent.setEventOn())
			return 1;
		DEBUG_PRINT(
			TEXT("audioBufferPlayThread: bufferPlayThreadEndEvent was written.\n"));
		if (audioBufferPoolInterface.getPlayContinueFlag())
			continue;
		else
			break;
	}
	return 0;
}


std::uint16_t AudioInterface::messageNotificationTemporaryThreadFunc() {
	bool poolEndFlag = false;
	//make message queue of calling thread
	::PeekMessage(&messageInfo, reinterpret_cast<HWND>(-1), 0, 0, PM_NOREMOVE);
	if (messageNotificationThreadFuncMainContent(poolEndFlag))
		return 1;
	return 0;
}

std::uint16_t AudioInterface::messageNotificationUnendingThreadFunc() {
	bool poolEndFlag = false;
	//make message queue of calling thread
	::PeekMessage(&messageInfo, reinterpret_cast<HWND>(-1), 0, 0, PM_NOREMOVE);

	for (;;) {
		if (audioPlayThreadStartEvent.waitEvent())
			return 1;
		if (threadEndFlag)
		{
			DEBUG_PRINT(TEXT("Audio thread ended successfully."));
			return 0;
		}
		if (messageNotificationThreadFuncMainContent(poolEndFlag))
			return 1;
	}
	return 0;
}

std::uint16_t AudioInterface::callbackEventThreadFuncMainContent(bool& poolEndFlag) {
	for (;;) {
		if (win32AudioApiInterface.preparePlayByCallbackNotification())
			return 1;

		if (bufferPoolThreadReadyEvent.waitEvent())
			return 1;

		poolEndFlag = false;
		if (writeInitialDataToAudioDevice(poolEndFlag))
			return 1;

		for (;;) {
			if (win32AudioApiInterface.audioEventTask())
				return 1;
			if (!poolEndFlag) {
				if (audioBufferPoolInterface.readTask(writeDataToAudioDeviceBuffer,
					this, poolEndFlag))
					return 1;
				win32AudioApiInterface.updateAudioDeviceBufferCurrentIndex();
			}
			else {
				win32AudioApiInterface.updateAudioDeviceBufferCurrentIndex();
				if (win32AudioApiInterface.checkPlayEndAfterPoolEnd())
					break;
			}
		}
		win32AudioApiInterface.taskAfterPlayEnd();
		if (bufferPlayThreadEndEvent.setEventOn())
			return 1;
		DEBUG_PRINT(
			TEXT("audioBufferPlayThread: bufferPlayThreadEndEvent was written.\n"));
		if (audioBufferPoolInterface.getPlayContinueFlag())
			continue;
		else
			break;
	}
	return 0;
}


std::uint16_t AudioInterface::callbackEventTemporaryThreadFunc() {
	bool poolEndFlag = false;
	if (callbackEventThreadFuncMainContent(poolEndFlag))
		return 1;
	return 0;
}

std::uint16_t AudioInterface::callbackEventUnendingThreadFunc() {
	bool poolEndFlag = false;
	for (;;) {
		if (audioPlayThreadStartEvent.waitEvent())
			return 1;
		if (threadEndFlag)
		{
			DEBUG_PRINT(TEXT("Audio thread ended successfully."));
			return 0;
		}
		if (callbackEventThreadFuncMainContent(poolEndFlag))
			return 1;
	}
	return 0;
}

static unsigned __stdcall messageNotificationTemporaryThreadFunc(void * audioInterfaceObjPtr) {
	return reinterpret_cast<AudioInterface*>(audioInterfaceObjPtr)->messageNotificationTemporaryThreadFunc();
}

unsigned __stdcall messageNotificationUnendingThreadFunc(void * audioInterfaceObjPtr) {
	return reinterpret_cast<AudioInterface*>(audioInterfaceObjPtr)->messageNotificationUnendingThreadFunc();
}

static unsigned __stdcall callbackEventTemporaryThreadFunc(void * audioInterfaceObjPtr) {
	return 		reinterpret_cast<AudioInterface*>(audioInterfaceObjPtr)->callbackEventTemporaryThreadFunc();
}

static unsigned __stdcall callbackEventUnendingThreadFunc(void * audioInterfaceObjPtr) {
	return 		reinterpret_cast<AudioInterface*>(audioInterfaceObjPtr)->callbackEventUnendingThreadFunc();
}

std::uint16_t AudioInterface::makeThreads() {
	if (threadsState == AudioThreadsState::always) {
		if (audioBufferPoolThread.initialize(audioBufferPoolUnendingThreadFunc, this, threadTaskNameConstants))
			return 1;
	}
	else {
		if (audioBufferPoolThread.initialize(audioBufferPoolTemporaryThreadFunc, this, threadTaskNameConstants))
			return 1;
	}
	switch (win32AudioApiInterface.getPlayDoneNotificationMethod()) {
	case Win32AudioSystem::PlayDoneNotificationType::thread:
		if (threadsState == AudioThreadsState::always)
		{
			if (audioPlayThread.initialize(::messageNotificationUnendingThreadFunc, this, threadTaskNameConstants))
				return 1;
		}
		else {
			if (audioPlayThread.initialize(::messageNotificationTemporaryThreadFunc, this, threadTaskNameConstants))
				return 1;
		}
		break;
	case Win32AudioSystem::PlayDoneNotificationType::callback:
		if (threadsState == AudioThreadsState::always)
		{
			if (audioPlayThread.initialize(::callbackEventUnendingThreadFunc, this, threadTaskNameConstants))
				return 1;
		}
		else {
			if (audioPlayThread.initialize(::callbackEventTemporaryThreadFunc, this, threadTaskNameConstants))
				return 1;
		}
		break;
	}
	return 0;
}

std::uint16_t AudioInterface::add(std::uint32_t &fileId, std::TSTRING &&filePath,
	bool loop, DataReadingMethod method) {
	return audioBufferPoolInterface.getPrimaryBufferRef().add(fileId, std::move(filePath), loop, method);
}

std::uint16_t AudioInterface::start(std::uint32_t fileId) {
	DEBUG_PRINT(TEXT("AudioInterface::start()\n"));
	bool audioPlayStartFlag = false;
	if (audioBufferPoolInterface.start(fileId, audioPlayStartFlag))
		return 1;
	if (audioPlayStartFlag)
		if (startAudioPlay())
			return 1;

	return 0;
}

std::uint16_t AudioInterface::stop(std::uint32_t fileId) {
	return audioBufferPoolInterface.getPrimaryBufferRef().stop(fileId);
}

std::uint16_t AudioInterface::remove(uint32_t fileId) {
	return audioBufferPoolInterface.getPrimaryBufferRef().remove(fileId);
}

std::uint16_t AudioInterface::allRemove() { return audioBufferPoolInterface.getPrimaryBufferRef().allRemove(); }

std::uint16_t AudioInterface::startAudioPlay() {
	if (threadsState == AudioThreadsState::always)
	{
		if (audioBufferPoolThreadStartEvent.setEventOn())
			return 1;
		if (audioPlayThreadStartEvent.setEventOn())
			return 1;
	}
	else if (threadsState == AudioThreadsState::temporary) {
		if (audioPlayThread.finalize())
			return 1;
		if (audioBufferPoolThread.finalize())
			return 1;
		if (makeThreads())
			return 1;
	}
	return 0;
}

std::uint16_t AudioInterface::finalize() {
	DEBUG_PRINT(TEXT("AudioInterface finalize start.\n"));
	if (allRemove())
		return 1;

	if (threadsState == AudioThreadsState::always)
	{
		threadEndFlag = true;
		if (audioPlayThreadStartEvent.setEventOn())
			return 1;
		if (audioBufferPoolThreadStartEvent.setEventOn())
			return 1;
	}
	if (audioBufferPoolThread.finalize())
		return 1;
	if (audioPlayThread.finalize())
		return 1;

	if (audioPlayThreadStartEvent.finalize())
		return 1;
	if (audioBufferPoolThreadStartEvent.finalize())
		return 1;

	if (bufferPoolThreadReadyEvent.finalize())
		return 1;
	if (bufferPlayThreadEndEvent.finalize())
		return 1;

	if (win32AudioApiInterface.finalize())
		return 1;
	if (audioBufferPoolInterface.finalize())
		return 1;

	return 0;
}

AudioInterface::~AudioInterface() {
}
