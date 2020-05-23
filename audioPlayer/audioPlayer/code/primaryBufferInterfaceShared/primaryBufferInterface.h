#pragma once

#include "primaryBufferInterfaceDLLheader.h"
#include <unordered_map>
//#include <vector>
#include "audioMessageQueue.h"
#include "secondaryBufferBase.h"
#include <cstring>
#include "characterCodeMacro.h"
#include "noCopyAndMoveTemplate.h"

class primaryBufferInterface_API PrimaryBufferInterface
    : public NoCopyAndMove<PrimaryBufferInterface> {
private:
  bool playOn = false;
  std::uint32_t newFileId = 0;
  AudioMessageQueue messageQueue;
  std::unordered_map<std::uint32_t,
                     std::unique_ptr<SecondaryBuffer::CommonBase>>
      secondaryBufferArray;
  std::uint8_t deviceChannelQuantity = 0;
  std::uint32_t deviceSamplingRate = 0;
  std::uint32_t framesPerPeriod = 0;
  std::uint16_t addMessageTask(
      std::uint32_t fileId,
      std::unique_ptr<SecondaryBuffer::CommonBase> &&secondaryBufferPtr);
  std::uint16_t startMessageTask(std::uint32_t fileId);
  std::uint16_t stopMessageTask(uint32_t fileId);
  std::uint16_t removeMessageTask(std::uint32_t fileId);
  std::uint16_t allRemoveMessageTask();
  std::uint16_t setSampleData(std::int16_t *out,
                              std::uint8_t deviceChannelIndex);

public:
  PrimaryBufferInterface();
  std::uint16_t initialize(std::uint8_t deviceChannelQuantity,
                           std::uint32_t deviceSamplingRate,
                           std::uint32_t framesPerPeriod);
  std::uint16_t add(std::uint32_t &fileId, std::TSTRING &&filePath, bool loop,
                    DataReadingMethod method);

  std::uint16_t start(std::uint32_t fileId);
  std::uint16_t stop(std::uint32_t fileId);
  std::uint16_t remove(std::uint32_t fileId);
  std::uint16_t allRemove();
  std::uint16_t currentAllMessageTask();

  void setPrimaryBufferPlayStatus();
  std::uint16_t readFileDataIfSuccessively();
  std::uint16_t makeAllPrimaryBufferData(std::int16_t *audioBufferPtr);
  bool getPlayOn() const noexcept;
  void setPlayOn(bool flag) noexcept;
};
