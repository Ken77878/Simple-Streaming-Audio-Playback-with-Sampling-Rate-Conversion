#include "getBinaryFileSize.h"
#include <iostream>

#ifdef WIN32
#include <io.h>    //_sopen_s
#include <stdio.h> //_fdopen
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else

#include <sys/types.h>
#include <sys/stat.h>
#endif
#include <fcntl.h> //_O_RDONLY | _O_BINARY

namespace MyUtility {
std::uint16_t getBinaryFileSize(const std::TSTRING &filePath,
                                std::uint32_t &dataSize) {
  FILE *fp = nullptr;
  dataSize = 0;
  struct stat stbuf = {0};
  int fd = 0;

  // Open a file.
#ifdef WIN32
  if (_tsopen_s(&fd, filePath.c_str(), _O_RDONLY | _O_BINARY, _SH_DENYWR, 0)) {
    std::tcerr << TEXT("_sopen_s error: failed to open file: ") << filePath
               << std::endl;
    return 1;
  }

  fp = _fdopen(fd, "rb");
  if (fp == NULL) {
    std::tcerr
        << TEXT("_fdopen error: failed to connect file discriptor to stream")
        << std::endl;
    return 1;
  }
#else
  fd = open(filePath.c_str(), O_RDONLY);
  if (fd == -1) {
    std::tcerr << TEXT("error: failed to open imagefile") << filePath
               << std::endl;
    return 1;
  }

  fp = fdopen(fd, "rb");
  if (fp == NULL) {
    std::tcerr << TEXT("error: failed to connect file discriptor to stream")
               << std::endl;
    return 1;
  }

#endif
  if (fstat(fd, &stbuf) == -1) {
    std::tcout << TEXT("error: failed to fstat") << std::endl;
    return 1;
  }

  dataSize = stbuf.st_size;

  // After _fdopen, close by using fclose, not _close.
  if (fclose(fp) != 0) {
    std::tcerr << TEXT("error: failed to close file ") << filePath << std::endl;
    return 1;
  }

  return 0;
}
} // namespace MyUtility
