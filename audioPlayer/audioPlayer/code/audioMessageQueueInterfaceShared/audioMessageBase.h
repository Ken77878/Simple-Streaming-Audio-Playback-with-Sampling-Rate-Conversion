#pragma once
#include <cstdint>
#include "audioMessageIdEnumerance.h"
#include "audioMessageQueueInterfaceDLLheader.h"
#include "noCopyAndMoveTemplate.h"

namespace AudioMessage {
class audioMessageQueueInterface_API Base : public NoCopyAndMove<Base> {
private:
  AudioMessageId messageId = AudioMessageId::stop;

public:
  Base(AudioMessageId messageId);
  AudioMessageId getMessageId() const noexcept;
  virtual ~Base();
};
} // namespace AudioMessage
