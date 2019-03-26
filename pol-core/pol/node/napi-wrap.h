#ifndef NAPI_WRAPPER_H
#define NAPI_WRAPPER_H
#define NODE_ADDON_API_DISABLE_DEPRECATED
#undef ERROR

#ifdef WINDOWS

#pragma warning( push )  // safes current warning settings
#pragma warning( disable : 4100 )
#include "napi.h"
#pragma warning( pop )  // restores old warnings

#else

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "napi.h"
#pragma GCC diagnostic pop
#endif

#endif