#include "audioMessageAdd.h"
#include <utility>

namespace AudioMessage {
Add::Add(
    AudioMessageId messageId, std::uint32_t fileId,
    std::unique_ptr<SecondaryBuffer::CommonBase> &&secondaryBufferUniquePtr)
    : OneFile(messageId, fileId),
      secondaryBufferUniquePtr(std::move(secondaryBufferUniquePtr)) {}

std::unique_ptr<SecondaryBuffer::CommonBase> &&
Add::getSecondaryBufferUniquePtr() {
  return std::move(secondaryBufferUniquePtr);
}
} // namespace AudioMessage
