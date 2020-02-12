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
#include "../clib/make_unique.hpp"
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

#define _INFO_PRINT_ DEBUGLOG

namespace Pol
{
namespace Core
{
using namespace Bscript;
namespace bp = boost::process;
namespace ba = boost::asio;

BApplicObjType processobjimp_type;


ScriptProcessDetailsRef::ScriptProcessDetailsRef( ScriptProcessDetails* ptr )
    : ref_ptr<ScriptProcessDetails>( ptr )
{
}

ScriptProcessDetails::ScriptProcessDetails( UOExecutor* uoexec, boost::asio::io_context& ios,
                                            std::string exeName, std::vector<std::string> args )
    : ref_counted(),
      waitingScripts(),
      weakptr(),
      readerOut( uoexec, ios ),
      readerErr( uoexec, ios ),
      in(),
      exeName( exeName ),
      exitCode( 0 )

{
  weakptr.set( this );
  auto this_w = weak_ptr<ScriptProcessDetails>( weakptr );
  process = bp::child(
      bp::exe = exeName, bp::args = args, ios, bp::std_out > readerOut.pipe,
      bp::std_err > readerErr.pipe, bp::std_in < this->in,
      bp::on_exit = [this_w]( int exit, const std::error_code& ec_in ) {
        // If process exited due to ScriptProcessDetails destruction, the `this` reference
        // is no longer valid.
        auto* thiz = this_w.exists() ? this_w.get_weakptr() : nullptr;
        if ( !thiz )
          return;

        // FIXME: better separate this lock
        PolLock lock;
        thiz->readerOut.readAll();
        thiz->readerErr.readAll();

        boost::system::error_code ec;
        thiz->exitCode = exit;

        for ( auto& uoex_w : thiz->waitingScripts )
        {
          auto* uoex = uoex_w.exists() ? uoex_w.get_weakptr() : nullptr;
          if ( uoex )
          {
            uoex->ValueStack.back().set( ec_in ? new BObject( new BError( ec_in.message() ) )
                                               : new BObject( new BLong( 1 ) ) );
            uoex->revive();
          }
        }
      } );
  readerOut.bufferOne();
  readerErr.bufferOne();
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
          ScriptProcessDetailsRef( new ScriptProcessDetails( uoexec, ios, exeName, args ) ) )
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
      timeout = -1;  // forever

    if ( ex.numParams() < 2 || !ex.getParam( 1, streamid ) )
      streamid = 1;  // stdout

    auto& reader = streamid == 2 ? value()->readerErr : value()->readerOut;

    return reader.getline( &ex, timeout );
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
}  // namespace Core

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
BufferReader::BufferReader( UOExecutor* uoexec, ba::io_context& context )
    : buffer(),
      stream( &buffer ),
      pipe( context ),
      uoexec_w( uoexec->weakptr ),
      isWaiting( false ),
      lineLock()
{
}

BufferReader::~BufferReader()
{
  pipe.cancel();
}

void BufferReader::bufferOne()
{
  _INFO_PRINT_ << "Reading from buffer\n";
  // FIXME: match all \n \r\n \r
  boost::asio::async_read_until(
      pipe, buffer, '\n',
      [this]( const boost::system::error_code& error,  // Result of operation.

              std::size_t bytes_transferred  // Number of bytes copied into the
                                             // buffers. If an error occurred,
                                             // this will be the  number of
                                             // bytes successfully transferred
                                             // prior to the error.
      ) {
        // The `this` reference is only valid iff no error occurred.
        if ( !error )
        {
          Clib::SpinLockGuard guard( lineLock );
          // Only read from buffer if our line buffer is empty.
          // FIXME: Verify if line == nullptr is always true
          if ( line == nullptr )
          {
            _INFO_PRINT_ << "buffer.size() = " << buffer.size()
                         << ", bytes_transferred = " << bytes_transferred << "\n";
            std::string readline;
            std::getline( stream, readline );
            if ( readline.back() == '\r' )
            {
              readline.pop_back();
            }
            _INFO_PRINT_ << "String: '" << readline << "'\n";
            if ( !isWaiting )
            {
              _INFO_PRINT_ << "No pending request; store in line\n";
              // line.swap( std::move( readline ) );
              line = Clib::make_unique<std::string>( std::move( readline ) );
              return;
            }
            else
            {
              Core::PolLock lock;
              auto* uoexec = uoexec_w.exists() ? uoexec_w.get_weakptr() : nullptr;
              // If calling script died, store in line buffer.
              if ( !uoexec )
              {
                _INFO_PRINT_ << "Pending request script died; store in line\n";
                isWaiting = false;
                line = Clib::make_unique<std::string>( std::move( readline ) );
                return;
              }
              // Respond to request
              else
              {
                _INFO_PRINT_ << "Pending request valid; wake script\n";
                passert( uoexec->blocked() );
                uoexec->ValueStack.back().set( new BObject( new String( std::move( readline ) ) ) );
                isWaiting = false;
                uoexec->revive();
                // Prefetch next line
                bufferOne();
                return;
              }
            }
          }
          else
          {
            // There already is a next line buffer
            _INFO_PRINT_ << "Next line buffer exists; skipping read from buffer\n";
            return;
          }
        }
      } );
}

// BError or String
// Ran inside PolLock (scripts thread)
Bscript::BObjectImp* BufferReader::getline( UOExecutor* uoex, int timeout )
{
  if ( uoex != ( uoexec_w.exists() ? uoexec_w.get_weakptr() : nullptr ) )
  {
    return new BError( "Only initiating script can use Process.readline()" );
  }

  Clib::SpinLockGuard lock( lineLock );
  // If no line buffer
  if ( line == nullptr )
  {
    // If pipe is closed, read from streambuf filled at process exit
    if ( !pipe.is_open() )
    {
      // If the streambuf is empty, return error
      if ( buffer.size() == 0 )
      {
        _INFO_PRINT_ << "Empty buffer\n";
        return new BError( "Process has terminated" );
      }
      // Get line from streambuf
      std::string readline;
      std::getline( stream, readline );
      if ( readline.back() == '\r' )
      {
        readline.pop_back();
      }
      _INFO_PRINT_ << "Got str from buffer: " << readline << "\n";
      return new String( readline );
    }

    // 0 timeout means synchronous check
    if ( timeout == 0 )
    {
      return new BError( "Timeout" );
    }

    // Create a request
    if ( !uoex->suspend( timeout == -1 ? 0 : timeout,
                         [this]( UOExecutor* uoex ) { isWaiting = false; } ) )
    {
      DEBUGLOG << "Script Error in '" << uoex->scriptname() << "' PC=" << uoex->PC << ": \n"
               << "\tThe execution of this script can't be blocked!\n";
      return new Bscript::BError( "Script can't be blocked" );
    }
    passert_r( !isWaiting, "BufferReader already waiting for line" );
    isWaiting = true;
    return new BError( "Timeout" );
  }
  else  // The line buffer is already filled.
  {
    std::string readline = *line.release();
    _INFO_PRINT_ << "Got str from line: " << readline << "\n";
    // Prefetch next line
    bufferOne();
    return new String( readline );
  }
}

void BufferReader::readAll()
{
  if ( pipe.is_open() )
  {
    _INFO_PRINT_ << "READ ALL\n";
    boost::system::error_code ec;
    boost::asio::read( pipe, buffer, boost::asio::transfer_all(), ec );
    pipe.close();
  }
}
}  // namespace Core
}  // namespace Pol
