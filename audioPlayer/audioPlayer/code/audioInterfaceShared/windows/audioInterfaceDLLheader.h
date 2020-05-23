#pragma once

#ifdef WIN32
#ifdef audioInterfaceShared_EXPORTS
#define audioInterface_API  __declspec(dllexport)
#else
#define audioInterface_API  __declspec(dllimport)
#endif
#else
#define audioInterface_API
#endif
