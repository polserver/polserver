#include "../../module/basiciomod.h"
#include "../../module/basicmod.h"
#include "../../module/uomod.h"
#include "../../uoexec.h"
#include "modwrap.h"


using namespace Pol;
using namespace Napi;

namespace Pol
{
namespace Node
{
}
}


static Napi::Object InitializeModwrap( Napi::Env env, Napi::Object exports )
{
  EscapableHandleScope scope( env );
  Pol::Node::NodeModuleWrap<Module::BasicIoExecutorModule>::Init( env, exports );
  Pol::Node::NodeModuleWrap<Module::BasicExecutorModule>::Init( env, exports );
  Pol::Node::NodeModuleWrap<Module::UOExecutorModule>::Init( env, exports );

  return exports;
}

NODE_API_MODULE_LINKED( modwrap, InitializeModwrap )
