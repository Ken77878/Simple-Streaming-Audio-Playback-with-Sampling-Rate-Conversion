#include "epollInterfaceBase.h"
#include "linuxApiWrapper/fileDescriptorApi.h"
#include "systemErrorInterface.h"
#include "fileDescriptorManagement.h"
// memset
#include <cstring>

// exit
#include <cstdlib>
#include <cerrno>
// printf
#include <cstdio>
#include "debugMacro.h"
#include <unistd.h>
namespace MyUtility {
std::uint16_t EpollInterfaceBase::initialize() {
  if ((epollFd = epoll_create1(0)) == -1) {
    SystemErrorInterface::systemErrorHandlingTask(TEXT("epoll_create1"));
    return 1;
  }
  return 0;
}

std::uint16_t EpollInterfaceBase::registerFileDescriptor(
    std::int32_t fileDescriptor, uint32_t eventFlag, bool nonBlockingFlag) {
  std::uint32_t epollTriggerFlag = 0;
  if (nonBlockingFlag) {
    if (setFileDescriptorNonblocking(fileDescriptor))
      return 1;
    epollTriggerFlag = EPOLLET;
  }
  std::memset(&epollEventObject, 0, sizeof(epoll_event));
  epollEventObject.events = eventFlag | epollTriggerFlag;
  epollEventObject.data.fd = fileDescriptor;

  if ((epoll_ctl(epollFd, EPOLL_CTL_ADD, fileDescriptor, &epollEventObject)) ==
      -1) {
    SystemErrorInterface::systemErrorHandlingTask(TEXT("epoll_ctl"));
    return 1;
  }
  ++maxEvents;
  availableEpollEventArray.resize(maxEvents);
  availableEpollEventArray.shrink_to_fit();

  return 0;
}

std::uint16_t
EpollInterfaceBase::registerFileDescriptor(std::int32_t fileDescriptor,
                                           struct epoll_event &epollEvent,
                                           bool nonBlockingFlag) {
  std::uint32_t epollTriggerFlag = 0;
  if (nonBlockingFlag) {
    if (setFileDescriptorNonblocking(fileDescriptor))
      return 1;
    epollTriggerFlag = EPOLLET;
    epollEvent.events = epollEvent.events | epollTriggerFlag;
  }

  if ((epoll_ctl(epollFd, EPOLL_CTL_ADD, fileDescriptor, &epollEvent)) == -1) {
    SystemErrorInterface::systemErrorHandlingTask(TEXT("epoll_ctl"));
    return 1;
  }
  ++maxEvents;
  availableEpollEventArray.resize(maxEvents);
  availableEpollEventArray.shrink_to_fit();

  return 0;
}

std::uint16_t
EpollInterfaceBase::eraseFileDescriptor(std::int32_t fileDescriptor) {
  if (epoll_ctl(epollFd, EPOLL_CTL_DEL, fileDescriptor, NULL) == -1) {
    SystemErrorInterface::systemErrorHandlingTask(TEXT("epoll_ctl"));
    return 1;
  }
  if (maxEvents)
    --maxEvents;
  availableEpollEventArray.resize(maxEvents);
  availableEpollEventArray.shrink_to_fit();
  return 0;
}

bool EpollInterfaceBase::checkEpollEventError(
    std::uint32_t epollEventsFlag) const {
  if (epollEventsFlag & (EPOLLERR | EPOLLHUP)) {
    if (epollEventsFlag & EPOLLERR) {
      SystemErrorInterface::systemErrorHandlingTask(TEXT("EPOLLERR"));
      DEBUG_PRINT("epoll: File descriptor error!!");

    } else if (epollEventsFlag & EPOLLHUP) {
      SystemErrorInterface::systemErrorHandlingTask(TEXT("EPOLLERR"));
      DEBUG_PRINT("epoll: File descriptor hangs up!!");
    }
    return true;
  }
  return false;
}
std::uint16_t EpollInterfaceBase::waitEpollEvent(std::int32_t milliSec) {
  // DEBUG_PRINT("waitEpollEvent was called\n");

  for (;;) {
    if ((fdQuantity = epoll_wait(epollFd, &availableEpollEventArray[0],
                                 maxEvents, milliSec)) == -1) {
      switch (errno) {
      case EINTR:
        // printf("EINTR: system call was interrupted by signal handler");
        continue;
        break;
      default:
        SystemErrorInterface::systemErrorHandlingTask(TEXT("epoll_wait"));
      }
      return 1;
    }
    break;
  }
  if (fdQuantity == 0) {
    printf("epoll timeout\n");
    return 1;
  }

  return 0;
}

std::uint16_t EpollInterfaceBase::finalize() {
  if (LinuxApiWrapper::closeWrap(epollFd)) {
    return 1;
  }
  return 0;
}

EpollInterfaceBase::~EpollInterfaceBase() {
#ifdef _DEBUG
  if (epollFd != 0) {
    std::exit(134);
  }
#endif
}
} // namespace MyUtility
