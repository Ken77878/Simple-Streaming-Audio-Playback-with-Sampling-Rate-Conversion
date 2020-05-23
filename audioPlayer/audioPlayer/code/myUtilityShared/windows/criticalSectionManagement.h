#pragma once
#include "noCopyAndMoveTemplate.h"
#define WIN32_LEAN_AND_MEAN             
#include <Windows.h>
#include <cstdint>
#include "../myUtilityDLLHeader.h"

namespace MyUtility {
	class myUtility_API CriticalSectionManagementInterface
		: public NoCopyAndMove<CriticalSectionManagementInterface> {
	private:
		CRITICAL_SECTION criticalSectionObject;
	public:
		CriticalSectionManagementInterface();
		void enter();
		void leave();
		~CriticalSectionManagementInterface();
	};
}