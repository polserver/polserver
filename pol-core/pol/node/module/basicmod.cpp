
#include "../clib/logfacility.h"
#include "../nodecall.h"
#include "basicmod.h"
#include "../napi-wrap.h"

using namespace Pol;
using namespace Napi;

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
  exports.Set( "print", Function::New( env, []( const CallbackInfo& info ) {
                 for ( size_t i = 0; i < info.Length(); i++ )
                 {
                   POLLOG_INFO << Node::ToUtf8Value( info[i] ) << " ";
                 }
                 POLLOG_INFO << "\n";
               } ) );
  return exports;
}

NODE_API_MODULE_LINKED( basic, InitializeBasic )
