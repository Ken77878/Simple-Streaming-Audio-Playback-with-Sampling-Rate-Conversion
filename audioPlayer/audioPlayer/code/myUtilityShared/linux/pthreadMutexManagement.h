#pragma once

#include <cstdint>
#include "pthread.h"
#include "noCopyAndMoveTemplate.h"

namespace MyUtility {
class PthreadMutexManagement : public NoCopyAndMove<PthreadMutexManagement> {
private:
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

public:
  std::uint16_t lock();
  std::uint16_t unlock();
  std::uint16_t finalize();
};
} // namespace MyUtility
