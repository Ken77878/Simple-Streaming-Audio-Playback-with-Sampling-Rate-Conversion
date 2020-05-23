#pragma once
#define WIN32_LEAN_AND_MEAN             
#include <Windows.h>
#include <cstdint>
#include "win32ApiWrapperDLLheader.h"

namespace Win32ApiWrapper {
	win32ApiWrapper_API std::uint16_t CreateEventWrap(
		HANDLE& event,
		LPSECURITY_ATTRIBUTES lpEventAttributes,
		BOOL                  bManualReset,
		BOOL                  bInitialState,
		LPCTSTR                lpName);

	win32ApiWrapper_API std::uint16_t  SetEventWrap(
		HANDLE hHandle);
	win32ApiWrapper_API std::uint16_t  ResetEventWrap(
		HANDLE hHandle);

}