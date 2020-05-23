#pragma once

#ifdef WIN32
#ifdef samplingRateConversionInterfaceShared_EXPORTS
#define samplingRateConversionInterface_API  __declspec(dllexport)
#else
#define samplingRateConversionInterface_API  __declspec(dllimport)
#endif
#else
#define samplingRateConversionInterface_API
#endif
