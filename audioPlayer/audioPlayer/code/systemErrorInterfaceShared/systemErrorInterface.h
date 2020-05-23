#pragma once

#include "systemErrorInterfaceDLLheader.h"
#include <cstdint>
#include "characterCodeMacro.h"
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

namespace SystemErrorInterface {
systemErrorInterface_API std::uint16_t
systemErrorHandlingTask(const TCHAR *lpszFunctionName);
}
