#ifndef NODE_OBJWRAP_H
#define NODE_OBJWRAP_H

#include "../bscript/berror.h"
#include "../bscript/bobject.h"
#include "../clib/weakptr.h"
#include "../napi-wrap.h"

namespace Pol
{
namespace Core
{
class UOExecutor;
}
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
  static Napi::Value Wrap( Napi::Env env, weak_ptr<Core::UOExecutor> uoexec,
                           Bscript::BObjectRef objref, unsigned long reqId = 0 );

  /**
   * Return a BObjectImp* corresponding to this Napi Value
   */
  static Bscript::BObjectRef Wrap( Napi::Env env, Napi::Value value, unsigned long reqId = 0 );


  Napi::Value ToString( const CallbackInfo& cbinfo );
  Napi::Value TypeOfInt( const CallbackInfo& cbinfo );


  Napi::Value GetMember( const CallbackInfo& cbinfo );
  Napi::Value GetMethodFunction( const CallbackInfo& info );


  Napi::Value SetMember( const CallbackInfo& cbinfo );

  /** Can run outside Node env */
  static bool resolveDelayedObject( u32 reqId, weak_ptr<Core::UOExecutor> uoexec, Bscript::BObjectRef objref );

private:
  static Napi::FunctionReference constructor;
  static std::map<u32, Napi::Promise::Deferred> delayedMap;


  Reference<External<Bscript::BObjectRef>> ref;
  weak_ptr<Core::UOExecutor> uoexec;
};

}  // namespace Node
}  // namespace Pol

#endif
