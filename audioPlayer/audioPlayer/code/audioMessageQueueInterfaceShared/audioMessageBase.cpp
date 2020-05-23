#include "audioMessageBase.h"

namespace AudioMessage {
Base::Base(AudioMessageId messageId) : messageId(messageId) {}

AudioMessageId Base::getMessageId() const noexcept { return messageId; }
Base::~Base() {}

} // namespace AudioMessage
