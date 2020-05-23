#pragma once
#include <cstdint>
#include <pthread.h>
#include "noCopyAndMoveTemplate.h"

typedef std::uint16_t (*startRoutineContentFuncPtr)(void *);
namespace MyUtility {
class PthreadWrapper : public NoCopyAndMove<PthreadWrapper> {
private:
  pthread_t threadId;
  // if thread exists
  bool existFlag = false;
  std::uint16_t threadReturnValue = 0;
  void *startRoutineContentArg = nullptr;
  std::uint16_t (*startRoutineContentFuncPtrValue)(void *) = nullptr;
  void *startRoutineContentFuncArg = nullptr;

public:
  std::uint16_t initialize(startRoutineContentFuncPtr funcPtr, void *arg);
  void setThreadReturnValue(std::uint16_t value) noexcept;
  std::uint16_t getThreadReturnValue() const noexcept;
  startRoutineContentFuncPtr getStartRoutineContentFuncPtr() const noexcept;
  void *getStartRoutineContentFuncArg() const noexcept;

  std::uint16_t finalize();

  ~PthreadWrapper();
};

} // namespace MyUtility
