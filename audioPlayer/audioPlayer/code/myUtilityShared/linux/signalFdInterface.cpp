#include "signalFdInterface.h"
#include <signal.h>
#include "systemErrorInterface.h"
#include "linuxApiWrapper/fileDescriptorApi.h"
#include <cstdio>

namespace MyUtility {
std::uint16_t
SignalFdInterface::initialize(std::vector<std::int32_t> &signalNoArray) {
  sigset_t mask;
  sigemptyset(&mask);
  std::uint32_t size = signalNoArray.size();
  for (std::uint32_t u = 0; u < size; ++u) {
    sigaddset(&mask, signalNoArray[u]);
  }
  // Block signals so that they aren't handled
  // according to their default dispositions
  if (pthread_sigmask(SIG_BLOCK, &mask, NULL) == -1)
    SystemErrorInterface::systemErrorHandlingTask(TEXT("sigprocmask"));

  if (fileDescriptor) {
    if (signalfd(fileDescriptor, &mask, 0) == -1) {
      SystemErrorInterface::systemErrorHandlingTask(TEXT("sigprocmask"));
      return 1;
    }
  } else {
    if ((fileDescriptor = signalfd(-1, &mask, 0)) == -1) {
      SystemErrorInterface::systemErrorHandlingTask(TEXT("sigprocmask"));
      return 1;
    }
  }
  return 0;
}
std::uint16_t SignalFdInterface::readTask() {
  struct signalfd_siginfo sigInfoTemp = {0};
  std::uint32_t returnByteSize = 0;

  if (LinuxApiWrapper::readWrap(returnByteSize, fileDescriptor,
                                reinterpret_cast<std::uint8_t *>(&sigInfoTemp),
                                sizeof(struct signalfd_siginfo)))
    return 1;

  if (returnByteSize != sizeof(struct signalfd_siginfo)) {
    printf("signalFd:write EINTR?");
    return 1;
  }
  return 0;
}

} // namespace MyUtility
