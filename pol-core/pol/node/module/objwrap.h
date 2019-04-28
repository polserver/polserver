#ifndef NODE_OBJWRAP_H
#define NODE_OBJWRAP_H

#include "../bscript/berror.h"
#include "../bscript/bobject.h"
#include "../bscript/executor.h"
#include "../clib/weakptr.h"
#include "../napi-wrap.h"

namespace Pol
{
namespace Core
{
// class UOExecutor;
}
namespace Node
{
using namespace Napi;

class RefCountedExecutor : public Bscript::Executor, public ref_counted
{
public:
  inline RefCountedExecutor() : Executor(), ref_counted() {}
};
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
  static Napi::Value Wrap( Napi::Env env, Bscript::BObjectRef objref, Napi::Object scriptModule,
                           unsigned long reqId = 0 );

  /**
   * Return a BObjectImp* corresponding to this Napi Value
   */
  static Bscript::BObjectRef Wrap( Napi::Env env, Napi::Value value, unsigned long reqId = 0 );


  Napi::Value ToString( const CallbackInfo& cbinfo );
  Napi::Value TypeOfInt( const CallbackInfo& cbinfo );
  Napi::Value IsTrue( const CallbackInfo& cbinfo );

  static Napi::Value SetMember( const CallbackInfo& cbinfo );
  static Napi::Value GetMember( const CallbackInfo& cbinfo );
  static Napi::Value CallMethod( const CallbackInfo& info );

  /** Can run outside Node env */
  static bool resolveDelayedObject( u32 reqId, Bscript::BObjectRef objref, bool inNodeThread = false);

  Reference<External<Bscript::BObjectRef>> ref;

  static void ReleaseSharedInstance()
  {
    SharedInstanceOwner.clear();
    SharedInstance = nullptr;
  }

private:
  // the script module where this object was created
  Napi::ObjectReference scriptModule;

  static Napi::FunctionReference constructor;
  static Napi::ObjectReference internalMethods;
  static std::map<u32, std::pair<Napi::ObjectReference, Napi::Promise::Deferred>> delayedMap;

  // We share a single executor for running functions.
  // Otherwise, we could pass the uoexec from creation and store it.
  // static Bscript::Executor methodExecutor;
  // static std::unique_ptr<Bscript::Executor> SharedExecutorInstance;
  static RefCountedExecutor* SharedInstance;
  static ref_ptr<RefCountedExecutor> SharedInstanceOwner;
};

}  // namespace Node
}  // namespace Pol

#endif
