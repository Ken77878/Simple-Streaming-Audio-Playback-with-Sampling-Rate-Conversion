#pragma once
#include "epollInterfaceBase.h"
#include <unordered_map>
#include "timerFdInterface.h"
#include "eventFdInterface.h"
#include <memory>
namespace MyUtility {

struct EventInfo {
  EventFdInterface eventObject;
  // autoReset(when waitEvent return 0)
  bool autoResetFlag = true;
  TimerFdInterface timerObject;
};

class EventWaitingInterface : public EpollInterfaceBase {
private:
  std::unordered_map<std::int32_t, std::unique_ptr<EventInfo>>
      eventInfoContainer;
  std::uint16_t epollEventTask(std::int32_t eventFd, bool &readyFlag);
  std::uint16_t epollEventTask(std::int32_t eventFd, bool &readyFlag,
                               bool &timeoutFlag);

public:
  std::uint16_t registerEvent(std::int32_t &eventFd, bool autoResetFlag = true,
                              bool initialState = false);
  std::uint16_t eraseEvent(std::int32_t eventFd);
  std::uint16_t setEventOn(std::int32_t eventFd);
  std::uint16_t setEventOff(std::int32_t eventFd);
  std::uint16_t waitEvent(std::int32_t eventFd);
  std::uint16_t waitEvent(std::int32_t eventFd, bool &timeoutFlag,
                          std::uint32_t milliSceconds);
  std::uint16_t finalize();
  ~EventWaitingInterface();
};

} // namespace MyUtility
