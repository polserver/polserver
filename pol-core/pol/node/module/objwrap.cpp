
#include "objwrap.h"
#include "../../uoexec.h"
#include "../../uoscrobj.h"
#include "../bscript/impstr.h"
#include "../bscript/objmembers.h"
#include "../clib/logfacility.h"
#include "../clib/stlutil.h"
#include "../napi-wrap.h"
#include "../nodecall.h"

#define REFGUARD( X, Y )       \
  if ( X.IsEmpty() )        \
  {                         \
    return Y; \
  }


using namespace Pol;
using namespace Napi;

namespace Pol
{
namespace Node
{
Napi::FunctionReference NodeObjectWrap::constructor;
Napi::ObjectReference NodeObjectWrap::internalMethods;


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
    auto x = Napi::ObjectWrap<NodeObjectWrap>::Unwrap( value.ToObject() );
    REFGUARD( x->ref, Bscript::BObjectRef(new Bscript::UninitObject) )
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

std::map<u32, std::pair<Napi::ObjectReference, Napi::Promise::Deferred>> NodeObjectWrap::delayedMap;


bool NodeObjectWrap::resolveDelayedObject( u32 reqId, Bscript::BObjectRef objref,
                                           bool inNodeThread )
{
  auto iter = delayedMap.find( reqId );
  if ( iter == delayedMap.end() )
    return false;

  if ( inNodeThread )
  {
    auto& scriptModule = iter->second.first;
    auto& promise = iter->second.second;
    Napi::Env env = promise.Env();
    promise.Resolve( Wrap( env, objref, scriptModule.Value(), reqId ) );
    delayedMap.erase( iter );
    return true;
  }
  else
  {
    // We really only resolve.. so if something errors, it will be a resolution of an error, just
    // like in Escript. eg. `if (await Target(who)) {}` will resolve with a wrapped BError (ie. an
    // Napi::Error), whih is falsey.
    auto& scriptModule = iter->second.first;
    auto& promise = iter->second.second;
    auto call = Node::makeCall<bool>( [&]( Napi::Env env, NodeRequest<bool>* /*request*/ ) {
      promise.Resolve( Wrap( env, objref, scriptModule.Value(), reqId ) );
      delayedMap.erase( iter );
      return true;
    } );
    return call.getRef();
  }
}

void NodeObjectWrap::cleanup()
{
  // Release the ScriptModule reference and the reference to the underlying BObjectRef.
  //
  scriptModule.Reset();
  ref.Reset();
}

// FIXME Vulnerable to circular references.. for now!
Napi::Value NodeObjectWrap::Wrap( Napi::Env env, Bscript::BObjectRef objref,
                                  Napi::Object scriptModule, unsigned long reqId )
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
    delayedMap.emplace( convt->reqId(),
                        std::make_pair( ObjectReference::New( scriptModule, 1 ), deferred ) );
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
      arr[i] = Wrap( env, convt->ref_arr.at( i ), scriptModule, reqId );
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
      obj[key.first] = Wrap( env, key.second, scriptModule, reqId );
    }
  }
  else if ( impptr->isa( Bscript::BObjectImp::BObjectType::OTUninit ) )
  {
    convertedVal = env.Undefined();
  }
  else if ( impptr->isa( Bscript::BObjectImp::BObjectType::OTApplicObj ) )
  {
    auto convt = Clib::explicit_cast<Bscript::BApplicObjBase*, Bscript::BObjectImp*>( impptr );

    std::string clazzName = "";
    if ( convt->object_type() == &Module::echaracterrefobjimp_type )
    {
      Module::ECharacterRefObjImp* chrref_imp =
          Clib::explicit_cast<Module::ECharacterRefObjImp*, Bscript::BApplicObjBase*>( convt );

      auto chr = chrref_imp->value().get();

      if ( chr->isa( Core::UOBJ_CLASS::CLASS_NPC ) )
        clazzName = "NPC";

      else
        clazzName = "Character";
    }
    else if ( convt->object_type() == &Module::euboatrefobjimp_type )
    {
      clazzName = "Boat";
    }
    else if ( convt->object_type() == &Module::emultirefobjimp_type )
    {
      clazzName = "Multi";
    }
    else if ( convt->object_type() == &Module::eitemrefobjimp_type )
    {
      clazzName = "Item";
    }
    else
    {
      clazzName = "PolObject";
    }

    // Bscript::BObjectImp::BObjectType type = convt->type();

    // convt->script_i

    convertedVal =
        Node::requireRef.Get( "wrapper" )
            .As<Object>()
            .Get( "proxyObject" )
            .As<Function>()
            .Call( {Napi::String::New( env, clazzName ),

                    Napi::External<Bscript::BObjectRef>::New(
                        env, new Bscript::BObjectRef( impptr ),
                        []( Napi::Env /*env*/, Bscript::BObjectRef* data ) { delete data; } ),
                    internalMethods.Value(), scriptModule

            } );
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
void NodeObjectWrap::Init( Napi::Env env, Napi::Object exports )
{
  NODELOG << "[node] Initializing NodeObjectWrap\n";
  Napi::HandleScope scope( env );

  Napi::Function func = DefineClass(
      env, "NodeObjectWrap",
      {/*InstanceMethod( "get_member", &NodeObjectWrap::GetMember ),*/
       InstanceMethod( "addListenerExtUoExec", &NodeObjectWrap::AddListenerExtUoExec ),
       InstanceMethod( "removeListenerExtUoExec", &NodeObjectWrap::RemoveListenerExtUoExec ),
       InstanceMethod( "isTrue", &NodeObjectWrap::IsTrue ),
       InstanceMethod( "toString", &NodeObjectWrap::ToString )} );

  constructor = Napi::Persistent( func );
  constructor.SuppressDestruct();

  Napi::Object methods = Object::New( env );
  methods["set_member"] = Function::New( env, &NodeObjectWrap::SetMember );
  methods["get_member"] = Function::New( env, &NodeObjectWrap::GetMember );
  methods["call_method"] = Function::New( env, &NodeObjectWrap::CallMethod );
  internalMethods = Napi::Persistent( methods );
  internalMethods.SuppressDestruct();

  exports["PolObject"] = constructor.Value();
}

// returns a function that when called will run call_method, and
// whos ToPrimitive will be the value (get_member)


Napi::Value NodeObjectWrap::IsTrue( const CallbackInfo& info )
{
  Napi::Env env = info.Env();
  REFGUARD( ref, env.Undefined() )

  auto objref = *( ref.Value().Data() );
  auto impptr = objref->impptr();
  return Boolean::New( env, impptr->isTrue() );
}

Napi::Value NodeObjectWrap::AddListenerExtUoExec( const CallbackInfo& info )
{
  Napi::Env env = info.Env();
  REFGUARD( ref, env.Undefined() )

  Core::UOExecutor* exec = scriptModule.Get( "extUoExec" ).As<External<Core::UOExecutor>>().Data();
  auto objref = *( ref.Value().Data() );
  auto impptr = objref->impptr();
  auto convt = Clib::explicit_cast<Bscript::BApplicObjBase*, Bscript::BObjectImp*>( impptr );
  bool added = false;

  int mask = info[1].As<Number>().Int32Value();
  exec->eventmask |= mask;

  if ( info.Length() >= 2 && info[2].IsObject() )
  {
    auto options = info[2].As<Object>();
    if ( options.Get( "range" ).IsNumber() )
    {
      exec->speech_size = options.Get( "range" ).As<Number>().Int32Value();
    }
  }

  if ( convt->object_type() == &Module::echaracterrefobjimp_type )
  {
    auto imp = Clib::explicit_cast<Module::ECharacterRefObjImp*, Bscript::BApplicObjBase*>( convt );
    added = imp->value()->addListener( exec );
  }
  else if ( convt->object_type() == &Module::euboatrefobjimp_type )
  {
    auto imp = Clib::explicit_cast<Module::EUBoatRefObjImp*, Bscript::BApplicObjBase*>( convt );
    added = imp->value()->addListener( exec );
  }
  else if ( convt->object_type() == &Module::emultirefobjimp_type )
  {
    auto imp = Clib::explicit_cast<Module::EMultiRefObjImp*, Bscript::BApplicObjBase*>( convt );
    added = imp->value()->addListener( exec );
  }
  else if ( convt->object_type() == &Module::eitemrefobjimp_type )
  {
    auto imp = Clib::explicit_cast<Module::EItemRefObjImp*, Bscript::BApplicObjBase*>( convt );
    added = imp->value()->addListener( exec );
  }
  return Boolean::New( env, added );
}

Napi::Value NodeObjectWrap::RemoveListenerExtUoExec( const CallbackInfo& info )
{
  Napi::Env env = info.Env();
  REFGUARD( ref, env.Undefined() )

  Core::UOExecutor* exec = scriptModule.Get( "extUoExec" ).As<External<Core::UOExecutor>>().Data();
  auto objref = *( ref.Value().Data() );
  auto impptr = objref->impptr();
  auto convt = Clib::explicit_cast<Bscript::BApplicObjBase*, Bscript::BObjectImp*>( impptr );
  bool removed = false;

  if ( convt->object_type() == &Module::echaracterrefobjimp_type )
  {
    auto imp = Clib::explicit_cast<Module::ECharacterRefObjImp*, Bscript::BApplicObjBase*>( convt );
    removed = imp->value()->removeListener( exec );
  }
  else if ( convt->object_type() == &Module::euboatrefobjimp_type )
  {
    auto imp = Clib::explicit_cast<Module::EUBoatRefObjImp*, Bscript::BApplicObjBase*>( convt );
    removed = imp->value()->removeListener( exec );
  }
  else if ( convt->object_type() == &Module::emultirefobjimp_type )
  {
    auto imp = Clib::explicit_cast<Module::EMultiRefObjImp*, Bscript::BApplicObjBase*>( convt );
    removed = imp->value()->removeListener( exec );
  }
  else if ( convt->object_type() == &Module::eitemrefobjimp_type )
  {
    auto imp = Clib::explicit_cast<Module::EItemRefObjImp*, Bscript::BApplicObjBase*>( convt );
    removed = imp->value()->removeListener( exec );
  }
  return Boolean::New( env, removed );
}


Napi::Value NodeObjectWrap::SetMember( const CallbackInfo& info )
{
  Napi::Env env = info.Env();

  if ( info.Length() < 3 )
  {
    return Napi::Error( env, String::New( env, "Invalid parameters, expected argc = 3" ) ).Value();
  }
  if ( !info[0].As<Object>().InstanceOf( NodeObjectWrap::constructor.Value() ) )
  {
    return Napi::Error(
               env, String::New( env, "Invalid parameters, expected argv[0] = NodeObjectWrap" ) )
        .Value();
  }

  auto intObj = NodeObjectWrap::Unwrap( info[0].ToObject() );
  REFGUARD( intObj->ref, env.Undefined() )

  auto impptr = ( *intObj->ref.Value().Data() )->impptr();

  Bscript::BObjectRef retRef;

  if ( info[1].IsNumber() )
  {
    retRef = impptr->set_member_id( info[1].ToNumber().Int32Value(), Wrap( env, info[2] )->impptr(),
                                    true /*unused*/ );
  }
  else if ( info[1].IsString() )
  {
    retRef = impptr->set_member( info[1].ToString().Utf8Value().c_str(),
                                 Wrap( env, info[2] )->impptr(), true /*unused*/ );
  }
  else
  {
    return Napi::Error(
               env, String::New( env, "Invalid parameters, expected argv[0] = string | number" ) )
        .Value();
  }
  return Wrap( env, retRef, intObj->scriptModule.Value() );
}

Napi::Value NodeObjectWrap::GetMember( const CallbackInfo& info )
{
  Napi::Env env = info.Env();

  if ( info.Length() < 2 )
  {
    return Napi::Error( env, String::New( env, "Invalid parameters, expected argc = 2" ) ).Value();
  }
  if ( !info[0].As<Object>().InstanceOf( NodeObjectWrap::constructor.Value() ) )
  {
    return Napi::Error(
               env, String::New( env, "Invalid parameters, expected argv[0] = NodeObjectWrap" ) )
        .Value();
  }

  Napi::Object obj;
  auto intObj = NodeObjectWrap::Unwrap( info[0].ToObject() );
  REFGUARD( intObj->ref, env.Undefined() )


  auto impptr = ( *intObj->ref.Value().Data() )->impptr();

  Bscript::BObjectRef retRef;
  if ( info[1].IsNumber() )
  {
    retRef = impptr->get_member_id( info[1].ToNumber().Int32Value() );
  }
  else if ( info[1].IsString() )
  {
    retRef = impptr->get_member( info[1].ToString().Utf8Value().c_str() );
  }
  else
  {
    return Napi::Error(
               env, String::New( env, "Invalid parameters, expected argv[0] = string | number" ) )
        .Value();
  }

  return NodeObjectWrap::Wrap( env, retRef, intObj->scriptModule.Value() );
}


RefCountedExecutor* NodeObjectWrap::SharedInstance;
ref_ptr<RefCountedExecutor> NodeObjectWrap::SharedInstanceOwner;


Napi::Value NodeObjectWrap::CallMethod( const CallbackInfo& info )
{
  Napi::Env env = info.Env();

  if ( info.Length() < 2 )
  {
    return Napi::Error( env, String::New( env, "Invalid parameters, expected argc > 1" ) ).Value();
  }
  if ( !info[0].As<Object>().InstanceOf( NodeObjectWrap::constructor.Value() ) )
  {
    return Napi::Error(
               env, String::New( env, "Invalid parameters, expected argv[0] = NodeObjectWrap" ) )
        .Value();
  }

  Napi::Object obj;
  auto intObj = NodeObjectWrap::Unwrap( info[0].ToObject() );
  REFGUARD( intObj->ref, env.Undefined() )

  auto impptr = ( *intObj->ref.Value().Data() )->impptr();

  // arg0=object, arg1 = func id or string, arg2+ = method params

  for ( size_t i = 2; i < info.Length(); ++i )
  {
    SharedInstance->fparams.emplace_back( Wrap( env, info[i] ) );
  }

  Bscript::BObjectImp* retVal = nullptr;
  if ( info[1].IsNumber() )
  {
    retVal = impptr->call_method_id( info[1].ToNumber().Int32Value(), *SharedInstance );
  }
  else if ( info[1].IsString() )
  {
    retVal = impptr->call_method( info[1].ToString().Utf8Value().c_str(), *SharedInstance );
  }

  SharedInstance->fparams.clear();

  if ( retVal == nullptr )
  {
    return Napi::Error(
               env, String::New( env, "Invalid parameters, expected argv[0] = string | number" ) )
        .Value();
  }
  return Wrap( env, Bscript::BObjectRef( retVal ), intObj->scriptModule.Value() );
}

Napi::Value NodeObjectWrap::ToString( const CallbackInfo& info )
{
  Napi::Env env = info.Env();
  REFGUARD( ref, env.Undefined() )

  auto objref = *( ref.Value().Data() );
  return String::New( env, objref->impptr()->getStringRep() );
}

Napi::Value NodeObjectWrap::TypeOfInt( const CallbackInfo& info )
{
  Napi::Env env = info.Env();
  REFGUARD( ref, env.Undefined() )
  auto objref = *( ref.Value().Data() );
  return Number::New( env, objref->impptr()->typeOfInt() );
}

NodeObjectWrap::NodeObjectWrap( const Napi::CallbackInfo& info )
    : Napi::ObjectWrap<NodeObjectWrap>( info )
{
  Napi::Env env = info.Env();
  Napi::HandleScope scope( env );

  size_t length = info.Length();

  // arg0 = extobj, arg1 = internal methods, arg2 = main module
  if ( length <= 2 )
  {
    Napi::TypeError::New( env, "Error: argc >= 2" ).ThrowAsJavaScriptException();
  }
  else if ( !info[0].IsExternal() )
  {
    Napi::TypeError::New( env, "arg[0]=External expected" ).ThrowAsJavaScriptException();
  }
  else if ( !info[2].IsObject() )  // todo better hcecking for type
  {
    Napi::TypeError::New( env, "arg[2]=ScriptModule expected" ).ThrowAsJavaScriptException();
  }

  this->ref = Reference<External<Bscript::BObjectRef>>::New(
      info[0].As<External<Bscript::BObjectRef>>(), 1 );

  this->scriptModule = Napi::ObjectReference::New( info[2].As<Object>(), 1 );

  if ( !NodeObjectWrap::SharedInstance )
  {
    NodeObjectWrap::SharedInstance = new RefCountedExecutor;
    NodeObjectWrap::SharedInstanceOwner.set( NodeObjectWrap::SharedInstance );
  }
}
}  // namespace Node
}  // namespace Pol

#undef REFGUARD