#pragma once

#ifdef WIN32
#ifdef wavInterfaceShared_EXPORTS
#define wavInterface_API  __declspec(dllexport)
#else
#define wavInterface_API  __declspec(dllimport)
#endif
#else
#define wavInterface_API
#endif
