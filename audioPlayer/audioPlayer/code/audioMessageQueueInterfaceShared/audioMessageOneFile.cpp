#include "audioMessageOneFile.h"
namespace AudioMessage {
OneFile::OneFile(AudioMessageId messageId, std::uint32_t fileId)
    : Base(messageId), fileId(fileId) {}

std::uint32_t OneFile::getFileId() const noexcept { return fileId; }
} // namespace AudioMessage
