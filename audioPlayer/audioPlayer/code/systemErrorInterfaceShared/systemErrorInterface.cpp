#include "systemErrorInterface.h"
#ifdef WIN32
#include <strsafe.h>
#include "characterCodeMacro.h"
#include <string>
#else
#include <string.h>
#include <array>
#include "debugMacro.h"

#endif

namespace SystemErrorInterface {
static std::uint16_t printSystemErrorMessage(const TCHAR *lpszFunctionName) {
#ifdef WIN32
  LPVOID lpMsgBuf = nullptr;
  LPVOID lpTotalMessageBuf = nullptr;
  DWORD systemErrorCode = GetLastError();
  if (systemErrorCode == 0)
    return 2;

  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, systemErrorCode,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0,
                NULL);

  // Display the error message

  lpTotalMessageBuf = (LPVOID)LocalAlloc(
      LMEM_ZEROINIT,
      (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunctionName) + 40) *
          sizeof(TCHAR));
  StringCchPrintf((LPTSTR)lpTotalMessageBuf,
                  LocalSize(lpTotalMessageBuf) / sizeof(TCHAR),
                  TEXT("%s failed with error %d: %s"), lpszFunctionName,
                  systemErrorCode, lpMsgBuf);
  std::TSTRING errorMessage = (LPCTSTR)lpTotalMessageBuf;
  tprintf(TEXT("%s\n"), errorMessage.c_str());
  LocalFree(lpMsgBuf);
  LocalFree(lpTotalMessageBuf);

#else
  std::array<char, 256> buffer;
  buffer.fill(0);
#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && !_GNU_SOURCE
  int32_t returnValue = strerror_r(errno, &buffer[0], buffer.size());
  if (!returnValue)
    std::printf("func %s, error code: %d, %s\n", lpszFunctionName, errno,
                &buffer[0]);
  else
    std::printf("seterror_r() failed : errno %d", errno);
#else
  char *bufferPtr = strerror_r(errno, &buffer[0], buffer.size());
  std::printf("func %s, error code: %d, %s\n", lpszFunctionName, errno,
              bufferPtr);
#endif
  errno = 0;
#endif
  return 0;
}

std::uint16_t systemErrorHandlingTask(const TCHAR *lpszFunctionName) {
  return printSystemErrorMessage(lpszFunctionName);
}

} // namespace SystemErrorInterface
