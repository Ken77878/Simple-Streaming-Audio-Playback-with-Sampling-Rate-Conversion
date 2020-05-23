#include "audioMessageQueue.h"
#include "audioMessageAdd.h"
#include <utility>

std::uint32_t AudioMessageQueue::size() {
  std::lock_guard<std::mutex> lock(queueMutex);
  return messageQueue.size();
}

std::unique_ptr<AudioMessage::Base> &AudioMessageQueue::front() {
  std::lock_guard<std::mutex> lock(queueMutex);
  return messageQueue.front();
}
void AudioMessageQueue::pop() {
  std::lock_guard<std::mutex> lock(queueMutex);
  messageQueue.pop();
}

void AudioMessageQueue::emplace(AudioMessageId messageId) {
  std::lock_guard<std::mutex> lock(queueMutex);
  messageQueue.emplace(new AudioMessage::Base(messageId));
}

void AudioMessageQueue::emplace(AudioMessageId messageId,
                                std::uint32_t fileId) {
  std::lock_guard<std::mutex> lock(queueMutex);
  messageQueue.emplace(new AudioMessage::OneFile(messageId, fileId));
}

void AudioMessageQueue::emplace(
    AudioMessageId messageId, std::uint32_t fileId,
    std::unique_ptr<SecondaryBuffer::CommonBase> &&secondaryBufferPtr) {
  std::lock_guard<std::mutex> lock(queueMutex);
  messageQueue.emplace(
      new AudioMessage::Add(messageId, fileId, std::move(secondaryBufferPtr)));
}
