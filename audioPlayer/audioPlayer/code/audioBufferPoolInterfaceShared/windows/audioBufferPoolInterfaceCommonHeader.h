#pragma once

#ifdef WIN32
#ifdef audioBufferPoolInterfaceShared_EXPORTS
#define audioBufferPoolInterface_API  __declspec(dllexport)
#else
#define audioBufferPoolInterface_API  __declspec(dllimport)
#endif
#else
#define audioBufferPoolInterface_API
#endif
