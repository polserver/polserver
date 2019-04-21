
#include "objwrap.h"
#include "../../uoexec.h"
#include "../bscript/impstr.h"
#include "../bscript/objmembers.h"
#include "../clib/logfacility.h"
#include "../clib/stlutil.h"
#include "../napi-wrap.h"
#include "../nodecall.h"

using namespace Pol;
using namespace Napi;

namespace Pol
{
namespace Node
{
Napi::FunctionReference NodeObjectWrap::constructor;


Bscript::BObjectRef NodeObjectWrap::Wrap( Napi::Env env, Napi::Value value, unsigned long reqId )
{
  Bscript::BObjectImp* convertedVal;
  if ( value.IsBoolean() )
  {
    convertedVal = new Bscript::BBoolean( value.ToBoolean() );
  }
  else if ( value.IsNumber() )
  {
    convertedVal = new Bscript::BLong( value.ToNumber() );
  }
  else if ( value.IsString() )
  {
    convertedVal = new Bscript::String( value.ToString() );
  }
  else if ( value.IsUndefined() )
  {
    convertedVal = new Bscript::UninitObject;
  }
  else if ( value.IsArray() )
  {
    auto jsArr = value.As<Array>();
    auto arr = new Bscript::ObjArray();
    for ( size_t i = 0; i < jsArr.Length(); ++i )
      arr->addElement( Wrap( env, jsArr[i], reqId )->impptr()->copy() );

    convertedVal = arr;
  }
  else if ( value.ToObject().InstanceOf( NodeObjectWrap::constructor.Value() ) )
  {
    auto x =
        Napi::ObjectWrap<NodeObjectWrap>::Unwrap( value.ToObject().Get( "_obj" ).As<Object>() );
    auto y = x->ref.Value();
    auto* z = y.Data();
    auto a = *z;
    return a;
  }
  else if ( value.IsObject() )
  {
    auto jsObj = value.As<Object>();
    auto obj = new Bscript::BStruct();
    auto propNames = jsObj.GetPropertyNames();
    for ( size_t i = 0; i < propNames.Length(); ++i )
    {
      auto propName = propNames.Get( i );
      NODELOG << "propname is " << propName.ToString().Utf8Value() << "\n";
      //  propName.As<String>().Utf8Value();
      obj->addMember( propName.ToString().Utf8Value().c_str(),
                      Wrap( env, jsObj.Get( propName ), reqId ) );
    }
    convertedVal = obj;
  }
  else
  {
    NODELOG.Format( "[{:04x}] [objwrap] error converting napi value of type {} to bobjectimp\n" )
        << reqId << Node::ValueTypeToString( value.Type() );
    convertedVal = new Bscript::UninitObject;
  }
  return Bscript::BObjectRef( convertedVal );
}

std::map<u32, Napi::Promise::Deferred> NodeObjectWrap::delayedMap;


bool NodeObjectWrap::resolveDelayedObject( u32 reqId, weak_ptr<Core::UOExecutor> uoexec,
                                           Bscript::BObjectRef objref )
{
  auto iter = delayedMap.find( reqId );
  if ( iter == delayedMap.end() )
    return false;

  // We really only resolve.. so if something errors, it will be a resolution of an error, just like
  // in Escript. eg. `if (await Target(who)) {}` will resolve with a wrapped BError (ie. an
  // Napi::Error), whih is falsey.
  auto& promise = iter->second;
  auto call = Node::makeCall<bool>( [&]( Napi::Env env, NodeRequest<bool>* /*request*/ ) {
    promise.Resolve( Wrap( env, uoexec, objref, reqId ) );
    delayedMap.erase( iter );
    return true;
  } );

  return call.getRef();
}

// FIXME Vulnerable to circular references.. for now!
Napi::Value NodeObjectWrap::Wrap( Napi::Env env, weak_ptr<Core::UOExecutor> uoexec,
                                  Bscript::BObjectRef objref, unsigned long reqId )
{
  EscapableHandleScope scope( env );
  Napi::Value convertedVal;

  /**
   * Lets do basic conversions here. Certain BObjectImps will be converted directly to
   * JavaScript primitives. Others will be wrapped in a ProxyObject.
   */

  auto impptr = objref.get()->impptr();

  if ( impptr->isa( Bscript::BObjectImp::BObjectType::OTBoolean ) )
  {
    auto convt = Clib::explicit_cast<Bscript::BBoolean*, Bscript::BObjectImp*>( impptr );
    convertedVal = Napi::Boolean::New( env, convt->value() );
  }
  else if ( impptr->isa( Bscript::BObjectImp::BObjectType::OTLong ) )
  {
    auto convt = Clib::explicit_cast<Bscript::BLong*, Bscript::BObjectImp*>( impptr );
    convertedVal = Napi::Number::New( env, convt->value() );
  }
  else if ( impptr->isa( Bscript::BObjectImp::BObjectType::OTString ) )
  {
    auto convt = Clib::explicit_cast<Bscript::String*, Bscript::BObjectImp*>( impptr );
    convertedVal = Napi::String::New( env, convt->value() );
  }
  else if ( impptr->isa( Bscript::BObjectImp::BObjectType::OTError ) )
  {
    auto convt = Clib::explicit_cast<Bscript::BError*, Bscript::BObjectImp*>( impptr );
    convertedVal = Napi::Error::New(
                       env, String::New( env, convt->FindMember( "errortext" )->getStringRep() ) )
                       .Value();
  }
  else if ( impptr->isa( Bscript::BObjectImp::BObjectType::OTDelayedObject ) )
  {
    auto convt = Clib::explicit_cast<Bscript::DelayedObject*, Bscript::BObjectImp*>( impptr );
    // Create a new deferred, and return the promise to the script.
    // We store the deferred in our map, to be resolved later.
    Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New( env );
    delayedMap.emplace( convt->reqId(), deferred );
    convertedVal = deferred.Promise();
  }
  else if ( impptr->isa( Bscript::BObjectImp::BObjectType::OTArray ) )
  {
    auto convt = Clib::explicit_cast<Bscript::ObjArray*, Bscript::BObjectImp*>( impptr );

    // Create a new array of proper size
    convertedVal = Napi::Array::New( env, convt->ref_arr.size() );
    auto arr = convertedVal.As<Array>();
    for ( size_t i = 0; i < convt->ref_arr.size(); i++ )
    {
      // Set the value
      arr[i] = Wrap( env, uoexec, convt->ref_arr.at( i ), reqId );
    }
  }
  else if ( impptr->isa( Bscript::BObjectImp::BObjectType::OTStruct ) )
  {
    auto convt = Clib::explicit_cast<Bscript::BStruct*, Bscript::BObjectImp*>( impptr );

    // Create a new object
    convertedVal = Napi::Object::New( env );
    auto obj = convertedVal.As<Object>();
    for ( auto key : convt->contents() )
    {
      obj[key.first] = Wrap( env, uoexec, key.second, reqId );
    }
  }
  else if ( impptr->isa( Bscript::BObjectImp::BObjectType::OTUninit ) )
  {
    convertedVal = env.Undefined();
  }
  else if ( impptr->isa( Bscript::BObjectImp::BObjectType::OTApplicObj ) )
  {
    convertedVal =
        Node::requireRef.Get( "wrapper" )
            .As<Object>()
            .Get( "proxyObject" )
            .As<Function>()
            .Call( {

                NodeObjectWrap::constructor.New(
                    {Napi::External<Bscript::BObjectRef>::New(
                         env, new Bscript::BObjectRef( impptr ),
                         []( Napi::Env /*env*/, Bscript::BObjectRef* data ) { delete data; } ),

                     Napi::External<weak_ptr<Core::UOExecutor>>::New(
                         env, new weak_ptr<Core::UOExecutor>( uoexec ) )} )} );
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

  Napi::Function func =
      DefineClass( env, "NodeObjectWrap",
                   {InstanceMethod( "getMember", &NodeObjectWrap::GetMember ),
                    InstanceMethod( "getMethodFunction", &NodeObjectWrap::GetMethodFunction ),
                    InstanceMethod( "toString", &NodeObjectWrap::ToString )} );

  constructor = Napi::Persistent( func );
  constructor.SuppressDestruct();
}

// returns a function that when called will run call_method, and
// whos ToPrimitive will be the value (get_member)

Napi::Value NodeObjectWrap::GetMember( const CallbackInfo& info )
{
  Napi::Env env = info.Env();
  auto objref = *( ref.Value().Data() );
  if ( info.Length() < 1 || !info[0].IsString() )
  {
    return Napi::Error( env, String::New( env, "Invalid parameters" ) ).Value();
  }

  //   objref->impptr()->get_member_id();
  auto membername = info[0].As<String>().Utf8Value();
  auto impptr = objref->impptr();
  auto ref = impptr->get_member( membername.c_str() );
  NODELOG << "membername is " << membername << " getmember " << ref->impptr()->getStringRep()
          << "\n";
  return Wrap( env, uoexec, ref );
}


Napi::Value NodeObjectWrap::GetMethodFunction( const CallbackInfo& info )
{
  Napi::Env env = info.Env();
  auto objref = *( ref.Value().Data() );
  if ( info.Length() < 1 || !info[0].IsString() )
  {
    return Napi::Error( env, String::New( env, "Invalid parameters" ) ).Value();
  }

  //   objref->impptr()->get_member_id();
  auto membername = info[0].As<String>().Utf8Value();
  auto impptr = objref->impptr();

  Napi::Function func = Napi::Function::New(
      env,
      [this, membername, impptr]( const CallbackInfo& cbinfo ) {
        Napi::Env env = cbinfo.Env();
        for ( size_t i = 0; i < cbinfo.Length(); ++i )
        {
          uoexec->fparams.emplace_back( Wrap( env, cbinfo[i] ) );
        }
        Bscript::BObjectImp* retVal = impptr->call_method( membername.c_str(), * (this->uoexec.get_weakptr()) );
        uoexec->fparams.clear();
        return Wrap( env, uoexec, Bscript::BObjectRef( retVal ) );
      },
      membername );

  return func;


  /*func.DefineProperty( Napi::PropertyDescriptor::Value(
      env.Global().Get( "Symbol" ).As<Object>().Get( "toPrimitive" ),
      Function::New( env, []( const CallbackInfo& cbinfo ) {
        auto ref = impptr->get_member( membername.c_str() );
        NODELOG << "membername is " << membername << " getmember " << ref->impptr()->getStringRep()
                << "\n";
        return Napi::String::New( cbinfo.Env(), ref->impptr()->getStringRep() );
      } ) ) );
  return func;*/

  // impptr->get_member impptr->call_method( membername, );
}

Napi::Value NodeObjectWrap::ToString( const CallbackInfo& info )
{
  Napi::Env env = info.Env();
  auto objref = *( ref.Value().Data() );
  return String::New( env, objref->impptr()->getStringRep() );
}

Napi::Value NodeObjectWrap::TypeOfInt( const CallbackInfo& info )
{
  Napi::Env env = info.Env();
  auto objref = *( ref.Value().Data() );
  return Number::New( env, objref->impptr()->typeOfInt() );
}

NodeObjectWrap::NodeObjectWrap( const Napi::CallbackInfo& info )
    : Napi::ObjectWrap<NodeObjectWrap>( info ), uoexec( nullptr )
{
  Napi::Env env = info.Env();
  Napi::HandleScope scope( env );

  size_t length = info.Length();

  if ( length <= 0 || !info[0].IsExternal() )
  {
    Napi::TypeError::New( env, "External expected" ).ThrowAsJavaScriptException();
  }

  this->ref = Reference<External<Bscript::BObjectRef>>::New(
      info[0].As<External<Bscript::BObjectRef>>(), 1 );
  auto weakptr = *( info[1].As<External<weak_ptr<Core::UOExecutor>>>().Data() );
  this->uoexec = weakptr;
}
}  // namespace Node
}  // namespace Pol
