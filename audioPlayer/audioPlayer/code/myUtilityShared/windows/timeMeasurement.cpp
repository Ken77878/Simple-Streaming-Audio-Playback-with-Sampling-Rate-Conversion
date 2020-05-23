#include "timeMeasurement.h"

namespace MyUtility {
	TimeMeasurement::TimeMeasurement() :faultFlag(false) {}
	int TimeMeasurement::start() {
		//	std::cout << "QueryPerformanceCounter():\n";
		if (!QueryPerformanceFrequency(&freq))      // ’PˆÊK“¾
		{
			faultFlag = true;
			return 1;
		}
		if (!::QueryPerformanceCounter(&startTime))
		{
			faultFlag = true;
			return 1;
		}
		return 0;
	}

	int TimeMeasurement::end(double & durationSeconds) {
		if (faultFlag)
			return 1;
		if (!QueryPerformanceCounter(&endTime))
			return 1;    // æ“¾¸”s
		//seconds
		durationSeconds = (double)(endTime.QuadPart - startTime.QuadPart) / freq.QuadPart;
		return 0;
	}
}