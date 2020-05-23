#pragma once

#ifdef WIN32
#ifdef win32ApiWrapperShared_EXPORTS
#define win32ApiWrapper_API  __declspec(dllexport)
#else
#define win32ApiWrapper_API  __declspec(dllimport)
#endif
#else
#define win32ApiWrapper_API
#endif
