#include "handleApi.h"
#include "systemErrorInterface.h"
#ifdef _DEBUG
#include <cstdlib>
#endif
#include <cstdio>

namespace Win32ApiWrapper {
	std::uint16_t  WaitForSingleObjectWrap(
		HANDLE hHandle
	)
	{
		if (WaitForSingleObject(hHandle, INFINITE) != WAIT_OBJECT_0)
		{
			SystemErrorInterface::systemErrorHandlingTask(TEXT("WaitForSigleObject"));
			return 1;
		}
		return 0;
	}

	win32ApiWrapper_API std::uint16_t  WaitForSingleObjectWrap(
		HANDLE hHandle, bool& timeoutFlag, DWORD  dwMilliseconds
	) {
		DWORD result = WaitForSingleObject(hHandle, dwMilliseconds);

		switch (result) {
		case WAIT_OBJECT_0:
			timeoutFlag = false;
			break;
		case	WAIT_TIMEOUT:
			timeoutFlag = true;
			break;
		case WAIT_FAILED:
			SystemErrorInterface::systemErrorHandlingTask(TEXT("WaitForSigleObject"));
			return 1;
			break;
		default:
			fprintf(stderr, "WaitForSingleObject: unknown return value\n");
			break;
		}
		return 0;
	}

	std::uint16_t CloseHandleWrap(HANDLE& hHandle) {
		if (!hHandle)
			return 0;
		if (!::CloseHandle(hHandle))
		{
			SystemErrorInterface::systemErrorHandlingTask(TEXT("CloseHandle"));
			return 1;
		}
		hHandle = nullptr;
		return 0;
	}



}