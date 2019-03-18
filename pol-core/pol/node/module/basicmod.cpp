
#include "../../clib/logfacility.h"
#include "../nodethread.h"
#include "basicmod.h"
#include "napi.h"
namespace Pol
{
namespace Node
{
namespace Module
{
}
}
}


static Napi::Object InitializeBasic( Napi::Env env, Napi::Object exports )
{
  using namespace Pol;

  POLLOG_INFO << "initializing";
  exports.Set( "print", Function::New( env, []( CallbackInfo& info ) {
                 for ( int i = 0; i < info.Length(); i++ )
                 {
                   POLLOG_INFO << info[i]
                                      .As<Object>()
                                      .Get( "toString" )
                                      .As<Function>()
                                      .Call( info[i], {} )
                                      .As<String>()
                                      .Utf8Value();
                 }
                 POLLOG_INFO << "\n";
               } ) );
  POLLOG_INFO << "inited";
  return exports;
}

NODE_API_MODULE_LINKED( basic, InitializeBasic )
