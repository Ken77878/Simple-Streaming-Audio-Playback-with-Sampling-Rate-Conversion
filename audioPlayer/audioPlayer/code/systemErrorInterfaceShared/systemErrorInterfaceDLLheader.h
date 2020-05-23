#pragma once
#ifdef WIN32
#ifdef systemErrorInterfaceShared_EXPORTS
#define systemErrorInterface_API __declspec(dllexport)
#else
#define systemErrorInterface_API __declspec(dllimport)
#endif
#else
#define systemErrorInterface_API
#endif