#ifndef NAPI_WRAPPER_H
#define NAPI_WRAPPER_H
#define NODE_ADDON_API_DISABLE_DEPRECATED
#undef ERROR
#pragma warning( push )  // safes current warning settings
#pragma warning( disable : 4100 )
#include "napi.h" 
#pragma warning( pop )  // restores old warnings


#endif