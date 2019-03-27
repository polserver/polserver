#ifndef NODE_MODWRAP_H
#define NODE_MODWRAP_H

#include "../../../clib/logfacility.h"
#include "../nodecall.h"
#include "../bscript/bobject.h"
#include "../napi-wrap.h"


namespace Pol
{
namespace Node
{

using namespace Napi;

class NodeModuleWrap : public Napi::ObjectWrap<NodeModuleWrap>
{
public:
  using ExtType = External<Bscript::BObjectRef>;
  using RefType = Reference<ExtType>;

  NodeModuleWrap( const Napi::CallbackInfo& info );
  static void Init( Napi::Env env, Napi::Object exports );

  /**
   * Return a Value corresponding to this impptr
   */
  static Napi::Value Wrap( Napi::Env env, Bscript::BObjectRef objref, unsigned long reqId = 0 );

private:
  static Napi::FunctionReference constructor;

  void SetRef( Bscript::BObjectRef& ref );


  Reference<External<Bscript::BObjectRef>> ref;
};
}
}


#endif
