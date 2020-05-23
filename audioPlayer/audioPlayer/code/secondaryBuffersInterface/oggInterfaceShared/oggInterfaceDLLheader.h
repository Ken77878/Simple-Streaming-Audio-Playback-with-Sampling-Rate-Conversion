#pragma once

#ifdef WIN32
#ifdef oggInterfaceShared_EXPORTS
#define oggInterface_API  __declspec(dllexport)
#else
#define oggInterface_API  __declspec(dllimport)
#endif
#else
#define oggInterface_API
#endif
