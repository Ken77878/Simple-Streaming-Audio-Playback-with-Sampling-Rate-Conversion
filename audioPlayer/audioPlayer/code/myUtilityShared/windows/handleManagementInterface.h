#pragma once
#include "noCopyAndMoveTemplate.h"
#define WIN32_LEAN_AND_MEAN             
#include <Windows.h>
#include <cstdint>
#include "../myUtilityDLLHeader.h"

namespace MyUtility {
	class myUtility_API HandleManagementInterface
		: public NoCopyAndMove<HandleManagementInterface> {
	private:
		HANDLE hHandle = nullptr;
	public:
		std::uint16_t initialize(HANDLE hHandle) noexcept;
		HANDLE getHANDLE() const noexcept;
		std::uint16_t finalize();
		~HandleManagementInterface();
	};
}