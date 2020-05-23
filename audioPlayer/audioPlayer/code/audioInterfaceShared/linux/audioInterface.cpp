#include "audioInterface.h"
#include "threadManagementInterface.h"
#include "eventFdInterface.h"
#include "signalFdInterface.h"
#include "linuxApiWrapper/fileDescriptorApi.h"
#include <vector>
#include "debugMacro.h"
#include "timepieceInterface.h"

AudioInterface & AudioInterface::getInstance() {
	static AudioInterface instance;
	return instance;
}

std::uint16_t AudioInterface::prepareAlsaPoll() {
  if (alsaApiInterface.initializePollFd(pollFdArrayPtr, pollFdCount))
    return 1;
  availableEpollEventArray.resize(pollFdCount);
  maxEvents += pollFdCount;
  struct epoll_event epollEvent;
  memset(&epollEvent, 0, sizeof(epoll_event));

  DEBUG_PRINT_ARGS("pollFdCount: %u\n", pollFdCount);
  DEBUG_PRINT_ARGS("pollFd: %d\n", (pollFdArrayPtr.get())[0].fd);
  for (std::uint32_t u = 0; u < static_cast<std::uint32_t>(pollFdCount); ++u) {
    epollEvent.data.fd = (pollFdArrayPtr.get())[u].fd;
    if ((pollFdArrayPtr.get())[u].events & 1) {
      epollEvent.events = EPOLLIN;
    } else if ((pollFdArrayPtr.get())[u].events & 4)
      epollEvent.events = EPOLLOUT;
    else {
      fprintf(stderr, "pollFdArray: wrong event only\n");
      return 1;
    }

    if (registerFileDescriptor((pollFdArrayPtr.get())[u].fd, epollEvent)) {
      return 1;
    }
  }
  return 0;
}

static void asyncCallback(snd_async_handler_t *ahandler) {
  AudioInterface *audioObjectPtr = reinterpret_cast<AudioInterface *>(
      snd_async_handler_get_callback_private(ahandler));
  if (dynamic_cast<MyUtility::EventFdInterface *>(
          audioObjectPtr->getPollingFdInterfacePtr())
          ->writeTask())
    _exit(343);
}

std::uint16_t AudioInterface::initialize() {
  if (alsaApiInterface.openPcmHandle())
    return 1;
  if (alsaApiInterface.initializeParams())
    return 1;

  if (audioBufferPoolInterface.initialize(
          alsaApiInterface.getDeviceChannelQuantity(),
          alsaApiInterface.getDeviceSamplingRate(),
          alsaApiInterface.getFramesPerPeriod()))
    return 1;
  if (MyUtility::EpollInterfaceBase::initialize())
    return 1;

  if (bufferPoolThreadReadyEventObject.initialize())
    return 1;
  if (registerFileDescriptor(
          bufferPoolThreadReadyEventObject.getFileDescriptor()))
    return 1;
  if (bufferPlayThreadEndEventObject.initialize())
    return 1;
  if (registerFileDescriptor(
          bufferPlayThreadEndEventObject.getFileDescriptor()))
    return 1;

  switch (alsaApiInterface.getAlsaWriteLoopMode()) {
  case AlsaWriteLoopMode::alsaPoll:
    if (prepareAlsaPoll())
      return 1;
    break;

  case AlsaWriteLoopMode::callbackEvent:
    pollingFdInterfacePtr.reset(new MyUtility::EventFdInterface);
    if (dynamic_cast<MyUtility::EventFdInterface *>(pollingFdInterfacePtr.get())
            ->initialize())
      return 1;
    if (registerFileDescriptor(pollingFdInterfacePtr->getFileDescriptor()))
      return 1;
    break;
  case AlsaWriteLoopMode::callbackSignal:
    pollingFdInterfacePtr.reset(new MyUtility::SignalFdInterface);
    std::vector<std::int32_t> signalNoArray;
    if (alsaApiInterface.initializeAsyncHandlerSignalNumber(asyncCallback,
                                                            this))
      return 1;
    signalNoArray.push_back(alsaApiInterface.getCallbackSignalNo());

    if (dynamic_cast<MyUtility::SignalFdInterface *>(
            pollingFdInterfacePtr.get())
            ->initialize(signalNoArray))
      return 1;
    if (registerFileDescriptor(pollingFdInterfacePtr->getFileDescriptor()))
      return 1;

    break;
  }

  return 0;
}

AlsaApiInterface &AudioInterface::getAlsaApiInterfaceRef() noexcept {
  return alsaApiInterface;
}
MyUtility::PollingFdBase *AudioInterface::getPollingFdInterfacePtr() {
  return pollingFdInterfacePtr.get();
}

std::uint16_t AudioInterface::add(std::uint32_t &fileId, std::string &&filePath,
                                  bool loop, DataReadingMethod method) {
  return audioBufferPoolInterface.getPrimaryBufferRef().add(
      fileId, std::move(filePath), loop, method);
}

std::uint16_t AudioInterface::start(std::uint32_t fileId) {
  DEBUG_PRINT("AudioInterface::start.\n");
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

std::uint16_t AudioInterface::allRemove() {
  return audioBufferPoolInterface.getPrimaryBufferRef().allRemove();
}

std::uint16_t AudioInterface::taskAfterPlayEnd() {
  if (alsaApiInterface.stopPcm()) {
    return 1;
  }
  return 0;
}

static std::uint16_t normalWriteBeforeStart(void *audioInterfaceObjectPtr,
                                            std::int16_t *audioBufferPtr) {

  return reinterpret_cast<AudioInterface *>(audioInterfaceObjectPtr)
      ->getAlsaApiInterfaceRef()
      .normalWriteBeforeStart(audioBufferPtr);
}

std::uint16_t AudioInterface::fillDeviceBufferInNormalWrite(std::uint32_t count,
                                                            bool &playEndFlag) {
  for (std::uint32_t u = 0; u < (count + 1); ++u) {
#ifdef _DEBUG
    alsaApiInterface.printPcmState();
#endif
    if (audioBufferPoolInterface.readTask(normalWriteBeforeStart, this,
                                          playEndFlag))
      return 1;
    if (playEndFlag)
      break;
  }
#ifdef _DEBUG
  alsaApiInterface.printPcmState();
#endif
  DEBUG_PRINT("fillDeviceBufferInNormalWrite end\n");
  return 0;
}

static std::uint16_t writeInMmapMode(void *audioInterfaceObjectPtr,
                                     std::int16_t *audioBufferPtr) {

  return reinterpret_cast<AudioInterface *>(audioInterfaceObjectPtr)
      ->getAlsaApiInterfaceRef()
      .writeInMmapMode(audioBufferPtr);
}

std::uint16_t AudioInterface::fillDeviceBufferInMMapWrite(std::uint32_t count,
                                                          bool &playEndFlag) {
  for (std::uint32_t u = 0; u < count; ++u) {
#ifdef _DEBUG
    alsaApiInterface.printPcmState();
#endif
    if (audioBufferPoolInterface.readTask(writeInMmapMode, this, playEndFlag))
      return 1;
    if (playEndFlag)
      break;
  }
#ifdef _DEBUG
  alsaApiInterface.printPcmState();
#endif
  DEBUG_PRINT("fillDeviceBufferInMmapWrite end\n");
  return 0;
}

std::uint16_t AudioInterface::epollAlsaPollTask(bool &eventComingFlag) {
  bool pollFdsComingFlag = false;
  std::uint32_t pollFdArrayIndex = 0;
  std::uint32_t events = 0;
  for (std::uint32_t u = 0; u < static_cast<std::uint32_t>(fdQuantity); ++u) {
    for (std::uint32_t v = 0; v < static_cast<std::uint32_t>(pollFdCount);
         ++v) {
      if (availableEpollEventArray[u].data.fd != (pollFdArrayPtr.get())[v].fd)
        continue;
      else {
        pollFdsComingFlag = true;
        pollFdArrayIndex = v;
        break;
      }
    }
    if (!pollFdsComingFlag)
      continue;
    if (checkEpollEventError(availableEpollEventArray[u].events))
      return 1;
    if ((pollFdArrayPtr.get())[pollFdArrayIndex].events & 1) {
      events = EPOLLIN;
    } else if ((pollFdArrayPtr.get())[u].events & 4)
      events = EPOLLOUT;

    if (availableEpollEventArray[u].events & events) {
      eventComingFlag = true;
      return 0;
    } else {
      fprintf(stderr, "unknown event came.\n");
      return 1;
    }
  }
  return 0;
}

static std::uint16_t writeInCallbackMode(void *audioInterfaceObjectPtr,
                                         std::int16_t *audioBufferPtr) {

  return reinterpret_cast<AudioInterface *>(audioInterfaceObjectPtr)
      ->getAlsaApiInterfaceRef()
      .writeInCallbackMode(audioBufferPtr);
}

std::uint16_t AudioInterface::callbackWriteTask(bool &playEndFlag) {
  DEBUG_PRINT("callbackWriteTask end\n");

  if (getAlsaApiInterfaceRef().possibleWrittenSizeIsBiggerThanPeriod()) {
    return 1;
  }

  if (!getAlsaApiInterfaceRef().getBiggerSizeThanPeriodFlag()) {
    return 0;
  }
  if (audioBufferPoolInterface.readTask(writeInCallbackMode, this, playEndFlag))
    return 1;
  return 0;
}

std::uint16_t AudioInterface::epollCallbackEventTask(bool &playEndFlag) {
  for (std::uint32_t u = 0; u < static_cast<std::uint32_t>(fdQuantity); ++u) {
    if (availableEpollEventArray[u].data.fd !=
        pollingFdInterfacePtr->getFileDescriptor())
      continue;

    if (checkEpollEventError(availableEpollEventArray[u].events))
      return 1;
    if (availableEpollEventArray[u].events & EPOLLIN) {
      if (dynamic_cast<MyUtility::EventFdInterface *>(
              pollingFdInterfacePtr.get())
              ->readTask())
        return 1;
      if (callbackWriteTask(playEndFlag))
        return 1;
    } else {
      fprintf(stderr, "Unknown epoll event came.\n");
      return 1;
    }
  }
  return 0;
}

std::uint16_t AudioInterface::epollCallbackSignalTask(bool &playEndFlag) {
  for (std::uint32_t u = 0; u < static_cast<std::uint32_t>(fdQuantity); ++u) {
    if (availableEpollEventArray[u].data.fd !=
        pollingFdInterfacePtr->getFileDescriptor())
      continue;

    if (checkEpollEventError(availableEpollEventArray[u].events))
      return 1;
    if (availableEpollEventArray[u].events & EPOLLIN) {
      if (dynamic_cast<MyUtility::SignalFdInterface *>(
              pollingFdInterfacePtr.get())
              ->readTask())
        return 1;
      if (callbackWriteTask(playEndFlag))
        return 1;
    } else {
      fprintf(stderr, "Unknown epoll event came.\n");
      return 1;
    }
  }
  return 0;
}

std::uint16_t AudioInterface::epollCallbackTask() {
  bool playEndFlag = false;
  bool eventComingFlag = false;
  while (!playEndFlag) {
    if (waitEpollEvent())
      return 1;
    switch (alsaApiInterface.getAlsaWriteLoopMode()) {
    case AlsaWriteLoopMode::callbackEvent:
      if (epollCallbackEventTask(playEndFlag))
        return 1;
      break;
    case AlsaWriteLoopMode::callbackSignal:
      if (epollCallbackSignalTask(playEndFlag))
        return 1;
      break;
    }
  }
  return 0;
}

std::uint16_t AudioInterface::bufferPoolThreadReadyEventTask(bool &readyFlag) {
  for (std::uint32_t u = 0; u < static_cast<std::uint32_t>(fdQuantity); ++u) {
    if (availableEpollEventArray[u].data.fd !=
        bufferPoolThreadReadyEventObject.getFileDescriptor())
      continue;
    if (checkEpollEventError(availableEpollEventArray[u].events))
      return 1;
    if (availableEpollEventArray[u].events & EPOLLIN) {
      if (bufferPoolThreadReadyEventObject.readTask())
        return 1;
      readyFlag = true;
      break;
    } else {
      fprintf(stderr, "Unknown epoll event came.\n");
      return 1;
    }
  }
  return 0;
}

std::uint16_t
AudioInterface::bufferPlayThreadEndEventTask(bool &bufferPlayThreadEndFlag) {
  for (std::uint32_t u = 0; u < static_cast<std::uint32_t>(fdQuantity); ++u) {
    if (availableEpollEventArray[u].data.fd !=
        bufferPlayThreadEndEventObject.getFileDescriptor())
      continue;
    if (checkEpollEventError(availableEpollEventArray[u].events))
      return 1;
    if (availableEpollEventArray[u].events & EPOLLIN) {
      if (bufferPlayThreadEndEventObject.readTask())
        return 1;
      bufferPlayThreadEndFlag = true;
      break;
    } else {
      fprintf(stderr, "Unknown epoll event came.\n");
      return 1;
    }
  }
  return 0;
}

static std::uint16_t writeInSimpleMode(void *audioInterfaceObjectPtr,
                                       std::int16_t *audioBufferPtr) {
  return reinterpret_cast<AudioInterface *>(audioInterfaceObjectPtr)
      ->getAlsaApiInterfaceRef()
      .writeInSimpleMode(audioBufferPtr);
}

std::uint16_t AudioInterface::simpleWritePlay() {
  bool playEndFlag = false;
  do {
    if (audioBufferPoolInterface.readTask(writeInSimpleMode, this, playEndFlag))
      return 1;
  } while (!playEndFlag);
  return 0;
}

std::uint16_t AudioInterface::mmapWritePlay() {
  bool playEndFlag = false;
  do {
    if (audioBufferPoolInterface.readTask(writeInMmapMode, this, playEndFlag))
      return 1;
  } while (!playEndFlag);
  return 0;
}

std::uint16_t AudioInterface::writeLittleLoopInEpoll(std::int16_t *dataPtr,
                                                     bool &rewriteFlag) {
  std::int16_t *currentDataPtr = dataPtr;
  std::uint32_t writtenFrames = alsaApiInterface.getFramesPerPeriod();
  bool writeEnd = false;
  bool eventComingFlag = false;
  while (!writeEnd) {
    if (alsaApiInterface.writeInPollMode(currentDataPtr, writtenFrames,
                                         writeEnd, rewriteFlag))
      return 1;
    if (writeEnd)
      break;

    for (;;) {
      if (waitEpollEvent()) {
        if (alsaApiInterface.pollErrorTask())
          return 1;
      }
      if (epollAlsaPollTask(eventComingFlag))
        return 1;
      if (eventComingFlag) {
        eventComingFlag = false;
        break;
      }
    }
  }
  return 0;
}

std::uint16_t
AudioInterface::writeBigLoopInEpoll(std::int16_t *audioBufferPtr) {
  bool rewriteFlag = false;
  do {
    if (rewriteFlag)
      rewriteFlag = false;

    if (writeLittleLoopInEpoll(audioBufferPtr, rewriteFlag))
      return 1;
  } while (rewriteFlag);
  return 0;
}

static std::uint16_t writeBigLoopInEpoll(void *audioInterfaceObjectPtr,
                                         std::int16_t *audioBufferPtr) {
  return reinterpret_cast<AudioInterface *>(audioInterfaceObjectPtr)
      ->writeBigLoopInEpoll(audioBufferPtr);
}

std::uint16_t AudioInterface::playLoopInEpoll() {
  bool playEndFlag = false;
  bool eventComingFlag = false;
  do {
    for (;;) {
      if (waitEpollEvent()) {
        if (alsaApiInterface.pollErrorTask())
          return 1;
      }

      if (epollAlsaPollTask(eventComingFlag))
        return 1;
      if (eventComingFlag) {
        eventComingFlag = false;
        break;
      }
    }
    if (audioBufferPoolInterface.readTask(::writeBigLoopInEpoll, this,
                                          playEndFlag))
      return 1;

  } while (!playEndFlag);
  DEBUG_PRINT("EpollPlayThread end.\n");
  return 0;
}

std::uint16_t AudioInterface::audioBufferPoolThreadFuncContent() {
  DEBUG_PRINT("audioBufferPoolThread start\n");
  for (;;) {
    DEBUG_PRINT("audioBufferPoolThread for loop start point\n");
    bool writeContinueFlag = false;
    if (audioBufferPoolInterface.fillAudioBufferPool(writeContinueFlag))
      return 1;
    if (bufferPoolThreadReadyEventObject.writeTask())
      return 1;
    DEBUG_PRINT("audioBufferPoolThread:bufferPoolThreadReadyEvent was set.\n");

    if (audioBufferPoolInterface.poolThreadTask())
      return 1;

    DEBUG_PRINT("audioBufferPoolThread:pool thread task ended.\n");

    bool bufferPlayThreadEndFlag = false;
    do {
      if (waitEpollEvent())
        return 1;
      if (bufferPlayThreadEndEventTask(bufferPlayThreadEndFlag))
        return 1;
    } while (!bufferPlayThreadEndFlag);
    DEBUG_PRINT("audioBufferPoolThread:bufferPlayThreadEndEvent came.\n");

    if (audioBufferPoolInterface.getPlayContinueFlag())
      continue;
    else
      break;
  }
  DEBUG_PRINT("audioBufferPoolThread end");
  return 0;
}

static std::uint16_t audioBufferPoolThreadFunc(void *audioInterfaceObjPtr) {
  return reinterpret_cast<AudioInterface *>(audioInterfaceObjPtr)
      ->audioBufferPoolThreadFuncContent();
}

std::uint16_t AudioInterface::audioBufferPlayThreadFuncContent() {
  DEBUG_PRINT("audioBufferPlayThread start\n");

  bool playEndFlag = false;
  for (;;) {
    DEBUG_PRINT("audioBufferPlayThread:for loop start point.\n");

    if (alsaApiInterface.initializeParams())
      return 1;
    bool bufferPoolThreadReadyFlag = false;
    do {
      if (waitEpollEvent())
        return 1;
      if (bufferPoolThreadReadyEventTask(bufferPoolThreadReadyFlag))
        return 1;
    } while (!bufferPoolThreadReadyFlag);
    DEBUG_PRINT("audioBufferPlayThread:bufferPoolThreadReady event came.\n");

    std::uint32_t count = alsaApiInterface.getPeriodCountPerRingBuffer() - 1;

    switch (alsaApiInterface.getAlsaWriteLoopMode()) {
    case AlsaWriteLoopMode::normal: {
      if (fillDeviceBufferInNormalWrite(count, playEndFlag))
        return 1;
      if (playEndFlag) {
        playEndFlag = false;
        if (alsaApiInterface.startPcm())
          return 1;
        break;
      }

      if (simpleWritePlay())
        return 1;
    } break;
    case AlsaWriteLoopMode::callbackEvent:
      if (fillDeviceBufferInNormalWrite(count, playEndFlag))
        return 1;
      if (playEndFlag) {
        playEndFlag = false;
        if (alsaApiInterface.startPcm())
          return 1;
        break;
      }

      if (alsaApiInterface.startCallback(asyncCallback, this))
        return 1;
      if (epollCallbackTask())
        return 1;
      break;
    case AlsaWriteLoopMode::callbackSignal:
      if (fillDeviceBufferInNormalWrite(count, playEndFlag))
        return 1;
      if (playEndFlag) {
        playEndFlag = false;
        if (alsaApiInterface.startPcm())
          return 1;
        break;
      }
      if (alsaApiInterface.startCallback(asyncCallback, this))
        return 1;
      if (epollCallbackTask())
        return 1;
      break;
    case AlsaWriteLoopMode::alsaPoll:
      if (fillDeviceBufferInNormalWrite(count, playEndFlag))
        return 1;
      if (playEndFlag) {
        playEndFlag = false;
        if (alsaApiInterface.startPcm())
          return 1;
        break;
      }
      if (playLoopInEpoll())
        return 1;
      break;
    case AlsaWriteLoopMode::mmap: {
      if (fillDeviceBufferInMMapWrite(count, playEndFlag))
        return 1;
      if (playEndFlag) {
        playEndFlag = false;
        if (alsaApiInterface.startPcm())
          return 1;
        break;
      }
      if (mmapWritePlay())
        return 1;
    } break;
    default:
      break;
    }
    if (taskAfterPlayEnd())
      return 1;

    DEBUG_PRINT("audioBufferPlayThread: main task ended.\n");

	if (bufferPlayThreadEndEventObject.writeTask())
		return 1;
    DEBUG_PRINT(
        "audioBufferPlayThread: bufferPlayThreadEndEvent was written.\n");
    if (audioBufferPoolInterface.getPlayContinueFlag())
      continue;
    else
      break;
  }
  DEBUG_PRINT("audioBufferPlayThread ended.\n");
  return 0;
}

static std::uint16_t audioBufferPlayThreadFunc(void *audioInterfaceObjPtr) {
  return reinterpret_cast<AudioInterface *>(audioInterfaceObjPtr)
      ->audioBufferPlayThreadFuncContent();
}

std::uint16_t AudioInterface::startAudioPlay() {
  DEBUG_PRINT("startPrimaryBuffer() was called.\n");
  if (bufferPoolThreadObject.finalize())
    return 1;
  if (bufferPoolThreadObject.getThreadReturnValue())
    return 1;

  if (bufferPlayThreadObject.finalize())
    return 1;
  if (bufferPlayThreadObject.getThreadReturnValue()) {
    return 1;
  }

  if (bufferPoolThreadObject.initialize(audioBufferPoolThreadFunc, this))
    return 1;
  if (bufferPlayThreadObject.initialize(audioBufferPlayThreadFunc, this))
    return 1;

  return 0;
}

std::uint16_t AudioInterface::finalizeAlsaPoll() {
  for (std::uint32_t u = 0; u < static_cast<std::uint32_t>(pollFdCount); ++u) {
    if (eraseFileDescriptor((pollFdArrayPtr.get())[u].fd))
      return 1;

    if (LinuxApiWrapper::closeWrap((pollFdArrayPtr.get())[u].fd))
      return 1;
  }
  return 0;
}

std::uint16_t AudioInterface::finalize() {
  DEBUG_PRINT("AudioInterface finalize start;\m");
  if (allRemove())
    return 1;
  if (bufferPoolThreadObject.finalize())
    return 1;
  if (bufferPoolThreadObject.getThreadReturnValue())
    return 1;

  if (bufferPlayThreadObject.finalize())
    return 1;
  if (bufferPlayThreadObject.getThreadReturnValue()) {
    return 1;
    return 0;
  }

  switch (alsaApiInterface.getAlsaWriteLoopMode()) {
  case AlsaWriteLoopMode::alsaPoll:
    if (finalizeAlsaPoll())
      return 1;
    break;
  case AlsaWriteLoopMode::callbackEvent:
  case AlsaWriteLoopMode::callbackSignal:
    if (pollingFdInterfacePtr->finalize())
      return 1;
    break;
  }
  if (EpollInterfaceBase::finalize())
    return 1;

  if (bufferPoolThreadReadyEventObject.finalize())
    return 1;
  if (bufferPlayThreadEndEventObject.finalize())
    return 1;

  if (audioBufferPoolInterface.finalize())
    return 1;
  DEBUG_PRINT("AudioInterface finalize ended successfully.");
  return 0;
}
AudioInterface::~AudioInterface() {}
