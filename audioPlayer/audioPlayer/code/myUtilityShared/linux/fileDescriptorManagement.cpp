// fcntl()
#include <fcntl.h>
#include "fileDescriptorManagement.h"
#include "systemErrorInterface.h"
#include "linuxApiWrapper/fileDescriptorApi.h"

namespace MyUtility {
std::uint16_t writeAll(std::int32_t fd, const void *bufferPtr,
                       std::uint32_t &remainByteSize) {

  std::uint32_t returnByteSize = 0;
  bool eagainFlag = false;
  if (LinuxApiWrapper::writeWrap(returnByteSize, fd, bufferPtr, remainByteSize,
                                 eagainFlag))
    return 1;
  else {
    if (eagainFlag) {
      return 0;
    }
    remainByteSize -= returnByteSize;
    if (!remainByteSize) {
      return 0;
    }
    if (writeAll(fd,
                 reinterpret_cast<const std::uint8_t *>(bufferPtr) +
                     returnByteSize,
                 remainByteSize))
      return 1;
  }
  return 0;
}

std::uint16_t readAll(std::int32_t fd, void *bufferPtr,
                      std::uint32_t &remainByteSize) {
  std::uint32_t returnByteSize = 0;
  bool eagainFlag = false;

  if (LinuxApiWrapper::readWrap(returnByteSize, fd, bufferPtr, remainByteSize,
                                eagainFlag))
    return 1;
  else {
    if (eagainFlag) {
      return 0;
    }
    remainByteSize -= returnByteSize;
    if (!remainByteSize) {
      return 0;
    }
    if (readAll(fd,
                reinterpret_cast<std::uint8_t *>(bufferPtr) + returnByteSize,
                remainByteSize))
      return 1;
  }
  return 0;
}

std::uint16_t setFileDescriptorNonblocking(std::int32_t fileDiscriptor) {
  std::int32_t flag = 0;
  if ((flag = fcntl(fileDiscriptor, F_GETFL, 0)) <
      0) { // ERR_MSG("fcntl(F_GETFL) failed!\n");
    SystemErrorInterface::systemErrorHandlingTask(TEXT("fcntl"));
    return 1;
  }
  if (fcntl(fileDiscriptor, F_SETFL, flag | O_NONBLOCK) <
      0) { // ERR_MSG("fcntl(F_SETFL) failed!\n");
    SystemErrorInterface::systemErrorHandlingTask(TEXT("fcntl"));
    return 01;
  }
  return 0;
}

} // namespace MyUtility
