#ifndef NAPI_WRAPPER_H
#define NAPI_WRAPPER_H
#define NODE_ADDON_API_DISABLE_DEPRECATED
#undef ERROR

enum
{
  NM_F_BUILTIN = 1 << 0,
  NM_F_LINKED = 1 << 1,
  NM_F_INTERNAL = 1 << 2,
};

#define NODE_API_MODULE_LINKED( modname, regfunc )                \
  napi_value __napi_##regfunc( napi_env env, napi_value exports ) \
  {                                                               \
    return Napi::RegisterModule( env, exports, regfunc );         \
  }                                                               \
  NAPI_MODULE_X( modname, __napi_##regfunc, nullptr,              \
                 NM_F_LINKED )  // NOLINT (readability/null_usage)


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