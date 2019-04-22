
#include "napi-wrap.h"

#include "../bscript/execmodl.h"
#include "../bscript/objmembers.h"
#include "../bscript/objmethods.h"
#include "../bscript/modmethods.h"


using namespace Pol;
using namespace Napi;

namespace Pol
{

namespace Node
{

Napi::Value GetModuleMethod( const Napi::CallbackInfo& cbinfo )
{
  Napi::Env env = cbinfo.Env();
  if ( cbinfo.Length() < 2 )
    return env.Undefined();

  auto modName = cbinfo[0].ToString().Utf8Value();
  auto funcName = cbinfo[1].ToString().Utf8Value();
  auto modFunc = Bscript::getKnownModuleMethod( modName.c_str(), funcName.c_str() );
  if (modFunc != nullptr)
  {
    EscapableHandleScope scope( env );
    auto obj = Object::New( env );
    obj["id"] = Number::New( env, modFunc->funcIdx );
    obj["argc"] = Number::New( env, modFunc->argc );
    return scope.Escape( obj );
  }
    return env.Undefined();
}

Napi::Value GetKnownObjMember( const Napi::CallbackInfo& cbinfo )
{
  Napi::Env env = cbinfo.Env();
  if ( cbinfo.Length() < 1 )
    return env.Undefined();

  Bscript::ObjMember* member =
      Bscript::getKnownObjMember( cbinfo[0].ToString().Utf8Value().c_str() );
  if ( member != nullptr )
  {
    EscapableHandleScope scope( env );
    auto obj = Object::New( env );
    obj["id"] = Number::New( env, member->id );
    obj["ro"] = Boolean::New( env, member->read_only );
    obj["name"] = String::New( env, member->code );
    return scope.Escape( obj );
  }
  return env.Undefined();
}

Napi::Value GetKnownObjMethod( const Napi::CallbackInfo& cbinfo )
{
  Napi::Env env = cbinfo.Env();
  if ( cbinfo.Length() < 1 )
    return env.Undefined();


  Bscript::ObjMethod* method =
      Bscript::getKnownObjMethod( cbinfo[0].ToString().Utf8Value().c_str() );
  if ( method != nullptr )
  {
    EscapableHandleScope scope( env );
    auto obj = Object::New( env );
    obj["id"] = Number::New( env, method->id );
    obj["name"] = String::New( env, method->code );
    return scope.Escape( obj );
  }
  return env.Undefined();
}

static Napi::Object InitializePolParser( Napi::Env env, Napi::Object exports )
{
  // NODELOG << "Received call from process._linkedbinding(). Exporting modules\n";
  exports["getKnownObjMember"] = Function::New( env, GetKnownObjMember );
  exports["getKnownObjMethod"] = Function::New( env, GetKnownObjMethod );
  exports["getKnownModuleMethod"] = Function::New( env, GetModuleMethod );


  return exports;
}

NODE_API_MODULE_LINKED( polparser, InitializePolParser )


}  // namespace Node
}  // namespace Pol
