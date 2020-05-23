#include "eventWaitingInterface.h"
#include <sys/eventfd.h>

#include <linux/version.h>
// memset
#include <cstring>
// write
#include <unistd.h>
#include <errno.h>
#include "systemErrorInterface.h"
#include "linuxApiWrapper/fileDescriptorApi.h"
// printf
#include <cstdio>
#include <cstdlib>
#include "debugMacro.h"
#define INFINITE 0xFFFFFFF

namespace MyUtility {
std::uint16_t EventWaitingInterface::registerEvent(std::int32_t &eventFd,
                                                   bool autoResetFlag,

                                                   bool initialState) {

  std::unique_ptr<EventInfo> eventInfoTempPtr(new EventInfo);
  if (eventInfoTempPtr->eventObject.initialize())
    return 1;
  eventFd = eventInfoTempPtr->eventObject.getFileDescriptor();

  eventInfoContainer.insert(std::pair<std::int32_t, std::unique_ptr<EventInfo>>(
      eventFd, std::move(eventInfoTempPtr)));

  eventInfoContainer[eventFd]->autoResetFlag = autoResetFlag;

  if (initialState)
    if (eventInfoContainer[eventFd]->eventObject.writeTask())
      return 1;
  return 0;
}

std::uint16_t EventWaitingInterface::eraseEvent(std::int32_t eventFd) {
  decltype(eventInfoContainer)::iterator ite = eventInfoContainer.find(eventFd);
  if (ite == eventInfoContainer.end())
    return 1;

  if (eventInfoContainer[eventFd]->eventObject.finalize())
    return 1;
  if (eventInfoContainer[eventFd]->timerObject.finalize())
    return 1;
  eventInfoContainer.erase(eventFd);
  return 0;
}

std::uint16_t EventWaitingInterface::setEventOn(std::int32_t eventFd) {
  if (eventInfoContainer[eventFd]->eventObject.writeTask())
    return 1;
  return 0;
}
std::uint16_t EventWaitingInterface::setEventOff(std::int32_t eventFd) {
  if (eventInfoContainer[eventFd]->eventObject.readTask())
    return 1;
  return 0;
}

std::uint16_t EventWaitingInterface::epollEventTask(std::int32_t eventFd,
                                                    bool &readyFlag) {
  for (std::uint32_t u = 0; u < fdQuantity; ++u) {
    if (availableEpollEventArray[u].data.fd ==
        eventInfoContainer[eventFd]->eventObject.getFileDescriptor()) {
      if (checkEpollEventError(availableEpollEventArray[u].events))
        return 1;

      if (eventInfoContainer[eventFd]->autoResetFlag)
        if (eventInfoContainer[eventFd]->eventObject.readTask())
          return 1;
      readyFlag = true;
      return 0;
    }
  }
  return 0;
}
std::uint16_t EventWaitingInterface::epollEventTask(std::int32_t eventFd,

                                                    bool &readyFlag,
                                                    bool &timeoutFlag) {
  for (std::uint32_t u = 0; u < fdQuantity; ++u) {
    if (availableEpollEventArray[u].data.fd ==
        eventInfoContainer[eventFd]->eventObject.getFileDescriptor()) {
      if (checkEpollEventError(availableEpollEventArray[u].events))
        return 1;

      if (eventInfoContainer[eventFd]->autoResetFlag)
        if (eventInfoContainer[eventFd]->eventObject.readTask())
          return 1;
      readyFlag = true;
      timeoutFlag = false;
      return 0;
    } else if (availableEpollEventArray[u].data.fd ==
               eventInfoContainer[eventFd]->timerObject.getFileDescriptor()) {
      if (eventInfoContainer[eventFd]->timerObject.readTask())
        return 1;
      if (eventInfoContainer[eventFd]->timerObject.setTimer(0, 0))
        return 1;
      if (eraseFileDescriptor(
              eventInfoContainer[eventFd]->timerObject.getFileDescriptor()))
        return 1;
      readyFlag = false;
      timeoutFlag = true;
    }
  }
  return 0;
}

std::uint16_t EventWaitingInterface::waitEvent(std::int32_t eventFd) {
  if (registerFileDescriptor(eventFd))
    return 1;
  bool readyFlag = false;
  do {
    if (waitEpollEvent())
      return 1;
    DEBUG_PRINT("epoll event came.\n");
    if (epollEventTask(eventFd, readyFlag))
      return 1;
  } while (!readyFlag);

  if (eraseFileDescriptor(eventFd))
    return 1;

  return 0;
}

std::uint16_t EventWaitingInterface::waitEvent(std::int32_t eventFd,
                                               bool &timeoutFlag,
                                               std::uint32_t milliSeconds) {
  if (registerFileDescriptor(eventFd))
    return 1;

  if (eventInfoContainer[eventFd]->timerObject.initialize())
    return 1;
  if (registerFileDescriptor(
          eventInfoContainer[eventFd]->timerObject.getFileDescriptor()))
    return 1;

  std::uint32_t seconds = milliSeconds / 1000;
  std::uint32_t nanoSeconds = milliSeconds % 1000 * 1000 * 1000;

  if (eventInfoContainer[eventFd]->timerObject.setTimer(seconds, nanoSeconds))
    return 1;
  bool readyFlag = false;
  do {
    if (waitEpollEvent())
      return 1;

    if (epollEventTask(eventFd, readyFlag, timeoutFlag))
      return 1;
  } while (!readyFlag && !timeoutFlag);
  if (eraseFileDescriptor(
          eventInfoContainer[eventFd]->timerObject.getFileDescriptor()))
    return 1;

  if (eraseFileDescriptor(eventFd))
    return 1;
  return 0;
}

std::uint16_t EventWaitingInterface::finalize() {
  for (auto eventInfoIte = eventInfoContainer.begin();
       eventInfoIte != eventInfoContainer.end(); ++eventInfoIte) {
    if (eventInfoIte->second->eventObject.finalize())
      return 1;
    if (eventInfoIte->second->timerObject.finalize())
      return 1;
  }
  if (EpollInterfaceBase::finalize())
    return 1;
  return 0;
}

EventWaitingInterface::~EventWaitingInterface() {}
} // namespace MyUtility
