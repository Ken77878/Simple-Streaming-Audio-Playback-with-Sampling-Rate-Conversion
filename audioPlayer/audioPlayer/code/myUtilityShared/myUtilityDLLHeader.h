#pragma once

#ifdef WIN32
#ifdef myUtilityShared_EXPORTS
#define myUtility_API  __declspec(dllexport)
#else
#define myUtility_API  __declspec(dllimport)
#endif
#else
#define myUtility_API
#endif
