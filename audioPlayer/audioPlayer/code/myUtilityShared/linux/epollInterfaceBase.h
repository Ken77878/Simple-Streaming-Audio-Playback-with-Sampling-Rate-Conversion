#pragma once

#include <cstdint>
#include <sys/epoll.h>
#include <vector>
#include "noCopyAndMoveTemplate.h"

namespace MyUtility {
class EpollInterfaceBase : public NoCopyAndMove<EpollInterfaceBase> {
protected:
  std::int32_t epollFd = 0;
  std::uint32_t maxEvents = 0;
  std::int32_t fdQuantity = 0;

  struct epoll_event epollEventObject;
  std::vector<struct epoll_event> availableEpollEventArray;

public:
  virtual std::uint16_t initialize();
  std::uint16_t registerFileDescriptor(std::int32_t fileDescriptor,
                                       uint32_t eventFlag = EPOLLIN,
                                       bool nonBlockingFlag = false);
  std::uint16_t registerFileDescriptor(std::int32_t fileDescriptor,
                                       struct epoll_event &epollEvent,
                                       bool nonBlockingFlag = false);
  std::uint16_t eraseFileDescriptor(std::int32_t fileDescriptor);
  bool checkEpollEventError(std::uint32_t epollEventsFlag) const;
  std::uint16_t waitEpollEvent(std::int32_t milliSec = -1);
  virtual std::uint16_t finalize();
  virtual ~EpollInterfaceBase();
};
} // namespace MyUtility
