#include "eventApi.h"
#include "systemErrorInterface.h"
namespace Win32ApiWrapper {
	std::uint16_t CreateEventWrap(HANDLE& event, LPSECURITY_ATTRIBUTES lpEventAttributes,
		BOOL                  bManualReset,
		BOOL                  bInitialState,
		LPCTSTR                lpName) {
		if ((event = CreateEvent(lpEventAttributes, bManualReset, bInitialState, lpName)) == nullptr) {
			SystemErrorInterface::systemErrorHandlingTask(TEXT("CreateEvent"));
			return 1;
		}
		return 0;
	}


	std::uint16_t  SetEventWrap(
		HANDLE hHandle) {
		if (!::SetEvent(hHandle))
		{
			SystemErrorInterface::systemErrorHandlingTask(TEXT("SetEvent"));
			return 1;
		}
		return 0;
	}

	std::uint16_t  ResetEventWrap(
		HANDLE hHandle) {
		if (!::ResetEvent(hHandle))
		{
			SystemErrorInterface::systemErrorHandlingTask(TEXT("SetEvent"));
			return 1;
		}
		return 0;
	 }

}
