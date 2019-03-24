
#include "../clib/logfacility.h"
#include "../napi-wrap.h"
#include "../nodecall.h"
#include "objwrap.h"


using namespace Pol;
using namespace Napi;

namespace Pol
{
namespace Node
{
Napi::FunctionReference NodeObjectWrap::constructor;

/**
 * Initialize the NodeObjectWrap module. It does not add any exports to
 * the JavaScript world (eg. no script should use `new NodeObjectWrap`).
 */
void NodeObjectWrap::Init( Napi::Env env, Napi::Object exports )
{
  NODELOG << "[node] Initializing NodeObjectWrap\n";
  Napi::HandleScope scope( env );

  Napi::Function func = DefineClass( env, "NodeObjectWrap",
                                     {
                                      InstanceMethod( "value", &NodeObjectWrap::GetValue )
                                      } );

  constructor = Napi::Persistent( func );
  constructor.SuppressDestruct();
}

NodeObjectWrap::NodeObjectWrap( const Napi::CallbackInfo& info )
    : Napi::ObjectWrap<NodeObjectWrap>( info )
{
  Napi::Env env = info.Env();
  Napi::HandleScope scope( env );

  int length = info.Length();

  if ( length <= 0 || !info[0].IsExternal() )
  {
    Napi::TypeError::New( env, "External expected" ).ThrowAsJavaScriptException();
  }

  this->ref = RefType::New( info[0].As<ExtType>(), 1 );
  this->ref.Value().As<ExtType>().Data();
}

Napi::Value NodeObjectWrap::GetValue( const Napi::CallbackInfo& info )
{
  //double num = this->value_;

  return Napi::Number::New( info.Env(), clock() );
}

}
}
