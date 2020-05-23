#pragma once
#include <cstdint>
#include <vector>
#include <sys/timerfd.h>
#include "pollingFdBase.h"
namespace MyUtility {
class TimerFdInterface : public PollingFdBase {
private:
public:
  std::uint16_t initialize();
  std::uint16_t setTimer(time_t seconds, std::int32_t nanoSeconds);
  std::uint16_t readTask();
};
} // namespace MyUtility
