#include "pthreadMutexManagement.h"
#include <cstdio>

namespace MyUtility {
std::uint16_t PthreadMutexManagement::lock() {
  if (pthread_mutex_lock(&mutex)) {
    fprintf(stderr, "pthread_mutex_lock error.");
    return 1;
  }
  return 0;
}
std::uint16_t PthreadMutexManagement::unlock() {
  if (pthread_mutex_unlock(&mutex)) {
    fprintf(stderr, "pthread_mutex_lock error.");
    return 1;
  }
  return 0;
}
std::uint16_t PthreadMutexManagement::finalize() {
  // only check unlock
  if (pthread_mutex_destroy(&mutex)) {
    fprintf(stderr, "pthread_mutex_destroy error.");
    return 1;
  }
  return 0;
}
} // namespace MyUtility
