#pragma once
#include <cstdint>
#include <vector>
#include <sys/signalfd.h>
#include "pollingFdBase.h"
namespace MyUtility {
class SignalFdInterface : public PollingFdBase {
private:
  // must be blocking fd
public:
  std::uint16_t initialize(std::vector<std::int32_t> &signalNoArray);
  std::uint16_t readTask();
};
} // namespace MyUtility
