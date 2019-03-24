#ifndef NODE_OBJWRAP_H
#define NODE_OBJWRAP_H

#include "../bscript/bobject.h"
#include "../napi-wrap.h"

namespace Pol
{
namespace Node
{
  using namespace Napi;

class NodeObjectWrap : public Napi::ObjectWrap<NodeObjectWrap>
{

public:
  using ExtType = External<Bscript::BObjectRef>;
  using RefType = Reference<ExtType>;

  NodeObjectWrap( const Napi::CallbackInfo& info );
  static void Init( Napi::Env env, Napi::Object exports );

private:
  static Napi::FunctionReference constructor;

  Napi::Value GetValue( const Napi::CallbackInfo& info );


  Reference<External<Bscript::BObjectRef>> ref;
};
}
}

#endif
