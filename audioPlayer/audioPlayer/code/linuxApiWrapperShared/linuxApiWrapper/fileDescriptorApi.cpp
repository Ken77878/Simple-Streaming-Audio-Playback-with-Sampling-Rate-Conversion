#include "fileDescriptorApi.h"
#include <unistd.h>
#include "systemErrorInterface.h"
#include <cerrno>

#include <cstdio>

namespace LinuxApiWrapper {

std::uint16_t writeWrap(std::uint32_t &returnByteSize, std::int32_t fd,
                        const void *bufferPtr, std::size_t byteCount) {
  std::int32_t returnValue = 0;
  if ((returnValue = write(fd, bufferPtr, byteCount)) == -1) {
    if (errno == EINTR) {
      if (writeWrap(returnByteSize, fd, bufferPtr, byteCount))
        return 1;
    } else {
      SystemErrorInterface::systemErrorHandlingTask(TEXT("write"));
      return 1;
    }
  }
  returnByteSize = static_cast<std::uint32_t>(returnValue);
  return 0;
}

std::uint16_t writeWrap(std::uint32_t &returnByteSize, std::int32_t fd,
                        const void *bufferPtr, std::size_t byteCount,
                        bool &eagainFlag) {
  std::int32_t returnValue = 0;
  if ((returnValue = write(fd, bufferPtr, byteCount)) == -1) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      eagainFlag = true;
      return 0;
    } else if (errno == EINTR) {
      if (writeWrap(returnByteSize, fd, bufferPtr, byteCount, eagainFlag))
        return 1;
    } else {
      SystemErrorInterface::systemErrorHandlingTask(TEXT("write"));
      return 1;
    }
  }
  returnByteSize = static_cast<std::uint32_t>(returnValue);
  return 0;
}

std::uint16_t readWrap(std::uint32_t &returnByteSize, std::int32_t fd,
                       void *bufferPtr, std::size_t byteCount) {
  std::int32_t returnValue = 0;
  if ((returnValue = read(fd, bufferPtr, byteCount)) == -1) {
    if (errno == EINTR) {
      if (readWrap(returnByteSize, fd, bufferPtr, byteCount))
        return 1;

    } else {
      printf("byteCount: %u\n", byteCount);
      SystemErrorInterface::systemErrorHandlingTask(TEXT("read"));
      return 1;
    }
  }
  returnByteSize = static_cast<std::uint32_t>(returnValue);
  return 0;
}

std::uint16_t readWrap(std::uint32_t &returnByteSize, std::int32_t fd,
                       void *bufferPtr, std::size_t byteCount,
                       bool &eagainFlag) {
  std::int32_t returnValue = 0;
  if ((returnValue = read(fd, bufferPtr, byteCount)) == -1) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      eagainFlag = true;
      return 0;
    } else if (errno == EINTR) {
      if (readWrap(returnByteSize, fd, bufferPtr, byteCount, eagainFlag))
        return 1;

    } else {
      printf("byteCount: %u\n", byteCount);
      SystemErrorInterface::systemErrorHandlingTask(TEXT("read"));
      return 1;
    }
  }
  returnByteSize = static_cast<std::uint32_t>(returnValue);
  return 0;
}

std::uint16_t closeWrap(std::int32_t &fileDescriptor) {
  if (!fileDescriptor)
    return 0;
  if (close(fileDescriptor) == -1) {
    SystemErrorInterface::systemErrorHandlingTask(TEXT("close"));
    return 1;
  } else
    fileDescriptor = 0;
  return 0;
}

} // namespace LinuxApiWrapper
