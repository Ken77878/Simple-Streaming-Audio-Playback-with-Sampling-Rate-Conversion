#include "criticalSectionManagement.h"


namespace MyUtility {
	CriticalSectionManagementInterface::CriticalSectionManagementInterface() {
		InitializeCriticalSection(&criticalSectionObject);
	}
	void CriticalSectionManagementInterface::enter() {
		EnterCriticalSection(&criticalSectionObject);
	}
	void CriticalSectionManagementInterface::leave() { 
		LeaveCriticalSection(&criticalSectionObject); }
	CriticalSectionManagementInterface::~CriticalSectionManagementInterface() {
		DeleteCriticalSection(&criticalSectionObject);
	}
}