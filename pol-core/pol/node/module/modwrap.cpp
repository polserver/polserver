#include "../../module/attributemod.h"
#include "../../module/basiciomod.h"
#include "../../module/basicmod.h"
#include "../../module/npcmod.h"
#include "../../module/uomod.h"
#include "../../module/vitalmod.h"
#include "../../module/unimod.h"


#include "../../uoexec.h"
#include "modwrap.h"


using namespace Pol;
using namespace Napi;

namespace Pol
{
namespace Node
{
static Napi::Object InitializeModwrap( Napi::Env env, Napi::Object exports )
{
  EscapableHandleScope scope( env );
  Pol::Node::NodeModuleWrap<Module::BasicIoExecutorModule>::Init( env, exports );
  Pol::Node::NodeModuleWrap<Module::BasicExecutorModule>::Init( env, exports );
  Pol::Node::NodeModuleWrap<Module::UOExecutorModule>::Init( env, exports );
  Pol::Node::NodeModuleWrap<Module::NPCExecutorModule>::Init( env, exports );
  Pol::Node::NodeModuleWrap<Module::AttributeExecutorModule>::Init( env, exports );
  Pol::Node::NodeModuleWrap<Module::VitalExecutorModule>::Init( env, exports );
  Pol::Node::NodeModuleWrap<Module::UnicodeExecutorModule>::Init( env, exports );

  return exports;
}

template <>
NodeModuleWrap<Module::NPCExecutorModule>::NodeModuleWrap( const Napi::CallbackInfo& info )
    : Napi::ObjectWrap<NodeModuleWrap<Module::NPCExecutorModule>>( info ), uoexec( nullptr )
{
  Napi::Env env = info.Env();
  Napi::HandleScope scope( env );

  size_t length = info.Length();

  if ( length <= 0 || !info[0].IsExternal() )
  {
    Napi::TypeError::New( env, "arg1=External expected" ).ThrowAsJavaScriptException();
  }

  uoexec = info[0].As<External<Core::UOExecutor>>().Data();

  polmod = static_cast<Module::NPCExecutorModule*>(
      uoexec->findModule( Module::NPCExecutorModule::modname ) );

  if ( polmod == nullptr )
  {
    Napi::TypeError::New( env, "Cannot wrap non-existent NPC module" ).ThrowAsJavaScriptException();
  }
}

NODE_API_MODULE_LINKED( modwrap, InitializeModwrap )
}  // namespace Node
}  // namespace Pol
