
#include "../../module/basiciomod.h"
#include "../../uoexec.h"
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
// namespace Core
//{
// class UOExecutor;
//}
namespace Node
{
Napi::FunctionReference NodeModuleWrap::constructor;


Napi::Value GetWrappedModule( const CallbackInfo& info )
{
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

  Napi::Function func =
      DefineClass( env, "NodeModuleWrap",
                   {
          //StaticAccessor( "basicio",
          //                          GetWrappedModule,                                 // getter
          //                          []( const CallbackInfo&, const Napi::Value& ) {}  // setter
          //                          )

                   } );

  constructor = Napi::Persistent( func );
  constructor.SuppressDestruct();
}
//
// NodeModuleWrap::NodeModuleWrap( const Napi::CallbackInfo& info, Object exports )
//   : Napi::ObjectWrap<NodeModuleWrap>(info)
NodeModuleWrap::NodeModuleWrap( const Napi::CallbackInfo& info )
    : Napi::ObjectWrap<NodeModuleWrap>( info )
{
  Napi::Env env = info.Env();
  Napi::HandleScope scope( env );

  size_t length = info.Length();

  if ( length <= 0 || !info[0].IsExternal() || !info[1].IsString() )
  {
    Napi::TypeError::New( env, "arg1=External, arg2=string expected" ).ThrowAsJavaScriptException();
  }

  auto modName = info[1].As<String>().Utf8Value();
  Core::UOExecutor* ex = info[0].As<External<Core::UOExecutor>>().Data();

  if ( modName == "basicio" )
  {
    auto x = new Module::BasicIoExecutorModule( *ex );
    for ( auto iter : x->function_table )
    {
      POLLOG_INFO << "Module has function " << iter.funcname << "\n";
    }
  }
}
}
}


static Napi::Object InitializeModwrap( Napi::Env env, Napi::Object exports )
{
  EscapableHandleScope scope( env );
  // Object modexports = Object::New( env );
  Pol::Node::NodeModuleWrap::Init( env, exports );

  for ( auto& mod : {"basicio", "basic"} )
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
