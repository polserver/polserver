#ifndef NODE_MODWRAP_H
#define NODE_MODWRAP_H

#include "../../../clib/logfacility.h"
#include "../bscript/bobject.h"
#include "../napi-wrap.h"
#include "../nodecall.h"


namespace Pol
{
namespace Bscript
{
template <typename Module>
class TmplExecutorModule;
}
namespace Module
{
class BasicIoExecutorModule;
}
namespace Node
{
using namespace Napi;

template <class T>
class NodeModuleWrap : public Napi::ObjectWrap<NodeModuleWrap<T>>
{
public:
  NodeModuleWrap( const Napi::CallbackInfo& info );
  Napi::Value MethodWrapper( const CallbackInfo& cbinfo );
  static void Init( Napi::Env env, Napi::Object exports );

private:
  T* polmod;
  weak_ptr<Core::UOExecutor> uoexec;
  static Napi::FunctionReference constructor;
};

}  // namespace Node
}  // namespace Pol

#include "modwrap-inl.h"

#endif
