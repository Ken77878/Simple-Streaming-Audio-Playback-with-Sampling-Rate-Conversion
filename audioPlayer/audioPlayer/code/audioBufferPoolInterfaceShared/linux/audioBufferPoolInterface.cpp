#include "audioBufferPoolInterface.h"
#include "debugMacro.h"

#include "unistd.h"

std::uint16_t
AudioBufferPoolInterface::initialize(std::uint8_t deviceChannelQuantity,
                                     std::uint32_t deviceSamplingRate,
                                     std::uint32_t framesPerPeriod) {
  if (primaryBuffer.initialize(deviceChannelQuantity, deviceSamplingRate,
                               framesPerPeriod))
    return 1;
  if (eventWaitingInterface.initialize())
    return 1;
  for (std::uint32_t u = 0; u < audioBufferPoolSize; ++u) {
    audioBufferPool[u].buffer.resize(deviceChannelQuantity * framesPerPeriod);

    if (eventWaitingInterface.registerEvent(
            audioBufferPool[u].writePossibleEvent, false, true))
      return 1;
    if (eventWaitingInterface.registerEvent(
            audioBufferPool[u].readPossibleEvent, false, false))
      return 1;
  }
  return 0;
}

PrimaryBufferInterface &AudioBufferPoolInterface::getPrimaryBufferRef() {
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
  DEBUG_PRINT("fillAudioBufferPool start\n");
  currentWriteIndex = 0;
  currentReadIndex = 0;
  writeContinueFlag = true;
  for (std::uint32_t u = 0; u < audioBufferPoolSize; ++u) {
    DEBUG_PRINT("fillAudioBufferPool one buffer\n");

    if (primaryBufferTask(writeContinueFlag))
      return 1;
    if (!writeContinueFlag) {
      if (poolStateMutex.lock())
        return 1;
      poolState = AudioBufferPoolThreadState::writeEnd;
      if (poolStateMutex.unlock())
        return 1;
      break;
    }
    audioBufferPool[currentWriteIndex].state = AudioBufferState::readPossible;
    if (eventWaitingInterface.setEventOn(
            audioBufferPool[currentWriteIndex].readPossibleEvent))
      return 1;
    if (eventWaitingInterface.setEventOff(
            audioBufferPool[currentWriteIndex].writePossibleEvent))
      return 1;

    ++currentWriteIndex;
  }
  currentWriteIndex = 0;
  DEBUG_PRINT("fillAudioBufferPool end\n");
  return 0;
}

std::uint16_t AudioBufferPoolInterface::writeTask(bool &writeContinueFlag) {
  DEBUG_PRINT("audioBufferPool writeTask Start\n");

  if (bufferStateMutex.lock())
    return 1;

  if (audioBufferPool[currentWriteIndex].state ==
      AudioBufferState::readPossible) {
    DEBUG_PRINT("audioBufferPool writeTask Wait\n");

    if (bufferStateMutex.unlock())
      return 1;
    if (eventWaitingInterface.waitEvent(
            audioBufferPool[currentWriteIndex].writePossibleEvent))
      return 1;
  } else {
    if (bufferStateMutex.unlock())
      return 1;
  }

  if (primaryBufferTask(writeContinueFlag))
    return 1;

  if (!writeContinueFlag) {
    if (poolStateMutex.lock())
      return 1;
    poolState = AudioBufferPoolThreadState::writeEnd;
    if (poolStateMutex.unlock())
      return 1;
    DEBUG_PRINT("========audioBufferPool write end finally.\n");
    return 0;
  }
  if (bufferStateMutex.lock())
    return 1;
  audioBufferPool[currentWriteIndex].state = AudioBufferState::readPossible;

  if (eventWaitingInterface.setEventOn(
          audioBufferPool[currentWriteIndex].readPossibleEvent))
    return 1;
  if (eventWaitingInterface.setEventOff(
          audioBufferPool[currentWriteIndex].writePossibleEvent))
    return 1;

  if (bufferStateMutex.unlock())
    return 1;
  DEBUG_PRINT_ARGS("=================readPossible: %u\n", currentWriteIndex);
  ++currentWriteIndex;
  if (currentWriteIndex == audioBufferPoolSize)
    currentWriteIndex = 0;
  // DEBUG_PRINT("audioBufferPool writeTask end\n");

  return 0;
}

std::uint16_t
AudioBufferPoolInterface::readTask(audioBufferAccessFunc readAudioBufferFunc,
                                   void *audioInterfaceObjectPtr,
                                   bool &playEndFlag) {
  DEBUG_PRINT("audioBufferPool readTaskStart\n");

  if (bufferStateMutex.lock())
    return 1;

  if (audioBufferPool[currentReadIndex].state ==
      AudioBufferState::writePossible) {
    DEBUG_PRINT("audioBufferPool readTask  writePossible\n");

    if (bufferStateMutex.unlock())
      return 1;

    if (poolStateMutex.lock())
      return 1;
    if (poolState == AudioBufferPoolThreadState::writeEnd) {
      if (poolStateMutex.unlock())
        return 1;
      playEndFlag = true;

      if (startMessageMutex.lock())
        return 1;

      if (primaryBuffer.currentAllMessageTask())
        return 1;
      primaryBuffer.setPrimaryBufferPlayStatus();

      if (primaryBuffer.getPlayOn()) {
        DEBUG_PRINT(
            "audioBufferPool readTask primary buffer on : to next loop\n");
        if (startMessageMutex.unlock())
          return 1;
        playContinueFlag = true;
        return 0;
      }
      DEBUG_PRINT("audioBufferPool readTask primary buffer off\n");
      if (poolStateMutex.lock())
        return 1;
      poolState = AudioBufferPoolThreadState::off;
      if (poolStateMutex.unlock())
        return 1;
      if (startMessageMutex.unlock())
        return 1;
      playContinueFlag = false;
      return 0;
    } else {
      DEBUG_PRINT("audioBufferPool readRask wait\n");
      if (eventWaitingInterface.waitEvent(
              audioBufferPool[currentReadIndex].readPossibleEvent))
        return 1;
    }

    if (poolStateMutex.unlock())
      return 1;
  }
  if (bufferStateMutex.unlock())
    return 1;
  if ((*readAudioBufferFunc)(audioInterfaceObjectPtr,
                             &audioBufferPool[currentReadIndex].buffer[0]))
    return 1;

  if (bufferStateMutex.lock())
    return 1;
  audioBufferPool[currentReadIndex].state = AudioBufferState::writePossible;
  if (eventWaitingInterface.setEventOn(
          audioBufferPool[currentReadIndex].writePossibleEvent))
    return 1;
  if (eventWaitingInterface.setEventOff(
          audioBufferPool[currentReadIndex].readPossibleEvent))
    return 1;

  if (bufferStateMutex.unlock())
    return 1;

  DEBUG_PRINT_ARGS("==================writePossible: %u\n", currentReadIndex);
  ++currentReadIndex;
  if (currentReadIndex == audioBufferPoolSize)
    currentReadIndex = 0;

  return 0;
}

std::uint16_t AudioBufferPoolInterface::start(std::uint32_t fileId,
                                              bool &audioPlayStartFlag) {
  DEBUG_PRINT("AudioInterface::start()\n");
  audioPlayStartFlag = false;
  if (startMessageMutex.lock())
    return 1;
  if (primaryBuffer.start(fileId))
    return 1;
  if (poolStateMutex.lock())
    return 1;
  if (poolState == AudioBufferPoolThreadState::off) {
    DEBUG_PRINT("primary buffer is stopped.\n");
    poolState = AudioBufferPoolThreadState::on;
    if (poolStateMutex.unlock())
      return 1;
    audioPlayStartFlag = true;
    // to play start
  } else {
    if (poolStateMutex.unlock())
      return 1;
  }
  if (startMessageMutex.unlock())
    return 1;

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
    if (eventWaitingInterface.eraseEvent(audioBufferPool[u].writePossibleEvent))
      return 1;
    if (eventWaitingInterface.eraseEvent(audioBufferPool[u].readPossibleEvent))
      return 1;
  }
  if (eventWaitingInterface.finalize())
    return 1;

  if (poolStateMutex.finalize())
    return 1;

  if (startMessageMutex.finalize())
    return 1;
  if (bufferStateMutex.finalize())
    return 1;
  return 0;
}
