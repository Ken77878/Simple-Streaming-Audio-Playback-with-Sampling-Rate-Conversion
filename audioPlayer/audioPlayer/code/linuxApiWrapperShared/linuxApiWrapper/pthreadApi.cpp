#include "pthreadApi.h"
#include "systemErrorInterface.h"

namespace LinuxApiWrapper {
std::uint16_t pthread_createWrap(pthread_t &thread, pthread_attr_t *attr,
                                 void *(*start_routine)(void *), void *arg) {
  if (pthread_create(&thread, attr, start_routine, arg)) {
    SystemErrorInterface::systemErrorHandlingTask(TEXT("pthread_create"));
    return 1;
  }
  return 0;
}

std::uint16_t pthread_joinWrap(pthread_t thread, void **threadPtrReturnPtr) {
  if (pthread_join(thread, threadPtrReturnPtr)) {
    SystemErrorInterface::systemErrorHandlingTask(TEXT("pthread_create"));
    return 1;
  }
  return 0;
}
} // namespace LinuxApiWrapper
