#pragma once
#include <cstdint>
#include <pthread.h>
namespace LinuxApiWrapper {
std::uint16_t pthread_createWrap(pthread_t &thread, pthread_attr_t *attr,
                                 void *(*start_routine)(void *), void *arg);
std::uint16_t pthread_joinWrap(pthread_t thread, void **threadPtrReturnPtr);
} // namespace LinuxApiWrapper
