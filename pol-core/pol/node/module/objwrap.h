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

  /**
   * Return a Value corresponding to this impptr
   */
  static Napi::Value Wrap( Napi::Env env, Bscript::BObjectRef objref, unsigned long reqId = 0 );

    /**
   * Return a Value corresponding to this impptr
   */
  static Bscript::BObjectImp* Wrap( Napi::Env env, Napi::Value value, unsigned long reqId = 0);

private:
  static Napi::FunctionReference constructor;

  void SetRef( Bscript::BObjectRef& ref );


  Reference<External<Bscript::BObjectRef>> ref;
};

}
}

#endif
