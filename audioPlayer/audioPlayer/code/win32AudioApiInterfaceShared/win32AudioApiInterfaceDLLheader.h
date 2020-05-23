#pragma once

#ifdef WIN32
#ifdef win32AudioApiInterfaceShared_EXPORTS
#define win32AudioApiInterface_API  __declspec(dllexport)
#else
#define win32AudioApiInterface_API  __declspec(dllimport)
#endif
#else
#define win32AudioApiInterface_API
#endif
