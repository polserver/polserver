#ifndef _H_MODWRAP_INL
#define _H_MODWRAP_INL

#include "../../polsem.h"
#include "modwrap.h"
#include "objwrap.h"

using namespace Pol;
using namespace Napi;

namespace Pol
{
namespace Node
{
template <typename PolModule>
Napi::FunctionReference NodeModuleWrap<PolModule>::constructor;

/**
 * Initialize the NodeModuleWrap module.
 */
template <typename PolModule>
void NodeModuleWrap<PolModule>::Init( Napi::Env env, Napi::Object exports )
{
  std::string lowerModname( PolModule::modname );
  std::transform( lowerModname.begin(), lowerModname.end(), lowerModname.begin(),
                  []( char c ) { return static_cast<char>(::tolower( c ) ); } );
  NODELOG << "[modwrap] Initializing " << lowerModname << "\n";
  Napi::HandleScope scope( env );

  Napi::Function func = ObjectWrap<NodeModuleWrap<PolModule>>::DefineClass(
      env, lowerModname.c_str(),
      {ObjectWrap<NodeModuleWrap<PolModule>>::InstanceMethod(
          "execFunc", &NodeModuleWrap<PolModule>::ExecFunction )} );
  constructor = Napi::Persistent( func );
  constructor.SuppressDestruct();
  exports.Set( lowerModname, func );
};


template <typename PolModule>
Napi::Value NodeModuleWrap<PolModule>::ExecFunction( const CallbackInfo& cbinfo )
{
  Napi::Env env = cbinfo.Env();

  if ( cbinfo.Length() == 0 )
  {
    return env.Undefined();
  }
  int funcIdx = -1;

  if ( cbinfo[0].IsNumber() )
  {
    funcIdx = cbinfo[0].As<Number>().Int32Value();
  }
  else
  {
    auto funcName = cbinfo[0].ToString().Utf8Value();
    funcIdx = polmod->functionIndex( funcName );

    if ( funcIdx == -1 )
      Napi::TypeError::New( env,
                            std::string( "Unknown function " ) + PolModule::modname + funcName )
          .ThrowAsJavaScriptException();
  }

  Bscript::BObjectImp* funcRet;
  {
    for ( u32 i = 1; i < cbinfo.Length(); ++i )
    {
      Bscript::BObjectRef convertedVal = Node::NodeObjectWrap::Wrap( env, cbinfo[i] );
      polmod->exec.fparams.emplace_back( convertedVal );
    }
    funcRet = polmod->execFunc( funcIdx );
    // cleanParams is protected... sooo
    polmod->exec.fparams.clear();
  }

  auto convertedFunctRet =
      NodeObjectWrap::Wrap( env, Bscript::BObjectRef( funcRet ) );
  return convertedFunctRet;
}


template <typename PolModule>
NodeModuleWrap<PolModule>::NodeModuleWrap( const Napi::CallbackInfo& info )
    : Napi::ObjectWrap<NodeModuleWrap<PolModule>>( info ), uoexec( nullptr )
{
  Napi::Env env = info.Env();
  Napi::HandleScope scope( env );

  size_t length = info.Length();

  if ( length <= 0 || !info[0].IsExternal() )
  {
    Napi::TypeError::New( env, "arg1=External expected" ).ThrowAsJavaScriptException();
  }

  uoexec = info[0].As<External<Core::UOExecutor>>().Data();

  polmod = static_cast<PolModule*>( uoexec->findModule( PolModule::modname ) );

  if ( polmod == nullptr )
  {
    polmod = new PolModule( *uoexec );
    uoexec->addModule( polmod );
  }
}

}  // namespace Node
}  // namespace Pol


#endif