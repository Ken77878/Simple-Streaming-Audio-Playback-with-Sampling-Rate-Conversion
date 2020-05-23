#pragma once
#include "noCopyAndMoveTemplate.h"
#define WIN32_LEAN_AND_MEAN             
#include <Windows.h>
#include <cstdint>
#include "characterCodeMacro.h"
#include <string>
#include "../myUtilityDLLHeader.h"

typedef unsigned(__stdcall *threadFuncAddress)(void *);

namespace MyUtility {

	class myUtility_API ThreadManagementInterface
		: public NoCopyAndMove<ThreadManagementInterface> {
	private:
		HANDLE threadHandle = nullptr;
		std::uint32_t id = 0;
		std::uint16_t threadReturnValue = 0;
		threadFuncAddress  threadTaskFuncPtr = nullptr;
		void * threadFuncArgsPtr = nullptr;
	    HANDLE threadTaskHandle = nullptr;
		DWORD threadTaskIndex = 0;
		std::TSTRING threadTaskName;
		std::uint16_t setThreadCharacteristics();
		std::uint16_t resetThreadCharacteristics();
	public:
	    void setThreadTaskName(std::TSTRING&& threadTaskName );
		void setThreadReturnValue(std::uint16_t returnValue) noexcept;
	
		std::uint16_t threadFuncContent();
		std::uint16_t initialize(unsigned(__stdcall *start_address)(void *),
			void *arglist, const std::TSTRING& threadTaskName) ;
		std::uint32_t getThreadId() const noexcept;
		std::uint16_t finalize();
		~ThreadManagementInterface();
	};
}