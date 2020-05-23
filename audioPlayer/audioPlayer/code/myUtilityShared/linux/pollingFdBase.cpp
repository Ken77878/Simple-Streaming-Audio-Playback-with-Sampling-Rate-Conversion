#include "pollingFdBase.h"
#include "linuxApiWrapper/fileDescriptorApi.h"
#ifdef _DEBUG
// exit
#include <cstdlib>
#include <cstdio>
#endif

namespace MyUtility {
std::int32_t PollingFdBase::getFileDescriptor() const noexcept {
  return fileDescriptor;
}

std::uint16_t PollingFdBase::finalize() {
  if (LinuxApiWrapper::closeWrap(fileDescriptor))
    return 1;
  return 0;
}
PollingFdBase::~PollingFdBase() {
#ifdef _DEBUG
  if (fileDescriptor) {
    printf("fileDescriptor must be close.\n");
    std::exit(300);
  }
#endif
}

} // namespace MyUtility
