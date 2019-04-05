#ifndef _H_MODWRAP_INL
#define _H_MODWRAP_INL

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
template <typename PolModule>
Napi::FunctionReference NodeModuleWrap<PolModule>::constructor;


/**
 * Initialize the NodeModuleWrap module. It does not add any exports to
 * the JavaScript world (eg. no script should use `new NodeModuleWrap`).
 */
template <typename PolModule>
void NodeModuleWrap<PolModule>::Init( Napi::Env env, Napi::Object exports )
{
  NODELOG << "[modwrap] Initializing " << PolModule::name << "\n";
  Napi::HandleScope scope( env );

  std::vector<Napi::ClassPropertyDescriptor<NodeModuleWrap<PolModule>>> props = {};
  for ( auto iter : PolModule::function_table )
  {
    NODELOG << "[modwrap] Module has function " << iter.funcname << "\n";
    props.emplace_back( ObjectWrap<NodeModuleWrap<PolModule>>::InstanceValue(
        iter.funcname.c_str(), String::New( env, iter.funcname ) ) );
  }

  Napi::Function func =
      ObjectWrap<NodeModuleWrap<PolModule>>::DefineClass( env, PolModule::name.c_str(), props );
  constructor = Napi::Persistent( func );
  constructor.SuppressDestruct();
  exports.Set( PolModule::name, func );
};

// extern Bscript::TmplExecutorModule<Module::BasicIoExecutorModule>::FunctionTable
//
// NodeModuleWrap::NodeModuleWrap( const Napi::CallbackInfo& info, Object exports )
//   : Napi::ObjectWrap<NodeModuleWrap>(info)
template <typename PolModule>
NodeModuleWrap<PolModule>::NodeModuleWrap( const Napi::CallbackInfo& info )
    : Napi::ObjectWrap<NodeModuleWrap<PolModule>>( info )
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
  polmod = new Module::BasicIoExecutorModule( *ex );
}
}
}


#endif