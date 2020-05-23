#pragma once
#include "audioBufferPoolInterface.h"
#include "alsaApiInterface.h"
#include "threadManagementInterface.h"
#include "noCopyAndMoveTemplate.h"
#include "epollInterfaceBase.h"
#include "pollingFdBase.h"
#ifdef _DEBUG
#include "eventFdInterface.h"
#include "timerFdInterface.h"
#endif

#define AUDIO_INTERFACE AudioInterface::getInstance()

class AudioInterface : public NoCopyAndMove<AudioInterface>,
                       public MyUtility::EpollInterfaceBase {

private:
  AudioBufferPoolInterface audioBufferPoolInterface;
  AlsaApiInterface alsaApiInterface;
  MyUtility::PthreadWrapper bufferPoolThreadObject;
  MyUtility::PthreadWrapper bufferPlayThreadObject;

  MyUtility::EventFdInterface bufferPoolThreadReadyEventObject;
  MyUtility::EventFdInterface bufferPlayThreadEndEventObject;

  std::unique_ptr<struct pollfd> pollFdArrayPtr;
  std::int16_t pollFdCount = 0;
  std::unique_ptr<MyUtility::PollingFdBase> pollingFdInterfacePtr;
  
  AudioInterface() = default;
  std::uint16_t bufferPoolThreadReadyEventTask(bool &readyFlag);

  std::uint16_t startAudioPlay();
  std::uint16_t prepareAlsaPoll();
  std::uint16_t fillDeviceBufferInNormalWrite(std::uint32_t count,
                                              bool &playEndFlag);
  std::uint16_t fillDeviceBufferInMMapWrite(std::uint32_t count,
                                            bool &playEndFlag);

  std::uint16_t taskAfterPlayEnd();
  std::uint16_t callbackWriteTask(bool &playEndFlag);
  std::uint16_t epollCallbackEventTask(bool &playEndFlag);
  std::uint16_t epollCallbackSignalTask(bool &playEndFlag);

  std::uint16_t epollCallbackTask();
  std::uint16_t epollAlsaPollTask(bool &eventComingFlag);

  std::uint16_t writeLittleLoopInEpoll(std::int16_t *dataPtr,
                                       bool &rewriteFlag);

  std::uint16_t simpleWritePlay();
  std::uint16_t playLoopInEpoll();
  std::uint16_t mmapWritePlay();

  std::uint16_t bufferPlayThreadEndEventTask(bool &bufferPlayThreadEndFlag);
  std::uint16_t finalizeAlsaPoll();
  ~AudioInterface();

public:
  static AudioInterface &getInstance();
  std::uint16_t initialize();
  AlsaApiInterface &getAlsaApiInterfaceRef() noexcept;

  MyUtility::PollingFdBase *getPollingFdInterfacePtr();
  std::uint16_t add(std::uint32_t &fileId, std::string &&filePath, bool loop,
                    DataReadingMethod method);
  std::uint16_t start(std::uint32_t fileId);
  std::uint16_t stop(std::uint32_t fileId);
  std::uint16_t remove(std::uint32_t fileId);
  std::uint16_t allRemove();
  std::uint16_t writeBigLoopInEpoll(std::int16_t *audioBufferPtr);

  std::uint16_t audioBufferPoolThreadFuncContent();
  std::uint16_t audioBufferPlayThreadFuncContent();

  std::uint16_t finalize();
};
