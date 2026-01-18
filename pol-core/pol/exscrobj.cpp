/** @file
 *
 * @par History
 * - 2005/10/02 Shinigami: added Prop Script.attached_to and Script.controller
 * - 2005/11/25 Shinigami: MTH_GET_MEMBER/"get_member" - GCC fix
 * - 2005/11/26 Shinigami: changed "strcmp" into "stricmp" to suppress Script Errors
 * - 2006/01/18 Shinigami: added attached_npc_ - to get attached NPC from AI-Script-Process Obj
 * - 2006/09/17 Shinigami: Script.sendevent() will return error "Event queue is full, discarding
 * event"
 * - 2006/09/23 Shinigami: Script_Cycles, Sleep_Cycles and Script_passes uses 64bit now
 * - 2009/12/21 Turley:    ._method() call fix
 */


#include "exscrobj.h"

#include <stddef.h>
#include <string>
#include <utility>

#include "bscript/berror.h"
#include "bscript/bobject.h"
#include "bscript/dict.h"
#include "bscript/executor.h"
#include "bscript/impstr.h"
#include "bscript/objmembers.h"
#include "bscript/objmethods.h"
#include "clib/strutil.h"
#include "module/polsystemmod.h"
#include "module/uomod.h"
#include "plib/systemstate.h"
#include "scrsched.h"
#include "uoexec.h"
#include "uoscrobj.h"


namespace Pol::Core
{
using namespace Bscript;

BApplicObjType scriptexobjimp_type;

ScriptExObjImp::ScriptExObjImp( UOExecutor* uoexec )
    : PolApplicObj<ScriptExPtr>( &scriptexobjimp_type, uoexec->weakptr )
{
}

const char* ScriptExObjImp::typeOf() const
{
  return "ScriptExRef";
}
u8 ScriptExObjImp::typeOfInt() const
{
  return OTScriptExRef;
}

BObjectImp* ScriptExObjImp::copy() const
{
  if ( value().exists() )
    return new ScriptExObjImp( value().get_weakptr() );
  return new BError( "Script has been destroyed" );
}


BObjectImp* ScriptExObjImp::call_polmethod_id( const int id, UOExecutor& ex, bool /*forcebuiltin*/ )
{
  if ( !value().exists() )
    return new BError( "Script has been destroyed" );

  Core::UOExecutor* uoexec = value().get_weakptr();

  switch ( id )
  {
  case MTH_GET_MEMBER:
  {
    if ( !ex.hasParams( 1 ) )
      return new BError( "Not enough parameters" );

    const String* mname;
    if ( ex.getStringParam( 0, mname ) )
    {
      BObjectRef ref_temp = get_member( mname->value().c_str() );
      BObjectRef& ref = ref_temp;
      BObject* bo = ref.get();
      BObjectImp* ret = bo->impptr();
      ret = ret->copy();
      if ( ret->isa( OTUninit ) )
      {
        std::string message = std::string( "Member " ) + std::string( mname->value() ) +
                              std::string( " not found on that object" );
        return new BError( message );
      }

      return ret;
    }
    return new BError( "Invalid parameter type" );
    break;
  }
  case MTH_SENDEVENT:
  {
    if ( !ex.hasParams( 1 ) )
      return new BError( "Not enough parameters" );
    BObjectImp* param0 = ex.getParamImp( 0 );
    if ( uoexec->signal_event( param0->copy() ) )
      return new BLong( 1 );
    return new BError( "Event queue is full, discarding event" );
  }

  case MTH_KILL:
    uoexec->seterror( true );

    // A Sleeping script would otherwise sit and wait until it wakes up to be killed.
    uoexec->revive();
    if ( uoexec->in_debugger_holdlist() )
      uoexec->revive_debugged();

    return new BLong( 1 );

  case MTH_LOADSYMBOLS:
  {
    int res = const_cast<EScriptProgram*>( uoexec->prog() )->read_dbg_file();
    return new BLong( !res );
  }

  case MTH_CLEAR_EVENT_QUEUE:  // DAVE added this 11/20
    return ( uoexec->clear_event_queue() );

    /**
     * .stacktrace(options?: struct{ format?: "array" | "string" } | uninit )
     */
  case MTH_STACKTRACE:
  {
    bool as_array = false;

    if ( ex.numParams() > 0 && !ex.getParamImp( 0 )->isa( OTUninit ) )
    {
      BStruct* options =
          static_cast<BStruct*>( ex.getParamImp( 0, Bscript::BObjectImp::OTStruct ) );

      if ( options == nullptr )
      {
        return new BError( "Invalid parameter type" );
      }

      if ( auto format_member = options->FindMember( "format" ) )
      {
        if ( !format_member->isa( OTString ) )
        {
          return new BError( "Invalid parameter type" );
        }

        auto format_string = format_member->getStringRep();

        // Explicitly case sensitive!
        if ( format_string == "array" )
        {
          as_array = true;
        }
        else if ( format_string != "string" )
        {
          return new BError( "Invalid parameter value" );
        }
      }
    }

    return uoexec->get_stacktrace( as_array );
  }

  default:
    return new BError( "undefined" );
  }
}

BObjectImp* ScriptExObjImp::call_polmethod( const char* methodname, UOExecutor& ex )
{
  ObjMethod* objmethod = getKnownObjMethod( methodname );
  if ( objmethod != nullptr )
    return this->call_polmethod_id( objmethod->id, ex );
  return new BError( "undefined" );
}

BObjectImp* GetGlobals( const UOExecutor* uoexec )
{
  BDictionary* dict = new BDictionary;

  BObjectRefVec::const_iterator itr = uoexec->Globals2->begin(), end = uoexec->Globals2->end();

  for ( unsigned idx = 0; itr != end; ++itr, ++idx )
  {
    const BObjectRef obref( ( *itr )->impref().copy() );

    if ( uoexec->prog()->globalvarnames.size() > idx )
      dict->addMember( uoexec->prog()->globalvarnames[idx].c_str(), obref );
    else
      dict->addMember( Clib::tostring( idx ).c_str(), obref );
  }
  return dict;
}

BObjectRef ScriptExObjImp::get_member_id( const int id )
{
  if ( !value().exists() )
    return BObjectRef( new BError( "Script has been destroyed" ) );

  UOExecutor* uoexec = value().get_weakptr();
  Module::UOExecutorModule* uoemod =
      static_cast<Module::UOExecutorModule*>( uoexec->findModule( "UO" ) );

  switch ( id )
  {
  case MBR_PID:
    return BObjectRef( new BLong( uoexec->pid() ) );
  case MBR_NAME:
    return BObjectRef( new String( uoexec->scriptname() ) );
  case MBR_PACKAGE:
  {
    const Pol::Plib::Package* pkg_ptr = uoexec->prog()->pkg;
    if ( pkg_ptr != nullptr )
    {
      return BObjectRef( new Pol::Module::PackageObjImp( pkg_ptr ) );
    }
    return BObjectRef( new BError( "Script has no package" ) );
  }
  case MBR_STATE:
    return BObjectRef( new String( uoexec->state() ) );
  case MBR_INSTR_CYCLES:
    return BObjectRef( new Double( static_cast<double>( uoexec->instr_cycles ) ) );
  case MBR_SLEEP_CYCLES:
    return BObjectRef( new Double( static_cast<double>( uoexec->sleep_cycles ) ) );
  case MBR_CONSEC_CYCLES:
  {
    u64 consec_cycles =
        uoexec->instr_cycles -
        ( uoexec->warn_runaway_on_cycle - Plib::systemstate.config.runaway_script_threshold ) +
        uoexec->runaway_cycles;
    return BObjectRef( new Double( static_cast<double>( consec_cycles ) ) );
  }
  case MBR_PC:
    return BObjectRef( new BLong( uoexec->PC ) );
  case MBR_CALL_DEPTH:
    return BObjectRef( new BLong( static_cast<int>( uoexec->ControlStack.size() ) ) );
  case MBR_NUM_GLOBALS:
    return BObjectRef( new BLong( static_cast<int>( uoexec->Globals2->size() ) ) );
  case MBR_VAR_SIZE:
    return BObjectRef( new BLong( static_cast<int>( uoexec->sizeEstimate() ) ) );
  case MBR_GLOBALS:
    return BObjectRef( GetGlobals( uoexec ) );
  case MBR_ATTACHED_TO:
    if ( uoemod->attached_chr_ != nullptr )
      return BObjectRef( new Module::ECharacterRefObjImp( uoemod->attached_chr_ ) );
    else if ( uoemod->attached_npc_ != nullptr )
      return BObjectRef( new Module::ECharacterRefObjImp( uoemod->attached_npc_ ) );
    else if ( uoemod->attached_item_.get() )
      return BObjectRef( new Module::EItemRefObjImp( uoemod->attached_item_.get() ) );
    else
      return BObjectRef( new BLong( 0 ) );
  case MBR_CONTROLLER:
    if ( uoemod->controller_.get() )
      return BObjectRef( new Module::ECharacterRefObjImp( uoemod->controller_.get() ) );
    else
      return BObjectRef( new BLong( 0 ) );

  default:
    return BObjectRef( UninitObject::create() );
  }
}

BObjectRef ScriptExObjImp::get_member( const char* membername )
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->get_member_id( objmember->id );
  return BObjectRef( UninitObject::create() );
}

ScriptWrapper::ScriptWrapper( ScriptExPtr script ) : _script( std::move( script ) ) {}
ScriptWrapper::~ScriptWrapper()
{
  if ( !_script.exists() )
    return;
  try
  {
    UOExecutor* uoexec = _script.get_weakptr();
    uoexec->keep_alive( false );
    uoexec->seterror( true );

    uoexec->revive();
    if ( uoexec->in_debugger_holdlist() )
      uoexec->revive_debugged();
  }
  catch ( ... )
  {
  }
}
ExportScriptObjImp::ExportScriptObjImp( UOExecutor* uoexec )
    : PolObjectImp( OTExportScript ), _ex( new ScriptWrapper( uoexec->weakptr ) ), _delayed( false )
{
}
ExportScriptObjImp::ExportScriptObjImp( std::shared_ptr<ScriptWrapper> script, bool delayed )
    : PolObjectImp( OTExportScript ), _ex( std::move( script ) ), _delayed( delayed )
{
}

const char* ExportScriptObjImp::typeOf() const
{
  return "ExportedScript";
}
std::string ExportScriptObjImp::getStringRep() const
{
  return "ExportedScript";
}
size_t ExportScriptObjImp::sizeEstimate() const
{
  return sizeof( *this ) + sizeof( ScriptWrapper );
}
u8 ExportScriptObjImp::typeOfInt() const
{
  return OTExportScript;
}
Bscript::BObjectImp* ExportScriptObjImp::copy() const
{
  return new ExportScriptObjImp( _ex, false );
}
Bscript::BObjectImp* ExportScriptObjImp::call_polmethod( const char* methodname,
                                                         Core::UOExecutor& ex )
{
  ObjMethod* objmethod = getKnownObjMethod( methodname );
  if ( objmethod != nullptr )
    return call_polmethod_id( objmethod->id, ex );
  return new BError( "undefined" );
}
Bscript::BObjectImp* ExportScriptObjImp::call_polmethod_id( const int id, Core::UOExecutor& ex,
                                                            bool /*forcebuiltin*/ )
{
  if ( !_ex->_script.exists() )
    return new BError( "Script destroyed" );
  UOExecutor* uoexec = _ex->_script.get_weakptr();
  switch ( id )
  {
  case MTH_CALL:
  {
    if ( _delayed )  // reentry
    {
      ex.pChild = nullptr;
      uoexec->pParent = nullptr;
      _delayed = false;
      if ( uoexec->error() )
        return new BLong( 0 );
      if ( uoexec->ValueStack.empty() )
        return new BLong( 1 );
      auto ret = uoexec->ValueStack.back().get()->impptr()->copy();
      uoexec->ValueStack.pop_back();
      return ret;
    }
    if ( !ex.hasParams( 1 ) )
      return new BError( "Not enough parameters" );
    const String* name;
    if ( !ex.getStringParam( 0, name ) )
      return new BError( "Invalid argument type" );
    ObjArray* arr = nullptr;
    if ( ex.hasParams( 2 ) )
    {
      if ( !ex.getObjArrayParam( 1, arr ) )
        return new BError( "Invalid argument type" );
    }
    const EScriptProgram* prog = uoexec->prog();
    bool found_func = false;
    u32 func_call_pc = 0;
    for ( const auto& func : prog->exported_functions )
    {
      if ( stricmp( func.name.c_str(), name->value().c_str() ) == 0 )
      {
        func_call_pc = func.PC;
        found_func = true;
        break;
      }
    }
    if ( !found_func )
      return new BError( "Exported function name not found" );
    if ( !uoexec->done )
      return new BError( "Executor is still running" );

    uoexec->initForFnCall( func_call_pc );
    if ( arr != nullptr )
    {
      for ( size_t i = 0; i < arr->ref_arr.size(); ++i )
        uoexec->pushArg( arr->ref_arr[i].get()->impptr() );
    }
    if ( ex.critical() )
    {
      uoexec->exec();
      Bscript::BObjectImp* ret;
      if ( uoexec->error() )
        ret = new BLong( 0 );
      else if ( uoexec->ValueStack.empty() )
        ret = new BLong( 1 );
      else
      {
        ret = uoexec->ValueStack.back().get()->impptr()->copy();
        uoexec->ValueStack.pop_back();
      }
      uoexec->set_running_to_completion( false );
      uoexec->suspend();
      return ret;
    }

    ex.pChild = uoexec;
    uoexec->pParent = &ex;
    ex.PC--;
    ex.ValueStack.push_back( BObjectRef( new BObject( UninitObject::create() ) ) );
    if ( arr != nullptr )
      ex.ValueStack.push_back( BObjectRef( new BObject( UninitObject::create() ) ) );
    ex.suspend();
    uoexec->revive();
    return new ExportScriptObjImp( _ex, true );
  }
  default:
    return new BError( "undefined" );
  }
}
Bscript::BObjectRef ExportScriptObjImp::get_member( const char* membername )
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return get_member_id( objmember->id );
  return BObjectRef( UninitObject::create() );
}
Bscript::BObjectRef ExportScriptObjImp::get_member_id( const int id )
{
  if ( !_ex->_script.exists() )
    return BObjectRef( new BError( "Script destroyed" ) );
  UOExecutor* uoexec = _ex->_script.get_weakptr();
  switch ( id )
  {
  case MBR_EXPORTED_FUNCTIONS:
  {
    auto array = std::make_unique<Bscript::ObjArray>();
    const EScriptProgram* prog = uoexec->prog();
    for ( const auto& func : prog->exported_functions )
      array->addElement( new String( func.name ) );
    return BObjectRef( array.release() );
  }
  default:
    return BObjectRef( UninitObject::create() );
  }
}
}  // namespace Pol::Core
