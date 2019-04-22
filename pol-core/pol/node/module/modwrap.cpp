#include "../../module/attributemod.h"
#include "../../module/basicmod.h"
#include "../../module/basiciomod.h"
#include "../../module/boatmod.h"
#include "../../module/cfgmod.h"
#include "../../module/clmod.h"
#include "../../module/datastore.h"
#include "../../module/filemod.h"
#include "../../module/guildmod.h"
// #include "../../module/httpmod.h"
#include "../../module/mathmod.h"
#include "../../module/npcmod.h"
#include "../../module/osmod.h"
#include "../../module/partymod.h"
#include "../../module/polsystemmod.h"
#include "../../module/sqlmod.h"
#include "../../module/storagemod.h"
#include "../../module/unimod.h"
#include "../../module/uomod.h"
#include "../../module/utilmod.h"
#include "../../module/vitalmod.h"


#include "../../uoexec.h"
#include "modwrap.h"


using namespace Pol;
using namespace Napi;

namespace Pol
{


#define DECL_MODULE(MODULE)  \
namespace Module { class MODULE; } \
namespace Bscript { extern template class TmplExecutorModule<Module::MODULE>; }

DECL_MODULE(AttributeExecutorModule)
DECL_MODULE(BasicExecutorModule)
DECL_MODULE(BasicIoExecutorModule)
DECL_MODULE(UBoatExecutorModule)
DECL_MODULE(ConfigFileExecutorModule)
DECL_MODULE(ClilocExecutorModule)
DECL_MODULE(DataFileExecutorModule)
DECL_MODULE(FileAccessExecutorModule)
DECL_MODULE(GuildExecutorModule)
// DECL_MODULE(HttpExecutorModule)
DECL_MODULE(MathExecutorModule)
DECL_MODULE(NPCExecutorModule)
DECL_MODULE(PartyExecutorModule)
DECL_MODULE(PolSystemExecutorModule)
DECL_MODULE(SQLExecutorModule)
DECL_MODULE(StorageExecutorModule)
DECL_MODULE(UnicodeExecutorModule)
DECL_MODULE(UOExecutorModule)
DECL_MODULE(UtilExecutorModule)
DECL_MODULE(VitalExecutorModule)
#undef DECL_MODULE


namespace Node
{
static Napi::Object InitializeModwrap( Napi::Env env, Napi::Object exports )
{
  EscapableHandleScope scope( env );
  
#define INIT_MODULE(MODULE) Pol::Node::NodeModuleWrap<Module::MODULE>::Init( env, exports );

INIT_MODULE(AttributeExecutorModule)
INIT_MODULE(BasicExecutorModule)
INIT_MODULE(BasicIoExecutorModule)
INIT_MODULE(UBoatExecutorModule)
INIT_MODULE(ConfigFileExecutorModule)
INIT_MODULE(ClilocExecutorModule)
INIT_MODULE(DataFileExecutorModule)
INIT_MODULE(FileAccessExecutorModule)
INIT_MODULE(GuildExecutorModule)
// INIT_MODULE(HttpExecutorModule)
INIT_MODULE(MathExecutorModule)
INIT_MODULE(NPCExecutorModule)
INIT_MODULE(PartyExecutorModule)
INIT_MODULE(PolSystemExecutorModule)
INIT_MODULE(SQLExecutorModule)
INIT_MODULE(StorageExecutorModule)
INIT_MODULE(UnicodeExecutorModule)
INIT_MODULE(UOExecutorModule)
INIT_MODULE(UtilExecutorModule)
INIT_MODULE(VitalExecutorModule)
#undef INIT_MODULE

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
