#pragma once

#ifdef WIN32
#ifdef audioMessageQueueInterfaceShared_EXPORTS
#define audioMessageQueueInterface_API  __declspec(dllexport)
#else
#define audioMessageQueueInterface_API  __declspec(dllimport)
#endif
#else
#define audioMessageQueueInterface_API
#endif
