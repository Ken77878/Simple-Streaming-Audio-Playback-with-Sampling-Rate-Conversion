#pragma once
#include <ctime>
#include <cstdint>
#include "noCopyAndMoveTemplate.h"

class TimepieceInterface : public NoCopyAndMove<TimepieceInterface> {
private:
  static clockid_t clockId;
  struct timespec startTime;
  struct timespec endTime;

public:
  std::uint16_t initialize();
  std::uint16_t start();
  std::uint16_t end();
};
