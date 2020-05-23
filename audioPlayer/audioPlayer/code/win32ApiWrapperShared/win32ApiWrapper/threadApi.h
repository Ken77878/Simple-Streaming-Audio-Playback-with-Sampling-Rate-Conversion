#pragma once
#define WIN32_LEAN_AND_MEAN             
#include <Windows.h>
#include <cstdint>
#include "win32ApiWrapperDLLheader.h"

namespace Win32ApiWrapper {

	win32ApiWrapper_API std::uint16_t _beginthreadexWrap(HANDLE& threadHandle, void *security,
		unsigned stack_size,
		unsigned(__stdcall *start_address)(void *),
		void *arglist,
		unsigned initflag,
		unsigned *thrdaddr);

	win32ApiWrapper_API std::uint16_t OpenThreadWrap(
		HANDLE& threadHandle,
		DWORD dwDesiredAccess,
		BOOL  bInheritHandle,
		DWORD dwThreadId
	);

	win32ApiWrapper_API std::uint16_t SetThreadPriorityWrap(
		HANDLE hThread,
		int    nPriority
	);

	win32ApiWrapper_API std::uint16_t ResumeThreadWrap(
		HANDLE hThread
	);

	win32ApiWrapper_API std::uint16_t AvSetMmThreadCharacteristicsWrap(
		HANDLE& taskHandle,
		LPCTSTR  TaskName,
		DWORD& TaskIndex
	);

	win32ApiWrapper_API std::uint16_t AvRevertMmThreadCharacteristicsWrap(
		HANDLE taskHandle
	); 
}