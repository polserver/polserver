
#include "../../module/basiciomod.h"
#include "../../uoexec.h"
#include "../bscript/impstr.h"
#include "../clib/logfacility.h"
#include "../clib/stlutil.h"
#include "../napi-wrap.h"
#include "../nodecall.h"
#include "modwrap.h"


using namespace Pol;
using namespace Napi;

namespace Pol
{
// namespace Core
//{
// class UOExecutor;
//}
namespace Bscript
{
extern template class TmplExecutorModule<Module::BasicIoExecutorModule>;
}
namespace Node
{
using PolBasicIoExecutorModule = Bscript::TmplExecutorModule<Module::BasicIoExecutorModule>;

}
}


static Napi::Object InitializeModwrap( Napi::Env env, Napi::Object exports )
{
  EscapableHandleScope scope( env );
  // Object modexports = Object::New( env );
  Pol::Node::NodeModuleWrap<Module::BasicIoExecutorModule>::Init( env, exports );
  /*
    for ( auto& mod : {"basicio", "basic"} )
    {
      exports.Set( mod, Function::New( env, []( const CallbackInfo& info ) {
                     for ( size_t i = 0; i < info.Length(); i++ )
                     {
                       POLLOG_INFO << Node::ToUtf8Value( info[i] ) << " ";
                     }
                     POLLOG_INFO << "\n";
                   } ) ); } */
  // exports.Set( "modwrap",  )

  return exports;
}

NODE_API_MODULE_LINKED( modwrap, InitializeModwrap )
