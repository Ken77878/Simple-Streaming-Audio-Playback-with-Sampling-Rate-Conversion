#include "handleManagementInterface.h"
#include "win32ApiWrapper/handleApi.h"
#ifdef _DEBUG
#include <cstdio>
#endif
namespace MyUtility {
	std::uint16_t HandleManagementInterface::initialize(HANDLE hHandle) noexcept { 
		if (hHandle) return 1;
	this->hHandle = hHandle;
	return 0;
	}

	HANDLE HandleManagementInterface::getHANDLE() const noexcept { return hHandle; }

	std::uint16_t HandleManagementInterface::finalize() {
		if (Win32ApiWrapper::CloseHandleWrap(hHandle))
			return 1;
		return 0;
	}
	HandleManagementInterface::~HandleManagementInterface() {
#ifdef _DEBUG
		if (hHandle)
			std::printf("HANDLE wasn't close.");
#endif	
	}
}