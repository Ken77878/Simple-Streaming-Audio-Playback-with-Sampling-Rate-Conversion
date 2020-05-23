#pragma once
#include <array>
#include <vector>
#include "primaryBufferInterface.h"
#include "criticalSectionManagement.h"
#include "eventManagementInterface.h"

#include "noCopyAndMoveTemplate.h"
#include "audioBufferPoolInterfaceCommonHeader.h"

constexpr std::uint16_t audioBufferPoolSize = 50;

typedef std::uint16_t (*audioBufferAccessFunc)(void *audioInterfaceObjectPtr,
                                               std::int16_t *audioBufferPtr);
enum class AudioBufferPoolThreadState { on, writeEnd, off };

enum class AudioBufferState { writePossible, readPossible };

struct audioBufferPoolInterface_API AudioBufferStructure {
  AudioBufferState state = AudioBufferState::writePossible;
  std::vector<std::int16_t> buffer;
  MyUtility::EventManagementInterface writePossibleEvent;
  MyUtility::EventManagementInterface readPossibleEvent;
};

class audioBufferPoolInterface_API AudioBufferPoolInterface
    : public NoCopyAndMove<AudioBufferPoolInterface> {
private:
  AudioBufferPoolThreadState poolState = AudioBufferPoolThreadState::off;
  bool playContinueFlag = false;
  std::array<AudioBufferStructure, audioBufferPoolSize> audioBufferPool;
  PrimaryBufferInterface primaryBuffer;
  std::uint16_t currentWriteIndex = 0;
  std::uint16_t currentReadIndex = 0;
  MyUtility::CriticalSectionManagementInterface poolStateCriticalSection;
  MyUtility::CriticalSectionManagementInterface startMessageCriticalSection;
  MyUtility::CriticalSectionManagementInterface bufferStateCriticalSection;
  std::uint16_t primaryBufferTask(bool &writeContinueFlag);
  
public:
  std::uint16_t initialize(std::uint8_t deviceChannelQuantity,
                           std::uint32_t deviceSamplingRate,
                           std::uint32_t framesPerPeriod);

  PrimaryBufferInterface &getPrimaryBufferRef() noexcept;

  std::uint16_t fillAudioBufferPool(bool &writeContinueFlag);
  std::uint16_t writeTask(bool &writeContinueFlag);
  std::uint16_t readTask(audioBufferAccessFunc readFunc,
                         void *audioInterfaceObjectPtr, bool &poolEndFlag);
  std::uint16_t start(std::uint32_t fileId, bool &poolThreadStartFlag);
  std::uint16_t poolThreadTask();
  bool getPlayContinueFlag() const noexcept;
  void setPlayContinueFlag(bool flag) noexcept;

  std::uint16_t finalize();
};
