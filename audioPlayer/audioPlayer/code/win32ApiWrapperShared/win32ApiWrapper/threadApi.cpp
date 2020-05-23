#include "threadApi.h"
#include <process.h>
#include "systemErrorInterface.h"
#include <array>
#include <avrt.h>

namespace Win32ApiWrapper {
	std::uint16_t _beginthreadexWrap(HANDLE& threadHandle, void *security,
		unsigned stack_size,
		unsigned(__stdcall *start_address)(void *),
		void *arglist,
		unsigned initflag,
		unsigned *thrdaddr) {
		if ((threadHandle = reinterpret_cast<HANDLE>(::_beginthreadex(
			security,
			stack_size,
			start_address,
			arglist,
			initflag,
			thrdaddr
		))) == 0) {
			std::array<TCHAR, 256> errorMsgBuffer;
			errorMsgBuffer.fill(TEXT('\0'));

			if (tstrerror_s(&errorMsgBuffer[0], errorMsgBuffer.size(), errno))

				tfprintf(stderr,
				(std::TSTRING(TEXT("_beginthreadex: ")) + &errorMsgBuffer[0]).c_str());
			return 1;
		}

		return 0;
	}

	std::uint16_t OpenThreadWrap(
		HANDLE& threadHandle,
		DWORD dwDesiredAccess,
		BOOL  bInheritHandle,
		DWORD dwThreadId
	) {
		if (!(threadHandle = OpenThread(dwDesiredAccess,
			bInheritHandle,
			dwThreadId)))
		{
			SystemErrorInterface::systemErrorHandlingTask(TEXT("OpenThread"));

			return 1;
		}
		return 0;
	}

	std::uint16_t SetThreadPriorityWrap(
		HANDLE hThread,
		int    nPriority
	) {
		return 0;
		if (!SetThreadPriority(hThread, nPriority))
		{
			SystemErrorInterface::systemErrorHandlingTask(TEXT("SetThreadPriority"));
			return 1;
		}
		return 0;
	}

	std::uint16_t ResumeThreadWrap(
		HANDLE hThread
	) {
		if (ResumeThread(hThread) == -1)
		{
			SystemErrorInterface::systemErrorHandlingTask(TEXT("GetExitCodeThread"));
			return 1;
		}
		return 0;
	}

	std::uint16_t AvSetMmThreadCharacteristicsWrap(
		HANDLE& taskHandle,
		LPCTSTR  TaskName,
		DWORD& TaskIndex
	) {
		if (!(taskHandle = AvSetMmThreadCharacteristics(TaskName, &TaskIndex)))
		{
			SystemErrorInterface::systemErrorHandlingTask(TEXT("AvSetMmThreadCharacteristics"));
		}
		return 0;
	}

	std::uint16_t AvRevertMmThreadCharacteristicsWrap(
		HANDLE taskHandle
	) {
		if (!AvRevertMmThreadCharacteristics(taskHandle))
		{
			SystemErrorInterface::systemErrorHandlingTask(TEXT("AvSetMmThreadCharacteristics"));
		}
		return 0;
	}

}