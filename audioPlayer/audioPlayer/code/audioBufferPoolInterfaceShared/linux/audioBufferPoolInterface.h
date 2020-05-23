#pragma once
#include <array>
#include <vector>
#include "primaryBufferInterface.h"
#include "eventWaitingInterface.h"
#include "pthreadMutexManagement.h"
#include "noCopyAndMoveTemplate.h"

constexpr std::uint16_t audioBufferPoolSize = 30;

typedef std::uint16_t (*audioBufferAccessFunc)(void *audioInterfaceObjectPtr,
                                               std::int16_t *audioBufferPtr);
enum class AudioBufferPoolThreadState { on, writeEnd, off };

enum class AudioBufferState { writePossible, readPossible };

struct AudioBufferStructure {
  AudioBufferState state = AudioBufferState::writePossible;
  std::vector<std::int16_t> buffer;
  std::int32_t writePossibleEvent = 0;
  std::int32_t readPossibleEvent = 0;
};

class AudioBufferPoolInterface
    : public NoCopyAndMove<AudioBufferPoolInterface> {
private:
  AudioBufferPoolThreadState poolState = AudioBufferPoolThreadState::off;
  bool playContinueFlag = false;
  std::array<AudioBufferStructure, audioBufferPoolSize> audioBufferPool;
  PrimaryBufferInterface primaryBuffer;
  std::uint16_t currentWriteIndex = 0;
  std::uint16_t currentReadIndex = 0;
  MyUtility::PthreadMutexManagement poolStateMutex;
  MyUtility::PthreadMutexManagement startMessageMutex;
  MyUtility::PthreadMutexManagement bufferStateMutex;
  MyUtility::EventWaitingInterface eventWaitingInterface;
  std::uint16_t primaryBufferTask(bool &writeContinueFlag);
  /*  std::uint16_t checkEpollEventReady(std::uint32_t bufferIndex,
                                       std::uint32_t eventFlag);
  */
public:
  std::uint16_t initialize(std::uint8_t deviceChannelQuantity,
                           std::uint32_t deviceSamplingRate,
                           std::uint32_t framesPerPeriod);

  PrimaryBufferInterface &getPrimaryBufferRef();

  std::uint16_t fillAudioBufferPool(bool &writeContinueFlag);
  std::uint16_t writeTask(bool &writeContinueFlag);
  std::uint16_t readTask(audioBufferAccessFunc readFunc,
                         void *audioInterfaceObjectPtr, bool &poolEndFlag);
  std::uint16_t start(std::uint32_t fileId, bool &poolThreadStartFlag);
  std::uint16_t poolThreadTask();
  // std::int16_t *getPlayedAudioBufferPtr() const;
  bool getPlayContinueFlag() const noexcept;
  void setPlayContinueFlag(bool flag) noexcept;

  std::uint16_t finalize();
};
