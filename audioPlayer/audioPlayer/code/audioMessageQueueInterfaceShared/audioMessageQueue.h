#pragma once

#include "audioMessageBase.h"
#include <queue>
#include <mutex>
#include <memory>
#include "secondaryBufferBase.h"
#include "audioMessageQueueInterfaceDLLheader.h"
#include "noCopyAndMoveTemplate.h"

class audioMessageQueueInterface_API AudioMessageQueue
    : public NoCopyAndMove<AudioMessageQueue> {
private:
  std::queue<std::unique_ptr<AudioMessage::Base>> messageQueue;
  std::mutex queueMutex;

public:
  std::uint32_t size();
  std::unique_ptr<AudioMessage::Base> &front();
  void pop();
  void emplace(AudioMessageId messageId);
  void emplace(AudioMessageId messageId, std::uint32_t fileId);
  void
  emplace(AudioMessageId messageId, std::uint32_t fileId,
          std::unique_ptr<SecondaryBuffer::CommonBase> &&secondaryBufferPtr);
};
