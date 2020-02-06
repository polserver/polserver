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
#include "../clib/logfacility.h"
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
namespace bp = boost::process;

BApplicObjType processobjimp_type;

ScriptProcessDetails::ScriptProcessDetails( UOExecutor* uoexec, boost::asio::io_context& ios,
                                            std::string exeName, std::vector<std::string> args )
    : script( uoexec->weakptr ),
      streamOut(),
      streamErr(),
      process(
          bp::exe = exeName, bp::args = args, ios, bp::std_out > this->streamOut,
          bp::std_err > this->streamErr, bp::std_in.close(),
          bp::on_exit = []( int exit, const std::error_code& ec_in ) {
            INFO_PRINT << "Process exited!\n";
          } )
{
}

ProcessObjImp::ProcessObjImp( UOExecutor* uoexec, boost::asio::io_context& ios, std::string exeName,
                              std::vector<std::string> args )
    : PolApplicObj<std::shared_ptr<ScriptProcessDetails>>(
          &processobjimp_type,
          std::make_shared<ScriptProcessDetails>( uoexec, ios, exeName, args ) )
{
}

ProcessObjImp::ProcessObjImp( std::shared_ptr<ScriptProcessDetails> other )
    : PolApplicObj<std::shared_ptr<ScriptProcessDetails>>( &processobjimp_type, other )
{
}

const char* ProcessObjImp::typeOf() const
{
  return "ProcessRef";
}
u8 ProcessObjImp::typeOfInt() const
{
  return OTProcessRef;
}

BObjectImp* ProcessObjImp::copy() const
{
  return new ProcessObjImp( obj_ );
}

BObjectImp* ProcessObjImp::call_polmethod_id( const int id, UOExecutor& ex, bool /*forcebuiltin*/ )
{
  switch ( id )
  {
  case MTH_KILL:
  {
    if ( process().running() )
      return new BError( "Process is not running" );

    process().terminate();
    return new BLong( 1 );
    break;
  }

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
  switch ( id )
  {
  case MBR_PID:
    return BObjectRef( new BLong( process().id() ) );
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
