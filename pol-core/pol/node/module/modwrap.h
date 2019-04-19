#ifndef NODE_MODWRAP_H
#define NODE_MODWRAP_H

#include "../../../clib/logfacility.h"
#include "../bscript/bobject.h"
#include "../napi-wrap.h"
#include "../nodecall.h"


namespace Pol
{
namespace Module
{
class BasicIoExecutorModule;
class BasicExecutorModule;
class UOExecutorModule;
class NPCExecutorModule;

}  // namespace Module
namespace Bscript
{
extern template class TmplExecutorModule<Module::BasicIoExecutorModule>;
extern template class TmplExecutorModule<Module::BasicExecutorModule>;
extern template class TmplExecutorModule<Module::UOExecutorModule>;
extern template class TmplExecutorModule<Module::NPCExecutorModule>;


}  // namespace Bscript

namespace Node
{
using namespace Napi;

template <class T>
class NodeModuleWrap : public Napi::ObjectWrap<NodeModuleWrap<T>>
{
public:
  NodeModuleWrap( const Napi::CallbackInfo& info );
  static void Init( Napi::Env env, Napi::Object exports );

  Napi::Value ExecFunction( const CallbackInfo& info );

private:
  T* polmod;
  Core::UOExecutor* uoexec;
  static Napi::FunctionReference constructor;
};

}  // namespace Node
}  // namespace Pol

#include "modwrap-inl.h"

#endif
