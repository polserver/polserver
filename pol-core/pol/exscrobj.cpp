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

#include "../bscript/berror.h"
#include "../bscript/bobject.h"
#include "../bscript/dict.h"
#include "../bscript/executor.h"
#include "../bscript/impstr.h"
#include "../bscript/objmembers.h"
#include "../bscript/objmethods.h"
#include "../clib/strutil.h"
#include "../plib/systemstate.h"
#include "module/uomod.h"
#include "module/polsystemmod.h"
#include "polcfg.h"
#include "uoexec.h"
#include "uoscrobj.h"

namespace Pol
{
namespace Core
{
using namespace Bscript;

BApplicObjType scriptexobjimp_type;

ScriptExObjImp::ScriptExObjImp( UOExecutor* uoexec )
    : BApplicObj<ScriptExPtr>( &scriptexobjimp_type, uoexec->weakptr )
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
  else
    return new BError( "Script has been destroyed" );
}


BObjectImp* ScriptExObjImp::call_method_id( const int id, Executor& ex, bool /*forcebuiltin*/ )
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
      else
      {
        return ret;
      }
    }
    else
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
    else
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

  default:
    return new BError( "undefined" );
  }
}

BObjectImp* ScriptExObjImp::call_method( const char* methodname, Executor& ex )
{
  ObjMethod* objmethod = getKnownObjMethod( methodname );
  if ( objmethod != nullptr )
    return this->call_method_id( objmethod->id, ex );
  else
    return new BError( "undefined" );
}

BObjectImp* GetGlobals( const UOExecutor* uoexec )
{
  BDictionary* dict = new BDictionary;

  BObjectRefVec::const_iterator itr = uoexec->Globals2.begin(), end = uoexec->Globals2.end();

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
    return BObjectRef( new Pol::Module::PackageObjImp( uoexec->prog()->pkg ) );
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
    return BObjectRef( new BLong( static_cast<int>( uoexec->Globals2.size() ) ) );
  case MBR_VAR_SIZE:
    return BObjectRef( new BLong( static_cast<int>( uoexec->sizeEstimate() ) ) );
  case MBR_GLOBALS:
    return BObjectRef( GetGlobals( uoexec ) );
  case MBR_ATTACHED_TO:
    if ( uoemod->attached_chr_ != nullptr )
      return BObjectRef( new Module::ECharacterRefObjImp( uoemod->attached_chr_ ) );
    else if ( uoemod->attached_npc_ != nullptr )
      return BObjectRef( new Module::ECharacterRefObjImp( uoemod->attached_npc_ ) );
    else if ( uoemod->attached_item_ )
      return BObjectRef( new Module::EItemRefObjImp( uoemod->attached_item_ ) );
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
  else
    return BObjectRef( UninitObject::create() );
}
}
}
