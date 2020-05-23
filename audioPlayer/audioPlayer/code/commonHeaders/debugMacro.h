#pragma once
#include <iostream>
#include <cstdio>
#include "characterCodeMacro.h"
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN             
#include <Windows.h>
#endif

#ifdef _DEBUG
#define DEBUG_PRINT(fmt)                                                       \
  do {                                                                         \
    tfprintf(stdout, fmt);                                                     \
  } while (0)
#else
#define DEBUG_PRINT(fmt)                                                       \
  do {                                                                         \
  } while (0)
#endif

#ifdef _DEBUG
#define DEBUG_PRINT_ARGS(fmt, ...)                                             \
  do {                                                                         \
    tfprintf(stdout, fmt, __VA_ARGS__);                                        \
  } while (0)
#else
#define DEBUG_PRINT_ARGS(fmt, ...)                                             \
  do {                                                                         \
  } while (0)
#endif

#ifdef _DEBUG
#define DEBUG_MSG(str)                                                         \
  do {                                                                         \
    std::tcout << str << std::endl;                                            \
  } while (false)
#else
#define DEBUG_MSG(str)                                                         \
  do {                                                                         \
  } while (false)
#endif
