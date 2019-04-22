
#include "napi-wrap.h"

#include "../bscript/execmodl.h"
#include "../bscript/objmembers.h"
#include "../bscript/objmethods.h"

using namespace Pol;
using namespace Napi;

namespace Pol
{
namespace Bscript
{
//using namespace Module;
//
//extern template TmplExecutorModule<BasicIoExecutorModule>;
}  // namespace Bscript

namespace Node
{
template <typename PolModule>
int GetModuleMethodTmpl( const std::string& name )
{
  std::cout << "funcmapinit: " << PolModule::function_table.size() << PolModule::_func_map_init
            << std::endl;

  if ( !PolModule::_func_map_init )
  {
    for ( unsigned idx = 0; idx < PolModule::function_table.size(); idx++ )
    {
      std::cout << "func table " << PolModule::function_table[idx].funcname << "\n";
      PolModule::_func_idx_map[PolModule::function_table[idx].funcname] = idx;
    }
    PolModule::_func_map_init = true;
  }


  auto itr = PolModule::_func_idx_map.find( name );
  auto index = itr != PolModule::_func_idx_map.end() ? itr->second : -1;
  return index;
}

Napi::Value GetModuleMethod( const Napi::CallbackInfo& cbinfo )
{
  Napi::Env env = cbinfo.Env();
  if ( cbinfo.Length() < 2 )
    return env.Undefined();

  auto modName = cbinfo[0].ToString().Utf8Value();
  auto funcName = cbinfo[1].ToString().Utf8Value();
  int funcIdx = -2;

  //  GetModuleMethodTmpl<Module::BasicIoExecutorModule>( funcName );
  std::string lowerModname;


#define ABC( V )                                                                  \
  lowerModname = Module::##V## ::modname;                                         \
  std::transform( lowerModname.begin(), lowerModname.end(), lowerModname.begin(), \
                  []( char c ) { return static_cast<char>(::tolower( c ) ); } );  \
  if ( modName == lowerModname )                                                  \
  {                                                                               \
    funcIdx = GetModuleMethodTmpl<Module::##V##>( funcName );                     \
  }

  //ABC( AttributeExecutorModule )
  //ABC( BasicIoExecutorModule )
  //ABC( BasicExecutorModule )
  //ABC( UBoatExecutorModule )
  //ABC( ConfigFileExecutorModule )
  //ABC( ClilocExecutorModule )
  //ABC( DataFileExecutorModule )
  //ABC( FileAccessExecutorModule )
  //ABC( GuildExecutorModule )
  //ABC( HttpExecutorModule )
  //ABC( MathExecutorModule )
  //ABC( NPCExecutorModule )
  //ABC( PartyExecutorModule )
  //ABC( PolSystemExecutorModule )
  //ABC( SQLExecutorModule )
  //ABC( StorageExecutorModule )
  //ABC( UnicodeExecutorModule )
  //ABC( UOExecutorModule )
  //ABC( UtilExecutorModule )
  //ABC( VitalExecutorModule )


  return funcIdx == -2 ? env.Undefined() : Number::New( env, funcIdx );
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
  exports["getModuleMethod"] = Function::New( env, GetModuleMethod );


  return exports;
}

NODE_API_MODULE_LINKED( polparser, InitializePolParser )


}  // namespace Node
}  // namespace Pol
