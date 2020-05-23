#pragma once
#include <cstdint>
#include <stddef.h>
// include asoundlib.h after stddef.h
// because "unknown type name 'size_t'" error occurs.
#include "alsa/asoundlib.h"
//
#include <poll.h>
#include <memory>
#include <vector>
#include "noCopyAndMoveTemplate.h"
#include "alsaWriteLoopModeEnumerance.h"

typedef std::uint16_t (*writeToBufferFunc)(void *primaryBufferObjectPtr,
                                           std::uint8_t *bufferPtr,
                                           std::uint32_t writtenFrameSize);
class AlsaApiInterface : public NoCopyAndMove<AlsaApiInterface> {
private:
  bool initializeFlag = false;
  AlsaWriteLoopMode writeLoopMode = AlsaWriteLoopMode::callbackSignal;
  std::int32_t returnValue = 0;
  snd_pcm_t *pcmHandle = nullptr;
  snd_pcm_access_t accessType;
  snd_async_handler_t *asyncHandler;
  snd_pcm_hw_params_t *hwParams;
  snd_pcm_sw_params_t *swParams;
  std::uint8_t deviceChannelQuantity = 2;
  std::uint16_t deviceSamplingRate = 44100;
  std::int32_t asyncHandlerSignalNumber = 0;

  // ring buffer size
  snd_pcm_uframes_t framesPerRingBuffer = 0;
  snd_pcm_uframes_t framesPerPeriod = 0;
  std::uint32_t periodCountPerRingBuffer;
  snd_pcm_sframes_t avail = 0;
  bool biggerSizeFlag = false;
  bool pollFlag = false;
  bool mmapPcmRunningFlag = false;
  std::uint16_t initializeHwParams();
  std::uint16_t initializeSwParams();
  std::uint16_t checkWritePossibleInMmapMode(bool &pcmRunningFlag);
  std::uint16_t writeLoopInMmap(std::int16_t *audioBufferPtr,
                                bool &pcmRunningFlag);

public:
  AlsaApiInterface();
  void printPcmState();
  std::uint16_t openPcmHandle();
  std::uint16_t initializeParams();
  std::uint16_t initializePollFd(std::unique_ptr<struct pollfd> &pollFdArrayPtr,
                                 std::int16_t &pollFdCount);
  std::uint16_t
  initializeAsyncHandlerSignalNumber(snd_async_callback_t callback,
                                     void *privateData);
  std::int32_t getCallbackSignalNo() const noexcept;
  std::uint16_t normalWriteBeforeStart(std::int16_t *dataPtr);
  std::uint16_t startCallback(snd_async_callback_t callback, void *privateData);
  std::uint16_t deleteAsyncHandler();
  std::uint16_t startPcm();
  std::uint16_t stopPcm();
  std::uint16_t possibleWrittenSizeIsBiggerThanPeriod();

  std::uint16_t writeInCallbackMode(std::int16_t *dataPtr);
  std::uint16_t pollErrorTask();
  std::uint16_t writeInPollMode(std::int16_t *&currentDataPtr,
                                std::uint32_t &writtenFrames, bool &writeEnd,
                                bool &rewriteFlag);

  std::uint16_t writeInSimpleMode(std::int16_t *dataPtr);
  std::uint16_t writeInMmapMode(std::int16_t *audioBufferPtr);

  std::uint8_t getDeviceChannelQuantity() const noexcept;
  std::uint16_t getDeviceSamplingRate() const noexcept;

  snd_pcm_uframes_t getFramesPerPeriod();

  bool getBiggerSizeThanPeriodFlag() const;

  std::uint32_t getPeriodCountPerRingBuffer() const;
  AlsaWriteLoopMode getAlsaWriteLoopMode() const;

  ~AlsaApiInterface();
};
