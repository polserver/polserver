/** @file
 *
 * @par History
 */


#ifndef NODETHREAD_H
#define NODETHREAD_H

#include "../bscript/eprog.h"
#include "napi-wrap.h"
#include <condition_variable>
#include <future>

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


namespace Pol
{
namespace Node
{
extern Napi::ObjectReference requireRef;
extern Napi::ThreadSafeFunction tsfn;

std::future<bool> start_node();

extern std::atomic<bool> running;

void cleanup();

void RegisterBuiltinModules();

// void start_node_thread();
}  // namespace Node
}  // namespace Pol

#endif
