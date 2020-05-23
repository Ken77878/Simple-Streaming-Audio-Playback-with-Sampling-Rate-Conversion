#pragma once
#include "audioInterfaceDLLHeader.h"
#include "noCopyAndMoveTemplate.h"
#include "win32AudioApiInterface.h"
#include "audioBufferPoolInterface.h"
#include "audioThreadsStateEnumeration.h"
#include "threadManagementInterface.h"
#include "eventManagementInterface.h"
#include <atomic>

#define AUDIO_INTERFACE AudioInterface::getInstance()

class audioInterface_API AudioInterface : public NoCopyAndMove<AudioInterface> {
	friend class NoCopyAndMove<AudioInterface>;

private:
	Win32AudioSystem::ApiInterface win32AudioApiInterface;
	AudioBufferPoolInterface audioBufferPoolInterface;
	AudioThreadsState threadsState = AudioThreadsState::always;
	MyUtility::EventManagementInterface audioPlayThreadStartEvent;
	MyUtility::EventManagementInterface audioBufferPoolThreadStartEvent;

	MyUtility::EventManagementInterface bufferPoolThreadReadyEvent;
	MyUtility::EventManagementInterface bufferPlayThreadEndEvent;

	MyUtility::ThreadManagementInterface audioBufferPoolThread;
	MyUtility::ThreadManagementInterface audioPlayThread;
	std::atomic<bool> threadEndFlag = false;
	MSG messageInfo = { 0 };

	AudioInterface() = default;
	std::uint16_t messageNotificationThreadFuncMainContent(bool& poolEndFlag);
	std::uint16_t callbackEventThreadFuncMainContent(bool& poolEndFlag);
	std::uint16_t makeThreads();
	std::uint16_t writeInitialDataToAudioDevice(bool & poolEndFlag);
	~AudioInterface();
public:
	static AudioInterface &getInstance();
std::uint16_t initialize();
	
	std::uint16_t audioBufferPoolThreadFuncMainContent();
	std::uint16_t audioBufferPoolUnendingThreadFuncContent();
	Win32AudioSystem::ApiInterface&  getWin32AudioApiInterfaceRef() noexcept;
	MSG&	getMessageInfo() noexcept;
	std::uint16_t messageNotificationTemporaryThreadFunc();
	std::uint16_t messageNotificationUnendingThreadFunc();
	std::uint16_t callbackEventTemporaryThreadFunc();
	std::uint16_t callbackEventUnendingThreadFunc();

	std::uint16_t add(std::uint32_t &fileId, std::TSTRING &&filePath, bool loop,
		DataReadingMethod method);
	std::uint16_t start(std::uint32_t fileId);
	std::uint16_t stop(std::uint32_t fileId);
	std::uint16_t remove(std::uint32_t fileId);
	std::uint16_t allRemove();
	std::uint16_t startAudioPlay();
	std::uint16_t finalize();
};
