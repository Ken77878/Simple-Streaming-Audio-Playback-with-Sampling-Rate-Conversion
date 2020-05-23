#pragma once
#include "../myUtilityDLLHeader.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
namespace MyUtility {
	class   myUtility_API  TimeMeasurement {
	private:
		LARGE_INTEGER freq , startTime , endTime;
		bool faultFlag = false;	
	public:
		TimeMeasurement();
		int start();
		// seconds
		int end(double & durationSeconds);
	};
}
