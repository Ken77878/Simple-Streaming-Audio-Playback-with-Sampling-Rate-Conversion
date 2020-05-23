#include "audioBufferPoolInterface.h"
#include "debugMacro.h"


std::uint16_t
AudioBufferPoolInterface::initialize(std::uint8_t deviceChannelQuantity,
                                     std::uint32_t deviceSamplingRate,
                                     std::uint32_t framesPerPeriod) {
  if (primaryBuffer.initialize(deviceChannelQuantity, deviceSamplingRate,
                               framesPerPeriod))
    return 1;

  for (std::uint32_t u = 0; u < audioBufferPoolSize; ++u) {
    audioBufferPool[u].buffer.resize(deviceChannelQuantity * framesPerPeriod);

    if (audioBufferPool[u].writePossibleEvent.initialize(nullptr, false, true, nullptr))
      return 1;
    if (audioBufferPool[u].readPossibleEvent.initialize(nullptr, false, false, nullptr))
      return 1;
  }
  return 0;
}

PrimaryBufferInterface &AudioBufferPoolInterface::getPrimaryBufferRef() noexcept{
  return primaryBuffer;
}

std::uint16_t
AudioBufferPoolInterface::primaryBufferTask(bool &writeContinueFlag) {
  // 1. read queue and update Secondary buffer
  if (primaryBuffer.currentAllMessageTask())
    return 1;

  primaryBuffer.setPrimaryBufferPlayStatus();

  if (!primaryBuffer.getPlayOn()) {
    writeContinueFlag = false;
    return 0;
  }

  // 2.make secondary buffers data and sum them
  if (primaryBuffer.makeAllPrimaryBufferData(
          &audioBufferPool[currentWriteIndex].buffer[0]))
    return 1;

  return 0;
}

std::uint16_t
AudioBufferPoolInterface::fillAudioBufferPool(bool &writeContinueFlag) {
  DEBUG_PRINT(TEXT("fillAudioBufferPool start\n"));
  currentWriteIndex = 0;
  currentReadIndex = 0;
  writeContinueFlag = true;
  for (std::uint32_t u = 0; u < audioBufferPoolSize; ++u) {
    DEBUG_PRINT(TEXT("fillAudioBufferPool one buffer\n"));

    if (primaryBufferTask(writeContinueFlag))
      return 1;
    if (!writeContinueFlag) {
		poolStateCriticalSection.enter();
        return 1;
      poolState = AudioBufferPoolThreadState::writeEnd;
	  poolStateCriticalSection.leave();
      break;
    }
    audioBufferPool[currentWriteIndex].state = AudioBufferState::readPossible;
    if (audioBufferPool[currentWriteIndex].readPossibleEvent.setEventOn())
      return 1;
    if (audioBufferPool[currentWriteIndex].writePossibleEvent.setEventOff())
      return 1;

    ++currentWriteIndex;
  }
  currentWriteIndex = 0;
  DEBUG_PRINT(TEXT("fillAudioBufferPool end\n"));
  return 0;
}

std::uint16_t AudioBufferPoolInterface::writeTask(bool &writeContinueFlag) {
  DEBUG_PRINT(TEXT("audioBufferPool writeTask Start\n"));

  bufferStateCriticalSection.enter();

  if (audioBufferPool[currentWriteIndex].state ==
      AudioBufferState::readPossible) {
    DEBUG_PRINT(TEXT("audioBufferPool writeTask Wait\n"));

	bufferStateCriticalSection.leave();
    if (audioBufferPool[currentWriteIndex].writePossibleEvent.waitEvent())
      return 1;
  } else {
	  bufferStateCriticalSection.leave();
  }

  if (primaryBufferTask(writeContinueFlag))
    return 1;

  if (!writeContinueFlag) {
    poolStateCriticalSection.enter();
    poolState = AudioBufferPoolThreadState::writeEnd;
    poolStateCriticalSection.leave();
    DEBUG_PRINT(TEXT("========audioBufferPool write end finally.\n"));
    return 0;
  }
  bufferStateCriticalSection.enter();
  audioBufferPool[currentWriteIndex].state = AudioBufferState::readPossible;

  if (audioBufferPool[currentWriteIndex].readPossibleEvent.setEventOn())
    return 1;
  if (audioBufferPool[currentWriteIndex].writePossibleEvent.setEventOff())
    return 1;

  bufferStateCriticalSection.leave();
  DEBUG_PRINT_ARGS(TEXT("=================readPossible: %u\n"), currentWriteIndex);
  ++currentWriteIndex;
  if (currentWriteIndex == audioBufferPoolSize)
    currentWriteIndex = 0;
  // DEBUG_PRINT(TEXT("audioBufferPool writeTask end\n"));

  return 0;
}

std::uint16_t
AudioBufferPoolInterface::readTask(audioBufferAccessFunc readAudioBufferFunc,
                                   void *audioInterfaceObjectPtr,
                                   bool &playEndFlag) {
  DEBUG_PRINT(TEXT("audioBufferPool readTaskStart\n"));

  bufferStateCriticalSection.enter();

  if (audioBufferPool[currentReadIndex].state ==
      AudioBufferState::writePossible) {
    DEBUG_PRINT(TEXT("audioBufferPool readTask  writePossible\n"));

    bufferStateCriticalSection.leave();

    poolStateCriticalSection.enter();
    if (poolState == AudioBufferPoolThreadState::writeEnd) {
      poolStateCriticalSection.leave();
      playEndFlag = true;
	 // Sleep(10000);
     startMessageCriticalSection.enter();

      if (primaryBuffer.currentAllMessageTask())
        return 1;
      primaryBuffer.setPrimaryBufferPlayStatus();

      if (primaryBuffer.getPlayOn()) {
        DEBUG_PRINT(
            TEXT("audioBufferPool readTask primary buffer on : to next loop\n"));
        startMessageCriticalSection.leave();
        playContinueFlag = true;
        return 0;
      }
      DEBUG_PRINT(TEXT("audioBufferPool readTask primary buffer off\n"));
      poolStateCriticalSection.enter();
      poolState = AudioBufferPoolThreadState::off;
      poolStateCriticalSection.leave();
      startMessageCriticalSection.leave();
      playContinueFlag = false;
      return 0;
    } else {
      DEBUG_PRINT(TEXT("audioBufferPool readRask wait\n"));
      if (audioBufferPool[currentReadIndex].readPossibleEvent.waitEvent())
        return 1;
    }

    poolStateCriticalSection.leave();
  }
  bufferStateCriticalSection.leave();
  if ((*readAudioBufferFunc)(audioInterfaceObjectPtr,
                             &audioBufferPool[currentReadIndex].buffer[0]))
    return 1;

  bufferStateCriticalSection.enter();
  audioBufferPool[currentReadIndex].state = AudioBufferState::writePossible;
  if (audioBufferPool[currentReadIndex].writePossibleEvent.setEventOn())
    return 1;
  if (audioBufferPool[currentReadIndex].readPossibleEvent.setEventOff())
    return 1;

  bufferStateCriticalSection.leave();

  DEBUG_PRINT_ARGS(TEXT("==================writePossible: %u\n"), currentReadIndex);
  ++currentReadIndex;
  if (currentReadIndex == audioBufferPoolSize)
    currentReadIndex = 0;

  return 0;
}

std::uint16_t AudioBufferPoolInterface::start(std::uint32_t fileId,
                                              bool &audioPlayStartFlag) {
  DEBUG_PRINT(TEXT("AudioInterface::start()\n"));
  audioPlayStartFlag = false;
  startMessageCriticalSection.enter();
  if (primaryBuffer.start(fileId))
    return 1;
  poolStateCriticalSection.enter();
  if (poolState == AudioBufferPoolThreadState::off) {
    DEBUG_PRINT(TEXT("primary buffer is stopped.\n"));
    poolState = AudioBufferPoolThreadState::on;
    poolStateCriticalSection.leave();
    audioPlayStartFlag = true;
    // to play start
  } else {
    poolStateCriticalSection.leave();
  }
  startMessageCriticalSection.leave();

  return 0;
}

std::uint16_t AudioBufferPoolInterface::poolThreadTask() {
  primaryBuffer.setPlayOn(true);
  bool writeContinueFlag = true;
  while (writeContinueFlag) {
    if (writeTask(writeContinueFlag))
      return 1;
  }
  return 0;
}
bool AudioBufferPoolInterface::getPlayContinueFlag() const noexcept {
  return playContinueFlag;
}
void AudioBufferPoolInterface::setPlayContinueFlag(bool flag) noexcept {
  playContinueFlag = flag;
}

std::uint16_t AudioBufferPoolInterface::finalize() {
  for (std::uint32_t u = 0; u < audioBufferPoolSize; ++u) {
    if (audioBufferPool[u].writePossibleEvent.finalize())
      return 1;
    if (audioBufferPool[u].readPossibleEvent.finalize())
      return 1;
  }
 
  return 0;
}
