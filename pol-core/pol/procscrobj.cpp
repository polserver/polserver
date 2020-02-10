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
#include "polsem.h"
#include "uoexec.h"
#include "uoscrobj.h"
#undef BOOST_ASIO_HAS_BOOST_REGEX
#include <boost/asio/read_until.hpp>
#include <boost/process.hpp>

namespace Pol
{
namespace Core
{
using namespace Bscript;
namespace bp = boost::process;

BApplicObjType processobjimp_type;


ScriptProcessDetailsRef::ScriptProcessDetailsRef( ScriptProcessDetails* ptr )
    : ref_ptr<ScriptProcessDetails>( ptr )
{
}


ScriptProcessDetails::ScriptProcessDetails( boost::asio::io_context& ios, std::string exeName,
                                            std::vector<std::string> args )
    : ref_counted(),
      waitingScripts(),
      in(),
      out( ios ),
      err( ios ),
      outBuf(),
      errBuf(),
      exeName( exeName ),
      exitCode( 0 ),
      process(
          bp::exe = exeName, bp::args = args, ios, bp::std_out > this->out, bp::std_err > this->err,
          bp::std_in < this->in, bp::on_exit = [this]( int exit, const std::error_code& ec_in ) {
            // INFO_PRINT << "Process exited! " << this->out.is_open() << " " << exit << " "
            //           << ec_in.message() << "\n";
            boost::system::error_code ec;
            exitCode = exit;
            if ( this->out.is_open() )
            {
              boost::asio::read( this->out, this->outBuf, boost::asio::transfer_all(), ec );
              this->out.close();
            }
            if ( this->err.is_open() )
            {
              boost::asio::read( this->err, this->errBuf, boost::asio::transfer_all(), ec );
              this->err.close();
            }
            for ( auto& uoex_w : waitingScripts )
            {
              auto* uoex = uoex_w.exists() ? uoex_w.get_weakptr() : nullptr;
              if ( uoex )
              {
                PolLock lock;
                uoex->ValueStack.back().set( ec_in ? new BObject( new BError( ec_in.message() ) )
                                                   : new BObject( new BLong( 1 ) ) );
                uoex->revive();
              }
            }
          } )
{
}

ScriptProcessDetails::~ScriptProcessDetails()
{
  if ( process.running() && process.joinable() )
  {
    DEBUGLOG << "\tProcess " << exeName << " (pid " << process.id()
             << ") terminating due to destruction of script object.\n\tEnsure a call to "
                "Process.wait() or "
                "Process.detach() is called; see documentation for more details.\n";
    process.terminate();
  }
}

ProcessObjImp::ProcessObjImp( UOExecutor* uoexec, boost::asio::io_context& ios, std::string exeName,
                              std::vector<std::string> args )
    : PolApplicObj<ScriptProcessDetailsRef>(
          &processobjimp_type,
          ScriptProcessDetailsRef( new ScriptProcessDetails( ios, exeName, args ) ) )
{
}

ProcessObjImp::ProcessObjImp( ScriptProcessDetailsRef other )
    : PolApplicObj<ScriptProcessDetailsRef>( &processobjimp_type, other )
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
  case MTH_READLINE:
  {
    int timeout;
    short streamid;

    if ( ex.numParams() < 1 || !ex.getParam( 0, timeout ) )
      timeout = 0;

    if ( ex.numParams() < 2 || !ex.getParam( 1, streamid ) )
      streamid = 1;  // stdout

    auto& streambuf = streamid == 2 ? value()->errBuf : value()->outBuf;

    if ( !process().running() )
    {
      auto begin = boost::asio::buffers_begin( streambuf.data() );
      auto end = boost::asio::buffers_end( streambuf.data() );

      if ( begin == end )
        return new BError( "Process has terminated" );

      auto found = std::find( begin, end, '\n' );
      std::string command{begin, found};  // use found, even if found == end (ie, a flush)
      streambuf.consume( command.length() + 1 );
      return new String( command );
    }

    if ( !ex.suspend( timeout ) )
    {
      DEBUGLOG << "Script Error in '" << ex.scriptname() << "' PC=" << ex.PC << ": \n"
               << "\tThe execution of this script can't be blocked!\n";
      return new Bscript::BError( "Script can't be blocked" );
    }

    weak_ptr<Core::UOExecutor> uoexec_w = ex.weakptr;
    auto res = BObjectRef( new BObject( new BError( "Timeout" ) ) );
    boost::asio::async_read_until(
        streamid == 2 ? value()->err : value()->out, streambuf, '\n',
        [&streambuf, uoexec_w, res](
            const boost::system::error_code& error,  // Result of operation.

            std::size_t bytes_transferred  // Number of bytes copied into the
                                           // buffers. If an error occurred,
                                           // this will be the  number of
                                           // bytes successfully transferred
                                           // prior to the error.
        ) {
          Core::PolLock lock;
          auto* uoexec = uoexec_w.exists() ? uoexec_w.get_weakptr() : nullptr;
          if ( !uoexec )
          {
            DEBUGLOG << "ProcessRef.readline() script has been destroyed\n";
            return;
          }

          bool shouldWake = uoexec->in_hold_list() == Core::HoldListType::TIMEOUT_LIST &&
                            uoexec->ValueStack.back()->impptr() == res.get()->impptr();

          if ( shouldWake )
          {
            if ( !error )
            {
              assert( streambuf.size() > bytes_transferred );
              // Extract up to the first delimiter.
              std::string command{
                  boost::asio::buffers_begin( streambuf.data() ),
                  boost::asio::buffers_begin( streambuf.data() ) + bytes_transferred - 1};
              // Consume through the first delimiter so that subsequent async_read_until
              // will not reiterate over the same data.

              streambuf.consume( bytes_transferred );
              uoexec_w.get_weakptr()->ValueStack.back().set( new BObject( new String( command ) ) );
              uoexec_w.get_weakptr()->revive();
            }
            else
            {
              DEBUGLOG << "Error! " << error.message() << " transferred " << bytes_transferred
                       << "\n";
              uoexec_w.get_weakptr()->ValueStack.back().set(
                  new BObject( new BError( error.message() ) ) );
              uoexec_w.get_weakptr()->revive();
            }
          }
        } );
    return res.get()->impptr();
  }
  case MTH_WAIT:
    if ( !ex.suspend() )
    {
      DEBUGLOG << "Script Error in '" << ex.scriptname() << "' PC=" << ex.PC << ": \n"
               << "\tThe execution of this script can't be blocked!\n";
      return new Bscript::BError( "Script can't be blocked" );
    }
    value()->waitingScripts.push_back( ex.weakptr );
    return new BLong( 0 );  // dummy
  case MTH_WRITE:
  {
    BObjectImp* toWrite;
    if ( ex.numParams() < 1 || !( toWrite = ex.getParamImp( 0 ) ) )
      return new BError( "Invalid parameter type" );
    std::string toWriteStr = toWrite->getStringRep();
    return new BLong( value()->in.write( toWriteStr.c_str(), toWriteStr.size() ) );
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
  case MBR_EXITCODE:
    if ( process().running() )
      return BObjectRef( new BError( "Process is still running" ) );
    else
      return BObjectRef( new BLong( value()->exitCode ) );

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
