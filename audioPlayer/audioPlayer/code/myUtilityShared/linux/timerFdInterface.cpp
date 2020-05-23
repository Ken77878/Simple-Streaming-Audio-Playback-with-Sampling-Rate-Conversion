#include "timerFdInterface.h"
#include "linuxApiWrapper/fileDescriptorApi.h"
#include "systemErrorInterface.h"

#include <cstdio>

namespace MyUtility {
std::uint16_t TimerFdInterface::initialize() {
  if ((fileDescriptor = timerfd_create(CLOCK_MONOTONIC, 0)) == -1) {
    return 1;
  }
  return 0;
}
std::uint16_t TimerFdInterface::setTimer(time_t seconds,
                                         std::int32_t nanoSeconds) {
  struct itimerspec timerInfo = {0};
  timerInfo.it_value.tv_sec = seconds;
  timerInfo.it_value.tv_nsec = nanoSeconds;
  timerInfo.it_interval.tv_sec = timerInfo.it_value.tv_sec;
  timerInfo.it_interval.tv_nsec = timerInfo.it_value.tv_nsec;
  if (timerfd_settime(fileDescriptor, 0, &timerInfo, nullptr) == -1) {
    SystemErrorInterface::systemErrorHandlingTask(TEXT("read"));
    return 1;
  }
  return 0;
}

std::uint16_t TimerFdInterface::readTask() {
  std::uint64_t timerBuffer = 0;
  std::uint32_t returnByteSize = 0;
  if (LinuxApiWrapper::readWrap(returnByteSize, fileDescriptor,
                                reinterpret_cast<std::uint8_t *>(&timerBuffer),
                                sizeof(std::uint64_t)))
    return 1;
  if (returnByteSize != sizeof(std::uint64_t)) {
    printf("timerFd:read EINTR?");
    return 1;
  }
  return 0;
}
} // namespace MyUtility
