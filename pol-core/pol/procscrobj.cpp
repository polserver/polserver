#include "procscrobj.h"

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
#include "module/polsystemmod.h"
#include "module/uomod.h"
#include "polcfg.h"
#include "uoexec.h"
#include "uoscrobj.h"
#include <boost/process.hpp>

namespace Pol
{
namespace Core
{
using namespace Bscript;

BApplicObjType processobjimp_type;

ProcessObjImp::ProcessObjImp( UOExecutor* uoexec, boost::process::child&& process )
    : PolApplicObj<ScriptProcessDetails>(
          &processobjimp_type,
          {uoexec->weakptr, std::make_shared<boost::process::child>( std::move( process ) )} )
{
}


ProcessObjImp::ProcessObjImp( UOExecutor* uoexec, std::shared_ptr<boost::process::child> process )
    : PolApplicObj<ScriptProcessDetails>( &processobjimp_type, {uoexec->weakptr, process} )
{
}

const char* ProcessObjImp::typeOf() const
{
  return "ProcessRef";
}
u8 ProcessObjImp::typeOfInt() const
{
  return OTScriptExRef;
}

BObjectImp* ProcessObjImp::copy() const
{
  Core::UOExecutor* uoexec = script();

  if ( uoexec != nullptr )
    return new ProcessObjImp( uoexec, value().process );
  else
    return new BError( "Script has been destroyed" );
}


BObjectImp* ProcessObjImp::call_polmethod_id( const int id, UOExecutor& ex, bool /*forcebuiltin*/ )
{
  Core::UOExecutor* uoexec = script();

  if ( uoexec == nullptr )
    return new BError( "Script has been destroyed" );

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

BObjectImp* ProcessObjImp::call_polmethod( const char* methodname, UOExecutor& ex )
{
  ObjMethod* objmethod = getKnownObjMethod( methodname );
  if ( objmethod != nullptr )
    return this->call_polmethod_id( objmethod->id, ex );
  else
    return new BError( "undefined" );
}

BObjectRef ProcessObjImp::get_member_id( const int id )
{
  Core::UOExecutor* uoexec = script();

  if ( uoexec == nullptr )
    return BObjectRef( new BError( "Script has been destroyed" ) );

  Module::UOExecutorModule* uoemod =
      static_cast<Module::UOExecutorModule*>( uoexec->findModule( "UO" ) );

  switch ( id )
  {
  case MBR_PID:
    return BObjectRef( new BLong( uoexec->pid() ) );
  default:
    return BObjectRef( UninitObject::create() );
  }
}

BObjectRef ProcessObjImp::get_member( const char* membername )
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->get_member_id( objmember->id );
  else
    return BObjectRef( UninitObject::create() );
}
}  // namespace Core
}  // namespace Pol
