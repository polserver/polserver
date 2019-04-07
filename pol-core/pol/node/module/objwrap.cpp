
#include "objwrap.h"
#include "../bscript/impstr.h"
#include "../clib/stlutil.h"
#include "../clib/logfacility.h"
#include "../napi-wrap.h"
#include "../nodecall.h"


using namespace Pol;
using namespace Napi;

namespace Pol
{
namespace Node
{
Napi::FunctionReference NodeObjectWrap::constructor;


Bscript::BObjectImp* NodeObjectWrap::Wrap(Napi::Env /*env*/, Napi::Value value, unsigned long reqId )
{
  Bscript::BObjectImp* convertedVal;
  if (value.IsBoolean())
  {
    convertedVal = new Bscript::BBoolean( value.ToBoolean() );
  }
  else if (value.IsNumber())
  {
    convertedVal = new Bscript::BLong( value.ToNumber() );
  }
  else if (value.IsString())
  {
    convertedVal = new Bscript::String( value.ToString() );
  } 
  else if (value.IsUndefined())
  {
    convertedVal = new Bscript::UninitObject;
  } 
  else
  {
    NODELOG.Format( "[{:04x}] [objwrap] error converting napi value of type {} to bobjectimp\n" )
        << reqId << Node::ValueTypeToString( value.Type() );
    convertedVal = new Bscript::UninitObject;
  }
  return convertedVal;

}

// FIXME Vulnerable to circular references.. for now!
Napi::Value NodeObjectWrap::Wrap( Napi::Env env, Bscript::BObjectRef objref, unsigned long reqId )
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
  else if ( objref->isa( Bscript::BObjectImp::BObjectType::OTArray ) )
  {
    auto convt = Clib::explicit_cast<Bscript::ObjArray*, Bscript::BObjectImp*>( impptr );

    convertedVal = Napi::Array::New( env, convt->ref_arr.size() );
    auto arr = convertedVal.As<Array>();
    for ( auto i = 0; i < convt->ref_arr.size(); i++ )
    {
      arr[i] = Wrap( env, convt->ref_arr.at( i ), reqId );
    }
    //convertedVal = Napi::String::New( env, convt->value() );
  }
  else if ( objref->isa( Bscript::BObjectImp::BObjectType::OTUninit ) )
  {
    convertedVal = env.Undefined();
  }
  else if ( objref->isa( Bscript::BObjectImp::BObjectType::OTApplicObj ) )
  {
    auto convt = Clib::explicit_cast<Bscript::BApplicObjBase*, Bscript::BObjectImp*>( impptr );
    //convt->
    convertedVal = Napi::String::New( env, "hello there" );
    //auto extVal = NodeObjectWrap::constructor.Call(
    //    {Napi::External<Bscript::BObject>::New( env, objref.get() )} );

    //// extVal.As<NodeObjectWrap>().GetValue();

    //requireRef.Get( "proxyObject" ).As<Function>().Call( {extVal} );

  }
  else
  {
    NODELOG.Format( "[{:04x}] [objwrap] error converting objref of type {}\n" )
        << reqId << impptr->typeOf();
    convertedVal = env.Undefined();
  }
  return scope.Escape( convertedVal );
}
/**
 * Initialize the NodeObjectWrap module. It does not add any exports to
 * the JavaScript world (eg. no script should use `new NodeObjectWrap`).
 */
void NodeObjectWrap::Init( Napi::Env env, Napi::Object /*exports*/ )
{
  NODELOG << "[node] Initializing NodeObjectWrap\n";
  Napi::HandleScope scope( env );

  Napi::Function func = DefineClass( env, "NodeObjectWrap",
                                     {/* InstanceMethod( "value", &NodeObjectWrap::GetValue ) */} );

  constructor = Napi::Persistent( func );
  constructor.SuppressDestruct();
}

NodeObjectWrap::NodeObjectWrap( const Napi::CallbackInfo& info )
    : Napi::ObjectWrap<NodeObjectWrap>( info )
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
