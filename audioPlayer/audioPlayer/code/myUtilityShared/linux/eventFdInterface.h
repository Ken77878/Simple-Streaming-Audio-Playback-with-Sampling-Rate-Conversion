#pragma once
#include <cstdint>
#include <vector>
#include "pollingFdBase.h"
namespace MyUtility {
class EventFdInterface : public PollingFdBase {
private:
public:
  std::uint16_t initialize();
  std::uint16_t writeTask();
  std::uint16_t readTask();
};
} // namespace MyUtility
