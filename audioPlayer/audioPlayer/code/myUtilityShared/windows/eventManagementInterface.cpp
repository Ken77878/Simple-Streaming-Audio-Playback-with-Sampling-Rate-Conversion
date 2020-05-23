#include "eventManagementInterface.h"
#include "win32ApiWrapper/eventApi.h"
#include "win32ApiWrapper/handleApi.h"
#ifdef _DEBUG
#include <cstdio>
#endif
namespace MyUtility {
	std::uint16_t EventManagementInterface::initialize(LPSECURITY_ATTRIBUTES lpEventAttributes,
		BOOL                  bManualReset,
		BOOL                  bInitialState,
		LPCTSTR                lpName)  {

		if (handle) return 1;
		if (Win32ApiWrapper::CreateEventWrap(handle, lpEventAttributes, bManualReset,
			bInitialState,
			lpName))
		{
			return 1;
		}
		return 0;
	}

	HANDLE EventManagementInterface::getHandle() const noexcept { return handle; }

	std::uint16_t EventManagementInterface::setEventOn() {
		if (Win32ApiWrapper::SetEventWrap(handle))
		{
			return 1;
		}
		return 0;
	}

	std::uint16_t EventManagementInterface::setEventOff() {
		if (Win32ApiWrapper::ResetEventWrap(handle))
		{
			return 1;
		}
		return 0;
	}

	std::uint16_t EventManagementInterface::waitEvent() {
		if (Win32ApiWrapper::WaitForSingleObjectWrap(handle))
			return 1;
		return 0;
	}

	std::uint16_t EventManagementInterface::waitEvent(bool& timeoutFlag, DWORD  dwMilliseconds) {
		if (Win32ApiWrapper::WaitForSingleObjectWrap(handle,
			timeoutFlag, dwMilliseconds
		))
			return 1;
		return 0;
	}

	std::uint16_t EventManagementInterface::finalize() {
		if (Win32ApiWrapper::CloseHandleWrap(handle))
			return 1;
		return 0;
	}

	EventManagementInterface::~EventManagementInterface() {
#ifdef _DEBUG
		if (handle)
			std::printf("Event handle wasn't close.\n");
#endif	
	}
}