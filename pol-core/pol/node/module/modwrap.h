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
// Bscript::TmplExecutorModule<Module::BasicIoExecutorModule>::FunctionTable tbl;

template <typename PolModule>
class NodeModuleWrap : public Napi::ObjectWrap<NodeModuleWrap<PolModule>>
{
public:
  NodeModuleWrap( const Napi::CallbackInfo& info );
  static void Init( Napi::Env env, Napi::Object exports );

  /**
   * Return a Value corresponding to this impptr
   */

private:
  PolModule* polmod;
  static Napi::FunctionReference constructor;
};

using BasicIoExecutorWrap = NodeModuleWrap<Bscript::TmplExecutorModule<Module::BasicIoExecutorModule>>;
}
}

#include "modwrap-inl.h"

#endif
