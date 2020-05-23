#pragma once

#ifdef WIN32
#ifdef audioTestShared_EXPORTS
#define audioTest_API  extern "C" __declspec(dllexport)
#else
#define audioTest_API  extern "C" __declspec(dllimport)
#endif
#else
#define audioTest_API
#endif
