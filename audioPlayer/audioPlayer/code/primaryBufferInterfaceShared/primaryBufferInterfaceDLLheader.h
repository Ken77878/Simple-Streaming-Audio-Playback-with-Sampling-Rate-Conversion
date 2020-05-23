#pragma once

#ifdef WIN32
#ifdef primaryBufferInterfaceShared_EXPORTS
#define primaryBufferInterface_API  __declspec(dllexport)
#else
#define primaryBufferInterface_API  __declspec(dllimport)
#endif
#else
#define primaryBufferInterface_API
#endif
