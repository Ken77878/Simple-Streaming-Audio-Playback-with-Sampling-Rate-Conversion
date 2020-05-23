#include "alsaApiInterface.h"
#include <signal.h>
#include "debugMacro.h"
// aplay -v test.wav
constexpr std::uint32_t bufferTime = 500000; /* ring buffer length in us */
// If periodTime is too small, callback will hang.
constexpr std::uint32_t periodTime = 100000; /* period time in us */

constexpr snd_pcm_format_t pcmFormat = SND_PCM_FORMAT_S16_LE;

AlsaApiInterface::AlsaApiInterface() : pollFlag(false) {}

static std::uint16_t
setDeviceChannelQuantity(snd_pcm_t *pcmHandle, snd_pcm_hw_params_t *hwParams,
                         std::uint8_t &deviceChannelQuantity) {
  std::int32_t returnValue = 0;
  // set the count of channels
  if ((returnValue = snd_pcm_hw_params_set_channels(
           pcmHandle, hwParams, deviceChannelQuantity)) < 0) {
    if (deviceChannelQuantity > 2) {
      // Two channels (stereo)
      deviceChannelQuantity = 2;
      if (setDeviceChannelQuantity(pcmHandle, hwParams,
                                   deviceChannelQuantity)) {
        return 1;
      }
    } else {
      std::printf("Error occurred in setting deviceChannelQuantity.: %s\n",
                  snd_strerror(returnValue));
      return 1;
    }
  }
  DEBUG_PRINT_ARGS(TEXT("Channel quantity of device was set %u channels.\n"),
                   deviceChannelQuantity);
  return 0;
}

void AlsaApiInterface::printPcmState() {

  std::int32_t avail = snd_pcm_avail_update(pcmHandle);
  printf("avail: %d\n", avail);
  if (snd_pcm_state(pcmHandle) == SND_PCM_STATE_RUNNING)
    printf("SND_PCM_STATE_RUNNING\n");
  else if (snd_pcm_state(pcmHandle) == SND_PCM_STATE_PREPARED)
    printf("SND_PCM_STATE_PREPARED\n");
  else if (snd_pcm_state(pcmHandle) == SND_PCM_STATE_OPEN)
    printf("SND_PCM_STATE_OPEN\n");
  else if (snd_pcm_state(pcmHandle) == SND_PCM_STATE_SETUP)
    printf("SND_PCM_STATE_SETUP\n");
  else if (snd_pcm_state(pcmHandle) == SND_PCM_STATE_XRUN)
    printf("SND_PCM_STATE_XRUN\n");

  else
    printf("pcm state: %d\n", snd_pcm_state(pcmHandle));
}

std::uint16_t AlsaApiInterface::openPcmHandle() {
  //  const char *pcm_name = strdup("plughw:0,3");
  const char *pcm_name = "default";
  // SND_PCM_NONBLOCK, SND_PCM_ASYNC
  std::int32_t openModeFlag = 0;
  if ((returnValue = snd_pcm_open(&pcmHandle, pcm_name, SND_PCM_STREAM_PLAYBACK,
                                  openModeFlag)) < 0) {
    std::printf("unable to open pcm device: %s\n", snd_strerror(returnValue));
    return 1;
  }
#ifdef _DEBUG
  printPcmState();
#endif
  return 0;
}

std::uint16_t AlsaApiInterface::initializeHwParams() {
  snd_pcm_hw_params_alloca(&hwParams);
  // choose all parameters
  // Fill it in with default values.
  if ((returnValue = snd_pcm_hw_params_any(pcmHandle, hwParams)) < 0) {
    printf(
        "Broken configuration for playback: no configurations available:%s\n",
        snd_strerror(returnValue));
    return 1;
  }

  // set hardware resampling
  // enable alsa-lib resampling
  std::int32_t resample = 1;
  if ((returnValue = snd_pcm_hw_params_set_rate_resample(pcmHandle, hwParams,
                                                         resample)) < 0) {
    printf("Resampling setup failed for playback: %s\n",
           snd_strerror(returnValue));
    return 1;
  }

  // set the interleaved read/write format
  if ((returnValue =
           snd_pcm_hw_params_set_access(pcmHandle, hwParams, accessType)) < 0) {
    printf("Access type not available for playback: %s\n",
           snd_strerror(returnValue));
    return 1;
  }

  // set the sample format
  // Signed 16-bit little-endian format
  if ((returnValue =
           snd_pcm_hw_params_set_format(pcmHandle, hwParams, pcmFormat)) < 0) {
    printf("Sample format not available for playback: %s\n",
           snd_strerror(returnValue));
    return 1;
  }

  // set the count of channels
  if (setDeviceChannelQuantity(pcmHandle, hwParams, deviceChannelQuantity)) {
    return 1;
  }

  // set the stream sampling rate
  std::uint32_t realSamplingRate = deviceSamplingRate;
  if ((returnValue = snd_pcm_hw_params_set_rate_near(
           pcmHandle, hwParams, &realSamplingRate, 0)) < 0) {
    printf("Rate %uHz not available for playback: %s\n", deviceSamplingRate,
           snd_strerror(returnValue));
    return 1;
  }

  DEBUG_PRINT_ARGS(TEXT("realSamplingRate: %d\n"), realSamplingRate);
  if (realSamplingRate != deviceSamplingRate) {
    printf("Rate doesn't match (requested %uHz, get %iHz)\n",
           deviceSamplingRate, realSamplingRate);
    return 1;
  }

  std::int32_t dir = 0;
  unsigned int time = bufferTime;
  // set the buffer time
  if ((returnValue = snd_pcm_hw_params_set_buffer_time_near(pcmHandle, hwParams,
                                                            &time, &dir)) < 0) {
    printf("Unable to set buffer time %u for playback: %s\n", bufferTime,
           snd_strerror(returnValue));
    return 1;
  }

  DEBUG_PRINT_ARGS("bufferTime: %u\n", time);

  if ((returnValue = snd_pcm_hw_params_get_buffer_size(
           hwParams, &framesPerRingBuffer)) < 0) {
    printf("Unable to get buffer size for playback: %s\n",
           snd_strerror(returnValue));
    return 1;
  }
  DEBUG_PRINT_ARGS("framesPerRingBuffer: %lu\n", framesPerRingBuffer);
  // set the period time

  if ((returnValue = snd_pcm_hw_params_set_period_time_near(pcmHandle, hwParams,
                                                            &time, &dir)) < 0) {
    printf("Unable to set period time %u for playback: %s\n", periodTime,
           snd_strerror(returnValue));
    return 1;
  }

  DEBUG_PRINT_ARGS("periodTime: %u\n", time);

  if ((returnValue = snd_pcm_hw_params_get_period_size(
           hwParams, &framesPerPeriod, &dir)) < 0) {
    printf("Unable to get period size for playback: %s\n",
           snd_strerror(returnValue));
    return 1;
  }

  DEBUG_PRINT_ARGS("framesPerPeriod: %lu\n", framesPerPeriod);

  periodCountPerRingBuffer = (framesPerRingBuffer / framesPerPeriod);
  // write the parameters to device
  // SND_PCM_STATE_OPEN
  if ((returnValue = snd_pcm_hw_params(pcmHandle, hwParams)) < 0) {
    printf("Unable to set hw params for playback: %s\n",
           snd_strerror(returnValue));
    return 1;
  }

  return 0;
}
std::uint16_t AlsaApiInterface::initializeSwParams() {
  snd_pcm_sw_params_alloca(&swParams);

  // get the current swParams
  if ((returnValue = snd_pcm_sw_params_current(pcmHandle, swParams)) < 0) {
    printf("Unable to determine current swParams for playback: %s\n",
           snd_strerror(returnValue));
    return 1;
  }

  // start the transfer when the buffer is almost full:
  if ((returnValue = snd_pcm_sw_params_set_start_threshold(
           pcmHandle, swParams,
           (framesPerRingBuffer / framesPerPeriod) * framesPerPeriod)) < 0) {
    printf("Unable to set start threshold mode for playback: %s\n",
           snd_strerror(returnValue));
    return 1;
  }

  // set minimum avail frames to consider PCM ready
  if ((returnValue = snd_pcm_sw_params_set_avail_min(pcmHandle, swParams,
                                                     framesPerPeriod)) < 0) {

    printf("Unable to set avail min for playback: %s\n",
           snd_strerror(returnValue));
    return 1;
  }
  // enable period events when requested
  if (pollFlag) {
    if ((returnValue =
             snd_pcm_sw_params_set_period_event(pcmHandle, swParams, 1)) < 0) {
      printf("Unable to set period event: %s\n", snd_strerror(returnValue));
      return 1;
    }
    DEBUG_PRINT("period_event was set.\n");
  }
  if ((returnValue = snd_pcm_sw_params(pcmHandle, swParams)) < 0) {
    printf("Unable to set sw params for playback: %s\n",
           snd_strerror(returnValue));
    return 1;
  }
#ifdef _DEBUG
  printPcmState();
#endif
  return 0;
}

std::uint16_t AlsaApiInterface::initializeParams() {
  if (initializeFlag)
    return 0;
  switch (writeLoopMode) {
  case AlsaWriteLoopMode::normal:
  case AlsaWriteLoopMode::callbackEvent:
  case AlsaWriteLoopMode::callbackSignal:
  case AlsaWriteLoopMode::alsaPoll:
    accessType = SND_PCM_ACCESS_RW_INTERLEAVED;
    pollFlag = true;
    break;
  case AlsaWriteLoopMode::mmap:
    accessType = SND_PCM_ACCESS_MMAP_INTERLEAVED;
    mmapPcmRunningFlag = false;
    break;
  default:
    accessType = SND_PCM_ACCESS_RW_INTERLEAVED;
    break;
  }

  if (initializeHwParams())
    return 1;
  if (initializeSwParams())
    return 1;
  DEBUG_PRINT("Alsa parameter was initialized successfully.\n");
  initializeFlag = true;
  return 0;
}

std::uint16_t AlsaApiInterface::initializePollFd(
    std::unique_ptr<struct pollfd> &pollFdArrayPtr, std::int16_t &pollFdCount) {
  pollFdCount = snd_pcm_poll_descriptors_count(pcmHandle);
  if (pollFdCount <= 0) {
    printf("Invalid poll descriptors count\n");
    return 1;
  }
  pollFdArrayPtr.reset(new struct pollfd[pollFdCount]);

  if ((returnValue = snd_pcm_poll_descriptors(pcmHandle, pollFdArrayPtr.get(),
                                              pollFdCount)) < 0) {
    printf("Unable to obtain poll descriptors for playback: %s\n",
           snd_strerror(returnValue));
    return 1;
  }
  DEBUG_PRINT("initializePollFd() normally end.\n");
  return 0;
}

std::int32_t AlsaApiInterface::getCallbackSignalNo() const noexcept {
  return asyncHandlerSignalNumber;
}

static std::uint16_t xrunRecovery(snd_pcm_t *handle, std::int32_t returnValue) {
  /*if (verbose)
    printf("stream recovery\n");*/
  if (returnValue == -EPIPE) { /* under-run */
    returnValue = snd_pcm_prepare(handle);
    if (returnValue < 0) {
      printf("Can't recovery from undreturnValueun, prepare failed: %s\n",
             snd_strerror(returnValue));
      return 1;
    }
    return 0;
  } else if (returnValue == -ESTRPIPE) {
    // wait until the suspend flag is released
    while ((returnValue = snd_pcm_resume(handle)) == -EAGAIN)
      sleep(1);
    if (returnValue < 0) {
      returnValue = snd_pcm_prepare(handle);
      if (returnValue < 0) {
        printf("Can't recovery from suspend, prepare failed: %s\n",
               snd_strerror(returnValue));
        return 1;
      }
    }
    return 0;
  }
  printf("xrunRecovery() can't be applied to this error.: %s\n",
         snd_strerror(returnValue));
  return 1;
}

std::uint16_t AlsaApiInterface::normalWriteBeforeStart(std::int16_t *dataPtr) {
  returnValue = snd_pcm_writei(pcmHandle, dataPtr, framesPerPeriod);
  if (returnValue < 0) {
    printf("normalWrite: Initial write error: %s\n", snd_strerror(returnValue));
    return 1;
  }
  if (returnValue != static_cast<std::int32_t>(framesPerPeriod)) {
    printf("normal write before start: written %i expected %li\n", returnValue,
           framesPerPeriod);
    return 1;
  }
  return 0;
}

std::uint16_t AlsaApiInterface::initializeAsyncHandlerSignalNumber(
    snd_async_callback_t callback, void *privateData) {
  if ((returnValue = snd_async_add_pcm_handler(&asyncHandler, pcmHandle,
                                               callback, privateData)) < 0) {
    printf("snd_async_add_pcm_handler failed.: %s\n",
           snd_strerror(returnValue));
    return 1;
  }
  if ((asyncHandlerSignalNumber = snd_async_handler_get_signo(asyncHandler)) <
      0) {
    printf("snd_async_handler_get_signo failed.: %s\n",
           snd_strerror(asyncHandlerSignalNumber));
    return 1;
  }

  if (deleteAsyncHandler())
    return 1;
  return 0;
}

std::uint16_t AlsaApiInterface::startCallback(snd_async_callback_t callback,
                                              void *privateData) {
  if ((returnValue = snd_async_add_pcm_handler(&asyncHandler, pcmHandle,
                                               callback, privateData)) < 0) {
    printf("snd_async_add_pcm_handler failed.: %s\n",
           snd_strerror(returnValue));
    return 1;
  }
#ifdef _DEBUG
  if (snd_pcm_state(pcmHandle) == SND_PCM_STATE_RUNNING)
    printf("SND_PCM_STATE_RUNNING\n");
  else if (snd_pcm_state(pcmHandle) == SND_PCM_STATE_PREPARED)
    printf("SND_PCM_STATE_PREPARED\n");
  else if (snd_pcm_state(pcmHandle) == SND_PCM_STATE_OPEN)
    printf("SND_PCM_STATE_OPEN\n");
  else if (snd_pcm_state(pcmHandle) == SND_PCM_STATE_SETUP)
    printf("SND_PCM_STATE_SETUP\n");
  else if (snd_pcm_state(pcmHandle) == SND_PCM_STATE_XRUN)
    printf("SND_PCM_STATE_XRUN\n");
  else
    printf("pcm state: %d\n", snd_pcm_state(pcmHandle));
#endif
  if (snd_pcm_state(pcmHandle) == SND_PCM_STATE_PREPARED) {
    if ((returnValue = snd_pcm_start(pcmHandle)) < 0) {
      printf("snd_pcm_start(): %s\n", snd_strerror(returnValue));
      return 1;
    }
  }
  return 0;
}

std::uint16_t AlsaApiInterface::deleteAsyncHandler() {
  if (snd_async_del_handler(asyncHandler)) {
    printf("snd_async_del_handler(): %s\n", snd_strerror(returnValue));
    return 1;
  }
  return 0;
}

std::uint16_t AlsaApiInterface::startPcm() {
  DEBUG_PRINT("startPcm\n");
  printPcmState();

  if (snd_pcm_state(pcmHandle) == SND_PCM_STATE_PREPARED)
    if ((returnValue = snd_pcm_start(pcmHandle)) < 0) {
      if (snd_pcm_state(pcmHandle) == SND_PCM_STATE_RUNNING)
        return 0;
      printf("snd_pcm_start failed: %s\n", snd_strerror(returnValue));
      return 1;
    }
  return 0;
}

std::uint16_t AlsaApiInterface::stopPcm() {
  // If SND_PCM_NONBLOCK is set,snd_pcm_drain return -EAGAIN.
  if ((returnValue = snd_pcm_drain(pcmHandle)) < 0) {
    if (returnValue == -ESTRPIPE) {
      while ((returnValue = snd_pcm_resume(pcmHandle)) == -EAGAIN)
        sleep(1); /* wait until the suspend flag is released */
      if (returnValue < 0) {
        returnValue = snd_pcm_prepare(pcmHandle);
        if (returnValue < 0) {
          printf("Can't recovery from suspend, prepare failed: %s\n",
                 snd_strerror(returnValue));
          return 1;
        }
      }
      initializeFlag = false;
      return 0;
    }
    printf("snd_pcm_drain failed.: %s\n", snd_strerror(returnValue));
    return 1;
  }
  initializeFlag = false;
  return 0;
}

std::uint16_t AlsaApiInterface::possibleWrittenSizeIsBiggerThanPeriod() {
  biggerSizeFlag = false;
  avail = snd_pcm_avail_update(pcmHandle);
  DEBUG_PRINT_ARGS("avail: %d\n", avail);
  if (avail < 0) {
    if (xrunRecovery(pcmHandle, avail)) {
      printf("snd_pcm_avail_update failed");
      return 1;
    }
  } else if (static_cast<std::uint32_t>(avail) >= framesPerPeriod) {
    biggerSizeFlag = true;
  }
  return 0;
}

std::uint16_t AlsaApiInterface::writeInCallbackMode(std::int16_t *dataPtr) {
  if ((returnValue = snd_pcm_writei(pcmHandle, dataPtr, framesPerPeriod)) < 0) {
    if (xrunRecovery(pcmHandle, returnValue)) {
      printf("snd_pcm_writei() failed.\n");
      return 1;
    }
    // skip one period
    return 0;
  }
  if (returnValue != static_cast<std::int32_t>(framesPerPeriod)) {
    printf("callback write error: written %i expected %li\n", returnValue,
           framesPerPeriod);
    // skip one period
  }
  return 0;
}

std::uint16_t AlsaApiInterface::pollErrorTask() {
  if (snd_pcm_state(pcmHandle) == SND_PCM_STATE_XRUN ||
      snd_pcm_state(pcmHandle) == SND_PCM_STATE_SUSPENDED) {
    std::int32_t errorCode =
        snd_pcm_state(pcmHandle) == SND_PCM_STATE_XRUN ? -EPIPE : -ESTRPIPE;
    if (xrunRecovery(pcmHandle, errorCode)) {
      return 1;
    }
    return 0;
  } else {
    printf("pollErrorTask() can't be applied to this error.");
    return 1;
  }
}

std::uint16_t AlsaApiInterface::writeInPollMode(std::int16_t *&currentDataPtr,
                                                std::uint32_t &writtenFrames,
                                                bool &writeEnd,
                                                bool &rewriteFlag) {
  returnValue = snd_pcm_writei(pcmHandle, currentDataPtr, writtenFrames);
  if (returnValue < 0) {
    if (xrunRecovery(pcmHandle, returnValue)) {
      printf("snd_pcm_writei() failed.\n");
      return 1;
    }
    writeEnd = true;
    // skip one period
    rewriteFlag = true;
    return 0;
  }
  currentDataPtr += returnValue * deviceChannelQuantity;
  writtenFrames -= returnValue;
  if (writtenFrames == 0) {
    writeEnd = true;
  }
  return 0;
}

std::uint16_t AlsaApiInterface::writeInSimpleMode(std::int16_t *dataPtr) {
  // printPcmState();
  std::uint32_t remainingWrittenFrameSize = framesPerPeriod;

  while (remainingWrittenFrameSize > 0) {
    returnValue = snd_pcm_writei(pcmHandle, dataPtr, remainingWrittenFrameSize);
    if (returnValue == -EAGAIN)
      continue;
    if (returnValue < 0) {
      if (xrunRecovery(pcmHandle, returnValue)) {
        printf("snd_pcm_writei failed.\n");
        return 1;
      }
      // skip one period
      break;
    }
    dataPtr += returnValue * deviceChannelQuantity;
    remainingWrittenFrameSize -= returnValue;
  }
  return 0;
}

std::uint16_t
AlsaApiInterface::checkWritePossibleInMmapMode(bool &pcmRunningFlag) {
  DEBUG_PRINT("checkWritePossibleInMmapMode\n");
#ifdef _DEBUG
  printPcmState();
#endif
  snd_pcm_state_t state = snd_pcm_state(pcmHandle);

  if (state == SND_PCM_STATE_XRUN) {
    if (xrunRecovery(pcmHandle, -EPIPE)) {
      return 1;
    }
    pcmRunningFlag = false;
  } else if (state == SND_PCM_STATE_SUSPENDED) {
    if (xrunRecovery(pcmHandle, -ESTRPIPE)) {
      return 1;
    }
  }

  while (1) {
    returnValue = snd_pcm_avail_update(pcmHandle);
    DEBUG_PRINT_ARGS("checkWritePossibleInMmapMode: avail %d\n", returnValue);

    if (returnValue < 0) {
      if (xrunRecovery(pcmHandle, returnValue)) {
        printf("snd_pcm_avail_update() failed.\n");
        return 1;
      }
      pcmRunningFlag = false;
      continue;
    } else if (returnValue < static_cast<std::int32_t>(framesPerPeriod)) {
      if (!pcmRunningFlag) {
        DEBUG_PRINT_ARGS("snd_pcm_start: avail %d\n", returnValue);

        if ((returnValue = snd_pcm_start(pcmHandle)) < 0) {
          printf("snd_pcm_start() failed: %s\n", snd_strerror(returnValue));
          return 1;
        }
        pcmRunningFlag = true;
      } else {
        DEBUG_PRINT_ARGS("snd_pcm_wait: avail %d\n", returnValue);

        if ((returnValue = snd_pcm_wait(pcmHandle, -1)) < 0) {
          if (xrunRecovery(pcmHandle, returnValue)) {
            printf("snd_pcm_wait() failed,\n");
            return 1;
          }
          pcmRunningFlag = false;
        }
      }
      continue;
    } else {
      DEBUG_PRINT("snd_pcm_avail_update() successed\n");
      break;
    }
  }
  return 0;
}

std::uint16_t AlsaApiInterface::writeLoopInMmap(std::int16_t *audioBufferPtr,
                                                bool &pcmRunningFlag) {
  DEBUG_PRINT("writeLoopInMmap start\n");
  std::uint32_t remainingWrittenFrameSize = framesPerPeriod;
  snd_pcm_uframes_t commitedFrameSize = 0;
  snd_pcm_uframes_t channelAreaFrameOffset = 0;
  const snd_pcm_channel_area_t *pcmChannelAreaPtr = nullptr;
  std::int16_t *mmapPtrTemp = nullptr;
  std::int16_t *audioBufferPtrTemp = nullptr;
  while (remainingWrittenFrameSize > 0) {
    commitedFrameSize = remainingWrittenFrameSize;
    returnValue =
        snd_pcm_mmap_begin(pcmHandle, &pcmChannelAreaPtr,
                           &channelAreaFrameOffset, &commitedFrameSize);
    if (returnValue < 0) {
      if (xrunRecovery(pcmHandle, returnValue)) {
        printf("snd_pcm_mmap_begin() failed.\n");
        return 1;
      }
      pcmRunningFlag = false;
    }
    mmapPtrTemp = reinterpret_cast<std::int16_t *>(pcmChannelAreaPtr->addr) +
                  channelAreaFrameOffset * deviceChannelQuantity;
    audioBufferPtrTemp =
        audioBufferPtr +
        deviceChannelQuantity * (framesPerPeriod - remainingWrittenFrameSize);

    for (uint32_t u = 0; u < commitedFrameSize; ++u)
      for (std::uint32_t v = 0; v < deviceChannelQuantity; ++v) {
        *mmapPtrTemp++ = *audioBufferPtrTemp++;
      }

    returnValue = snd_pcm_mmap_commit(pcmHandle, channelAreaFrameOffset,
                                      commitedFrameSize);
    if (returnValue < 0 ||
        static_cast<snd_pcm_uframes_t>(returnValue) != commitedFrameSize) {
      if (xrunRecovery(pcmHandle, returnValue >= 0 ? -EPIPE : returnValue)) {
        printf("snd_pcm_mmap_commit() failed.\n");
        return 1;
      }
      pcmRunningFlag = false;
    }
    remainingWrittenFrameSize -= commitedFrameSize;
  }
  return 0;
}

std::uint16_t AlsaApiInterface::writeInMmapMode(std::int16_t *audioBufferPtr) {
  DEBUG_PRINT("writeInMmapMode start\n");
  if (checkWritePossibleInMmapMode(mmapPcmRunningFlag))
    return 1;
  if (writeLoopInMmap(audioBufferPtr, mmapPcmRunningFlag))
    return 1;
  return 0;
}

bool AlsaApiInterface::getBiggerSizeThanPeriodFlag() const {
  return biggerSizeFlag;
}

std::uint32_t AlsaApiInterface::getPeriodCountPerRingBuffer() const {
  return periodCountPerRingBuffer;
}

AlsaWriteLoopMode AlsaApiInterface::getAlsaWriteLoopMode() const {
  return writeLoopMode;
}
snd_pcm_uframes_t AlsaApiInterface::getFramesPerPeriod() {
  return framesPerPeriod;
}
std::uint8_t AlsaApiInterface::getDeviceChannelQuantity() const noexcept {
  return deviceChannelQuantity;
}
std::uint16_t AlsaApiInterface::getDeviceSamplingRate() const noexcept {
  return deviceSamplingRate;
}

AlsaApiInterface::~AlsaApiInterface() { snd_pcm_close(pcmHandle); }
