#ifndef NODE_OBJWRAP_H
#define NODE_OBJWRAP_H

#include "../../bscript/bobject.h"
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

  static Napi::Object Init( Napi::Env env, Napi::Object exports );
  NodeObjectWrap( const Napi::CallbackInfo& info );

private:
  static Napi::FunctionReference constructor;

  Napi::Value GetValue( const Napi::CallbackInfo& info );


  Reference<External<Bscript::BObjectRef>> ref;
};
}
}

#endif
