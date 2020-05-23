#pragma once
#include <cstdint>
#include "audioMessageBase.h"
#include "audioMessageQueueInterfaceDLLheader.h"

namespace AudioMessage {
class  audioMessageQueueInterface_API OneFile : public Base {
private:
  std::uint32_t fileId = 0;

public:
  OneFile(AudioMessageId messageId, std::uint32_t fileId);
  std::uint32_t getFileId() const noexcept;
};
} // namespace AudioMessage
