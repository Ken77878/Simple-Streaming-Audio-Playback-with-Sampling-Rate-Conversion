#include "threadManagementInterface.h"
#include "win32ApiWrapper/threadApi.h"
#include "win32ApiWrapper/handleApi.h"
#include <cstdio>


namespace MyUtility {
	void ThreadManagementInterface::setThreadTaskName(std::TSTRING&& threadTaskName) {
		this->threadTaskName = threadTaskName;
	}


	void ThreadManagementInterface::setThreadReturnValue(std::uint16_t returnValue) noexcept {
		threadReturnValue = returnValue;
	}


	std::uint16_t ThreadManagementInterface::setThreadCharacteristics() {
		if (threadTaskName == TEXT(""))
			return 0;
		if (Win32ApiWrapper::AvSetMmThreadCharacteristicsWrap(
			threadTaskHandle,
			threadTaskName.c_str(), threadTaskIndex))
		{
			return 1;
		}
		return 0;
	}

	std::uint16_t ThreadManagementInterface::resetThreadCharacteristics() {
		if (!threadTaskHandle)
			return 0;
		if (Win32ApiWrapper::AvRevertMmThreadCharacteristicsWrap(
			threadTaskHandle))
		{
			return 1;
		}
		threadTaskHandle = nullptr;
		return 0;
	}

	std::uint16_t ThreadManagementInterface::threadFuncContent() {
		if (setThreadCharacteristics())
			return 1;
		if ((*threadTaskFuncPtr)(threadFuncArgsPtr))
			return 1;
		if (resetThreadCharacteristics())
			return 1;
		return 0;
	}

	static		unsigned __stdcall threadFunc(void * threadManagementInterfaceObjPtr) {
		reinterpret_cast<ThreadManagementInterface*>(threadManagementInterfaceObjPtr)->setThreadReturnValue(
			reinterpret_cast<ThreadManagementInterface*>(threadManagementInterfaceObjPtr)->
			threadFuncContent()
		);
		return 0;
	}

	uint16_t ThreadManagementInterface::initialize(
		unsigned(__stdcall *start_address)(void *),
		void *arglist,const  std::TSTRING& threadTaskName
	) {
		if (threadHandle) {
			std::fprintf(stderr, "Thread can't be created because thread still exists.");
			return 1;
		}

		this->threadTaskName = threadTaskName;
		threadTaskIndex = 0;
		threadTaskFuncPtr = start_address;
		threadFuncArgsPtr = arglist;
		if (Win32ApiWrapper::_beginthreadexWrap(
			threadHandle, nullptr,
			0,
			&threadFunc,
			this,
			CREATE_SUSPENDED,
			&id
		))
			return 1;
		HANDLE specialThreadHandle = nullptr;
		if (Win32ApiWrapper::OpenThreadWrap(specialThreadHandle,
			THREAD_SET_INFORMATION, false, id))
			return 1;
		if (Win32ApiWrapper::SetThreadPriorityWrap(specialThreadHandle, THREAD_PRIORITY_TIME_CRITICAL))
			return 1;
		if (Win32ApiWrapper::CloseHandleWrap(specialThreadHandle))
			return 1;

		if (Win32ApiWrapper::ResumeThreadWrap(threadHandle))
			return 1;
		return 0;
	}

std::uint32_t ThreadManagementInterface::getThreadId() const noexcept{
	return id;
}

	std::uint16_t ThreadManagementInterface::finalize() {
		if (threadHandle)
		{
			if (Win32ApiWrapper::WaitForSingleObjectWrap(threadHandle))
				return 1;
			if (Win32ApiWrapper::CloseHandleWrap(threadHandle))
				return 1;
			if (threadReturnValue) {
				std::fprintf(stderr, "Thread can't be created because Thread still exists.");
				return 1;
			}
		}
		return 0;
	}
	ThreadManagementInterface::
		~ThreadManagementInterface() {
#ifdef _DEBUG
		if (threadHandle)
			fprintf(stderr, "ThreadHandle wasn't  close");
		if (threadTaskHandle)
			fprintf(stderr, "ThreadTaskHandle wasn't  close");;
#endif

	}
}