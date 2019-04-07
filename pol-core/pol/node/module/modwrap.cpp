
#include "../../module/basiciomod.h"
#include "../../module/basicmod.h"
//#include "../../../bscript/execmodl.h"
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
namespace Node
{
using PolBasicIoExecutorModule = Bscript::TmplExecutorModule<Module::BasicIoExecutorModule>;

}
}


static Napi::Object InitializeModwrap( Napi::Env env, Napi::Object exports )
{
  EscapableHandleScope scope( env );
  Pol::Node::NodeModuleWrap<Module::BasicIoExecutorModule>::Init( env, exports );
  Pol::Node::NodeModuleWrap<Module::BasicExecutorModule>::Init( env, exports );


  return exports;
}

NODE_API_MODULE_LINKED( modwrap, InitializeModwrap )
