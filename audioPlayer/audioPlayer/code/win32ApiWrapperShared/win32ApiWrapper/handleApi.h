#pragma once
#define WIN32_LEAN_AND_MEAN             
#include <Windows.h>
#include <cstdint>
#include "win32ApiWrapperDLLheader.h"

namespace Win32ApiWrapper {
	win32ApiWrapper_API std::uint16_t  WaitForSingleObjectWrap(
		HANDLE hHandle
	);
	win32ApiWrapper_API std::uint16_t  WaitForSingleObjectWrap(
		HANDLE hHandle,bool& timeoutFlag,  DWORD  dwMilliseconds = INFINITE
	);
	win32ApiWrapper_API std::uint16_t CloseHandleWrap(HANDLE& hHandle);
	
}