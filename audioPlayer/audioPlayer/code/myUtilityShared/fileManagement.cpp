#include "fileManagement.h"
#include "systemErrorInterface.h"
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

namespace MyUtility {
std::uint16_t fopenWrap(FILE *&fp, std::TSTRING &filePath) {
#ifdef WIN32
  errno_t error = tfopen_s(&fp, filePath.c_str(), TEXT("rb"));
  if (error != 0) {
    tprintf(TEXT("failed to open file: %s\n"), filePath.c_str());
    return 1;
  }
#else
  if ((fp = fopen(filePath.c_str(), "rb")) == nullptr) {
    printf("failed to open file: %s\n", filePath.c_str());
    SystemErrorInterface::systemErrorHandlingTask(TEXT("fopen"));
    return 1;
  }
#endif
  return 0;
}

std::uint16_t fseekWrap(FILE *stream, long offset, int origin) {
  if (fseek(stream, offset, origin)) {
    switch (errno) {
    case EBADF:
      std::printf("stream is incorrect.\n");
      break;
    case EINVAL:
      std::printf("origin may be wrong. Or new offset is negative.\n");
      break;
    default:
      SystemErrorInterface::systemErrorHandlingTask(TEXT("fseek"));
      break;
    }
    errno = 0;
    return 1;
  }
  return 0;
}

std::uint16_t ftellWrap(FILE *stream, std::int32_t &offset) {
  if ((offset = ftell(stream)) == -1l) {
    SystemErrorInterface::systemErrorHandlingTask(TEXT("ftell"));
    return 1;
  }
  return 0;
}

std::uint16_t freadWrap(void *bufferPtr, size_t size, size_t count,
                        FILE *stream) {
  size_t returnedSize = fread(bufferPtr, size, count, stream);
  if (returnedSize < count) {
    if (ferror(stream)) {
      SystemErrorInterface::systemErrorHandlingTask(TEXT("fread"));
      return 1;
    }
    if (feof(stream)) {
      printf("end-of-file in fread: programming is wrong.\n");
      return 1;
    }
    return 1;
  }
  return 0;
}

std::uint16_t fcloseWrap(FILE *&stream) {
  if (stream == nullptr)
    return 0;
  if (fclose(stream) == EOF)
    return 1;
  stream = nullptr;
  return 0;
}
} // namespace MyUtility
