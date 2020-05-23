#include "eventFdInterface.h"
#include <sys/eventfd.h>
#include "systemErrorInterface.h"
#include "linuxApiWrapper/fileDescriptorApi.h"
#include <cstdio>
namespace MyUtility {
std::uint16_t EventFdInterface::initialize() {
  if ((fileDescriptor = eventfd(0, 0)) == -1) {
    SystemErrorInterface::systemErrorHandlingTask(TEXT("eventfd"));
    return 1;
  }
  return 0;
}
std::uint16_t EventFdInterface::writeTask() {
  std::uint64_t eventBuffer = 1;
  std::uint32_t returnByteSize = 0;

  if (LinuxApiWrapper::writeWrap(returnByteSize, fileDescriptor,
                                 reinterpret_cast<std::uint8_t *>(&eventBuffer),
                                 sizeof(std::uint64_t)))
    return 1;

  if (returnByteSize != sizeof(std::uint64_t)) {
    printf("eventFd:write EINTR?");
    return 1;
  }

  return 0;
}
std::uint16_t EventFdInterface::readTask() {
  std::uint64_t eventBuffer = 1;
  std::uint32_t returnByteSize = 0;

  if (LinuxApiWrapper::readWrap(returnByteSize, fileDescriptor,
                                reinterpret_cast<std::uint8_t *>(&eventBuffer),
                                sizeof(std::uint64_t)))
    return 1;
  if (returnByteSize != sizeof(std::uint64_t)) {
    printf("eventFd:read EINTR?");
    return 1;
  }

  return 0;
}

} // namespace MyUtility
