#ifndef NODE_OBJWRAP_H
#define NODE_OBJWRAP_H

#include "../../bscript/bobject.h"
#include "../napi-wrap.h"

namespace Pol
{
namespace Node
{

  class NodeObjectWrap : public Napi::ObjectWrap<NodeObjectWrap>
{
public:
  static Napi::Object Init( Napi::Env env, Napi::Object exports );
  NodeObjectWrap( const Napi::CallbackInfo& info );

private:
  static Napi::FunctionReference constructor;

  Napi::Value GetValue( const Napi::CallbackInfo& info );
  Napi::Value PlusOne( const Napi::CallbackInfo& info );
  Napi::Value Multiply( const Napi::CallbackInfo& info );

  double value_;
  Bscript::BObjectRef ref;

};
}
}

#endif
