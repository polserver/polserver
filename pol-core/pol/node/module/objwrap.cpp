
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

Napi::Object NodeObjectWrap::Init( Napi::Env env, Napi::Object exports )
{
  Napi::HandleScope scope( env );

  Napi::Function func = DefineClass( env, "NodeObjectWrap",
                                     {InstanceMethod( "plusOne", &NodeObjectWrap::PlusOne ),
                                      InstanceMethod( "value", &NodeObjectWrap::GetValue ),
                                      InstanceMethod( "multiply", &NodeObjectWrap::Multiply )} );

  constructor = Napi::Persistent( func );
  constructor.SuppressDestruct();

  NODELOG << "We've set our NodeObjectWrapper constructor\n";
}

NodeObjectWrap::NodeObjectWrap( const Napi::CallbackInfo& info ) : Napi::ObjectWrap<NodeObjectWrap>( info )
{
  Napi::Env env = info.Env();
  Napi::HandleScope scope( env );

  int length = info.Length();

  if ( length <= 0 || !info[0].IsNumber() )
  {
    Napi::TypeError::New( env, "Number expected" ).ThrowAsJavaScriptException();
  }

  Napi::Number value = info[0].As<Napi::Number>();
  this->value_ = value.DoubleValue();
}

Napi::Value NodeObjectWrap::GetValue( const Napi::CallbackInfo& info )
{
  double num = this->value_;

  return Napi::Number::New( info.Env(), num );
}

Napi::Value NodeObjectWrap::PlusOne( const Napi::CallbackInfo& info )
{
  this->value_ = this->value_ + 1;

  return NodeObjectWrap::GetValue( info );
}

Napi::Value NodeObjectWrap::Multiply( const Napi::CallbackInfo& info )
{
  Napi::Number multiple;
  if ( info.Length() <= 0 || !info[0].IsNumber() )
  {
    multiple = Napi::Number::New( info.Env(), 1 );
  }
  else
  {
    multiple = info[0].As<Napi::Number>();
  }

  Napi::Object obj =
      constructor.New( {Napi::Number::New( info.Env(), this->value_ * multiple.DoubleValue() )} );

  return obj;
}
}
}
