#pragma once

#include <cstdint>
#include "noCopyAndMoveTemplate.h"

namespace MyUtility {
class PollingFdBase : public NoCopyAndMove<PollingFdBase> {
protected:
  std::int32_t fileDescriptor = 0;
  virtual std::uint16_t readTask() = 0;

public:
  PollingFdBase() = default;
  PollingFdBase(const PollingFdBase &) = delete;
  PollingFdBase &operator=(const PollingFdBase &) = delete;
  PollingFdBase(PollingFdBase &&) noexcept = delete;
  PollingFdBase &operator=(PollingFdBase &&) noexcept = delete;

  std::int32_t getFileDescriptor() const noexcept;
  virtual std::uint16_t finalize();
  virtual ~PollingFdBase();
};
} // namespace MyUtility
