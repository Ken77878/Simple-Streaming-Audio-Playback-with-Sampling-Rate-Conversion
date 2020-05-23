#include "timepieceInterface.h"
#include "errno.h"
#include "systemErrorInterface.h"
#include <vector>
#include <cstdio>
clockid_t TimepieceInterface::clockId = CLOCK_REALTIME;
std::uint16_t TimepieceInterface::initialize() {
  if (clock_getres(CLOCK_MONOTONIC_RAW, &startTime) != -1) {
    clockId = CLOCK_MONOTONIC_RAW;
    return 0;
  }
  if (errno != EINVAL) {
    SystemErrorInterface::systemErrorHandlingTask(TEXT("clock_getres"));
    return 1;
  }
  if (clock_getres(CLOCK_MONOTONIC, &startTime) != -1) {
    clockId = CLOCK_MONOTONIC;
    return 0;
  }
  if (errno != EINVAL) {
    SystemErrorInterface::systemErrorHandlingTask(TEXT("clock_getres"));
    return 1;
  }
  if (clock_getres(CLOCK_REALTIME, &startTime) != -1) {
    clockId = CLOCK_REALTIME;
    return 0;
  }
  SystemErrorInterface::systemErrorHandlingTask(TEXT("clock_getres"));
  return 1;
}
std::uint16_t TimepieceInterface::start() {
  if (clock_gettime(CLOCK_REALTIME, &startTime) == -1)
    return 1;
  return 0;
}
std::uint16_t TimepieceInterface::end() {
  if (clock_gettime(CLOCK_REALTIME, &endTime))
    return 1;

  std::int64_t interval = (endTime.tv_sec - startTime.tv_sec) * 1000000000 +
                          endTime.tv_nsec - startTime.tv_nsec;
  std::printf("%ld nano sec\n", interval);
  return 0;
}
