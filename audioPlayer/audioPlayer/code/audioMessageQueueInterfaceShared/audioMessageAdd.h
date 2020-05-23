#pragma once
#include "audioMessageOneFile.h"
#include <memory>
#include "secondaryBufferBase.h"
#include "audioMessageQueueInterfaceDLLheader.h"

namespace AudioMessage {
class audioMessageQueueInterface_API Add : public OneFile {
private:
  std::unique_ptr<SecondaryBuffer::CommonBase> secondaryBufferUniquePtr;

public:
  Add(AudioMessageId messageId, std::uint32_t fileId,
      std::unique_ptr<SecondaryBuffer::CommonBase> &&secondarayBuffer);
  std::unique_ptr<SecondaryBuffer::CommonBase> &&getSecondaryBufferUniquePtr();
};
} // namespace AudioMessage
