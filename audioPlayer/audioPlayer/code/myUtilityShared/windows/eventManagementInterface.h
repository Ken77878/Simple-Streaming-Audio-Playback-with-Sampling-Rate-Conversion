#pragma once
#include "noCopyAndMoveTemplate.h"
#define WIN32_LEAN_AND_MEAN             
#include <Windows.h>
#include <cstdint>
#include "../myUtilityDLLHeader.h"

namespace MyUtility {
	class myUtility_API EventManagementInterface
		: public NoCopyAndMove<EventManagementInterface> {
	private:
		HANDLE handle = nullptr;
	public:
		EventManagementInterface() = default;
		std::uint16_t initialize(	LPSECURITY_ATTRIBUTES lpEventAttributes,
		BOOL                  bManualReset,
		BOOL                  bInitialState,
		LPCTSTR                lpName);
		HANDLE getHandle() const noexcept;
		std::uint16_t setEventOn();
		std::uint16_t setEventOff();
		std::uint16_t waitEvent();
		std::uint16_t waitEvent( bool& timeoutFlag, DWORD  dwMilliseconds);
		std::uint16_t finalize();
		~EventManagementInterface();
	};
}