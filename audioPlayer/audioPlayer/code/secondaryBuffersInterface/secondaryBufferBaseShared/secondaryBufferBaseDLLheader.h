#pragma once

#ifdef WIN32
#ifdef secondaryBufferBaseShared_EXPORTS
#define secondaryBufferBase_API  __declspec(dllexport)
#else
#define secondaryBufferBase_API  __declspec(dllimport)
#endif
#else
#define secondaryBufferBase_API
#endif
