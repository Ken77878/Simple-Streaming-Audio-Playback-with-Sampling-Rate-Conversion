
#include "threadManagementInterface.h"
#include "linuxApiWrapper/pthreadApi.h"
#ifdef _DEBUG
#include <cstdlib>
#endif

namespace MyUtility {

static void *startRoutine(void *arg) {
  reinterpret_cast<PthreadWrapper *>(arg)->setThreadReturnValue(
      (*(reinterpret_cast<PthreadWrapper *>(arg)
             ->getStartRoutineContentFuncPtr()))(
          reinterpret_cast<PthreadWrapper *>(arg)
              ->getStartRoutineContentFuncArg()));
  pthread_exit(nullptr);
}

std::uint16_t PthreadWrapper::initialize(::startRoutineContentFuncPtr funcPtr,
                                         void *arg) {
  if (existFlag) {
    // errorHandling
    return 1;
  }
  startRoutineContentFuncPtrValue = funcPtr;
  startRoutineContentFuncArg = arg;

  if (LinuxApiWrapper::pthread_createWrap(threadId, nullptr, startRoutine,
                                          this))
    return 1;
  existFlag = true;
  return 0;
}

void PthreadWrapper::setThreadReturnValue(std::uint16_t value) noexcept {
  threadReturnValue = value;
}

std::uint16_t PthreadWrapper::getThreadReturnValue() const noexcept {
  return threadReturnValue;
}

startRoutineContentFuncPtr PthreadWrapper::getStartRoutineContentFuncPtr() const
    noexcept {
  return startRoutineContentFuncPtrValue;
}
void *PthreadWrapper::getStartRoutineContentFuncArg() const noexcept {
  return startRoutineContentFuncArg;
}

std::uint16_t PthreadWrapper::finalize() {
  if (!existFlag)
    return 0;
  if (LinuxApiWrapper::pthread_joinWrap(threadId, nullptr))
    return 1;
  existFlag = false;
  return 0;
}
PthreadWrapper::~PthreadWrapper() {
#ifdef _DEBUG
  if (existFlag)
    std::exit(555);
#endif
}
} // namespace MyUtility
