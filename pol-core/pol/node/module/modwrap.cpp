
#include "../bscript/impstr.h"
#include "../clib/logfacility.h"
#include "../clib/stlutil.h"
#include "../napi-wrap.h"
#include "../nodecall.h"
#include "modwrap.h"


using namespace Pol;
using namespace Napi;

namespace Pol
{
namespace Node
{
Napi::FunctionReference NodeModuleWrap::constructor;


Napi::Value NodeModuleWrap::Wrap( Napi::Env env, Bscript::BObjectRef objref, unsigned long reqId )
{
  EscapableHandleScope scope( env );
  Napi::Value convertedVal;

  /**
   * Lets do basic conversions here. Certain BObjectImps will be converted directly to
   * JavaScript primitives. Others will be wrapped in a ProxyObject.
   */

  auto impptr = objref.get()->impptr();

  if ( objref->isa( Bscript::BObjectImp::BObjectType::OTBoolean ) )
  {
    auto convt = Clib::explicit_cast<Bscript::BBoolean*, Bscript::BObjectImp*>( impptr );
    convertedVal = Napi::Boolean::New( env, convt->value() );
  }
  else if ( objref->isa( Bscript::BObjectImp::BObjectType::OTLong ) )
  {
    auto convt = Clib::explicit_cast<Bscript::BLong*, Bscript::BObjectImp*>( impptr );
    convertedVal = Napi::Number::New( env, convt->value() );
  }
  else if ( objref->isa( Bscript::BObjectImp::BObjectType::OTString ) )
  {
    auto convt = Clib::explicit_cast<Bscript::String*, Bscript::BObjectImp*>( impptr );
    convertedVal = Napi::String::New( env, convt->value() );
  }
  else if ( objref->isa( Bscript::BObjectImp::BObjectType::OTUninit ) )
  {
    convertedVal = env.Undefined();
  }
  else if ( objref->isa( Bscript::BObjectImp::BObjectType::OTMobileRef ) )
  {
    auto extVal = NodeModuleWrap::constructor.Call(
        {Napi::External<Bscript::BObject>::New( env, objref.get() )} );

    // extVal.As<NodeModuleWrap>().GetValue();

    requireRef.Get( "proxyObject" ).As<Function>().Call( {extVal} );
  }
  else
  {
    NODELOG.Format( "[{:04x}] [objwrap] error converting objref of type {}\n" )
        << reqId << impptr->typeOf();
    convertedVal = env.Undefined();
  }
  return scope.Escape( convertedVal );
}

Napi::Value GetWrappedModule( const CallbackInfo& info ) {
return Function::New( info.Env(), []( const CallbackInfo& cbinfo ) {} );
}
/**
 * Initialize the NodeModuleWrap module. It does not add any exports to
 * the JavaScript world (eg. no script should use `new NodeModuleWrap`).
 */
void NodeModuleWrap::Init( Napi::Env env, Napi::Object /*exports*/ )
{
  NODELOG << "[node] Initializing NodeModuleWrap\n";
  Napi::HandleScope scope( env );

  Napi::Function func = DefineClass( env, "NodeModuleWrap",
                                     {StaticAccessor( "basicio",
                                                      GetWrappedModule,                   // getter
                                                      []( const CallbackInfo&, const Napi::Value& ) {}  // setter
                                                      )} );

  constructor = Napi::Persistent( func );
  constructor.SuppressDestruct();
}
//
//NodeModuleWrap::NodeModuleWrap( const Napi::CallbackInfo& info, Object exports )
//   : Napi::ObjectWrap<NodeModuleWrap>(info)
NodeModuleWrap::NodeModuleWrap( const Napi::CallbackInfo& info )
    : Napi::ObjectWrap<NodeModuleWrap>( info )
  {
  Napi::Env env = info.Env();
  Napi::HandleScope scope( env );

  size_t length = info.Length();

  if ( length <= 0 || !info[0].IsExternal() )
  {
    Napi::TypeError::New( env, "External expected" ).ThrowAsJavaScriptException();
  }

  this->ref = RefType::New( info[0].As<ExtType>(), 1 );
  this->ref.Value().As<ExtType>().Data();
}
}
}



static Napi::Object InitializeModwrap( Napi::Env env, Napi::Object exports )
{
  EscapableHandleScope scope( env );
  //Object modexports = Object::New( env );
  
  for (auto& mod : { "basicio", "basic" })
  {
    exports.Set( mod, Function::New( env, []( const CallbackInfo& info ) {
                   for ( size_t i = 0; i < info.Length(); i++ )
                   {
                     POLLOG_INFO << Node::ToUtf8Value( info[i] ) << " ";
                   }
                   POLLOG_INFO << "\n";
                 } ) );
  }

  return exports;
}

NODE_API_MODULE_LINKED( modwrap, InitializeModwrap )
