#include "server.h"

#include "../../bscript/eprog.h"
#include "../../clib/esignal.h"
#include "../../clib/logfacility.h"
#include "../../clib/threadhelp.h"
#include "../../clib/weakptr.h"
#include "../../plib/systemstate.h"
#include "../globals/network.h"
#include "../polclock.h"
#include "../polsem.h"
#include "../scrsched.h"
#include "../uoexec.h"

#include <dap/io.h>
#include <dap/network.h>
#include <dap/protocol.h>
#include <dap/session.h>
#include <memory>

namespace dap
{
// Define a custom `initialize` request that adds optional `password`.
class PolInitializeRequest : public InitializeRequest
{
public:
  optional<string> password;
};

DAP_STRUCT_TYPEINFO_EXT( PolInitializeRequest, InitializeRequest, "initialize",
                         DAP_FIELD( password, "password" ) );

// Define a custom `launch` request that adds `pid`.
class PolAttachRequest : public AttachRequest
{
public:
  number pid;
};

DAP_STRUCT_TYPEINFO_EXT( PolAttachRequest, AttachRequest, "attach", DAP_FIELD( pid, "pid" ) );
}  // namespace dap

namespace Pol
{
namespace Core
{
using namespace Bscript;

class DapDebugClientThread : public ExecutorDebugListener,
                             public std::enable_shared_from_this<ExecutorDebugListener>
{
public:
  DapDebugClientThread( const std::shared_ptr<dap::ReaderWriter>& rw )
      : _rw( rw ), _session( dap::Session::create() ), _uoexec_wptr( nullptr )
  {
  }

  virtual ~DapDebugClientThread() {}

  void run();

  void on_halt() override;

  void on_destroy() override;

private:
  std::shared_ptr<dap::ReaderWriter> _rw;
  std::unique_ptr<dap::Session> _session;
  weak_ptr<UOExecutor> _uoexec_wptr;
  ref_ptr<EScriptProgram> _script;
};

void DapDebugClientThread::on_halt()
{
  dap::StoppedEvent event;
  event.reason = "pause";
  event.threadId = 1;
  _session->send( event );
}

void DapDebugClientThread::on_destroy()
{
  dap::ExitedEvent event;
  event.exitCode = 0;
  _session->send( event );

  // threadsafe...? Is the event flushed to the socket before closing?
  _rw->close();
}

void DapDebugClientThread::run()
{
  POLLOG_INFO << "Debug client thread started.\n";

  // Session event handlers that are only attached once initialized with the password (if
  // required).
  auto attach_authorized_handlers = [&]()
  {
    _session->registerHandler( [&]( const dap::ConfigurationDoneRequest& )
                               { return dap::ConfigurationDoneResponse(); } );

    _session->registerHandler(
        [&]( const dap::PolAttachRequest& request ) -> dap::ResponseOrError<dap::AttachResponse>
        {
          PolLock lock;
          UOExecutor* uoexec;

          if ( _uoexec_wptr.exists() )
          {
            _uoexec_wptr.get_weakptr()->detach_debugger();
            _uoexec_wptr.clear();
          }

          auto pid = request.pid;
          if ( find_uoexec( pid, &uoexec ) )
          {
            EScriptProgram* prog = const_cast<EScriptProgram*>( uoexec->prog() );

            if ( prog->read_dbg_file() == 0 )
            {
              if ( !uoexec->attach_debugger( shared_from_this() ) )
                return dap::Error( "Debugger already attached." );

              _uoexec_wptr = uoexec->weakptr;
              _script.set( prog );

              return dap::AttachResponse();
            }
            return dap::Error( "No debug information available." );
          }
          return dap::Error( "Unknown process id '%d'", int( pid ) );
        } );

    // After sending an AttachResponse, check if executor is halted. If so, trigger the on_halt()
    // event.
    _session->registerSentHandler(
        [&]( const dap::ResponseOrError<dap::AttachResponse>& )
        {
          PolLock lock;
          if ( _uoexec_wptr.exists() )
            if ( _uoexec_wptr.get_weakptr()->halt() )
            {
              on_halt();
            }
        } );

    _session->registerHandler(
        [&]( const dap::ThreadsRequest& ) -> dap::ResponseOrError<dap::ThreadsResponse>
        {
          PolLock lock;
          if ( !_uoexec_wptr.exists() )
            return dap::Error( "No script attached." );

          UOExecutor* exec = _uoexec_wptr.get_weakptr();
          dap::ThreadsResponse response;
          dap::Thread thread;
          thread.id = 1;
          thread.name = Clib::tostring( exec->pid() );
          response.threads.push_back( thread );
          return response;
        } );

    _session->registerHandler( [&]( const dap::SetExceptionBreakpointsRequest& )
                               { return dap::SetExceptionBreakpointsResponse(); } );

    _session->registerHandler(
        [&]( const dap::ContinueRequest& ) -> dap::ResponseOrError<dap::ContinueResponse>
        {
          PolLock lock;
          if ( !_uoexec_wptr.exists() )
          {
            return dap::Error( "No script attached." );
          }

          UOExecutor* uoexec = _uoexec_wptr.get_weakptr();
          if ( !uoexec->in_debugger_holdlist() )
          {
            return dap::Error( "Script not ready to trace." );
          }

          uoexec->dbg_run();
          uoexec->revive_debugged();
          return dap::ContinueResponse{};
        } );

    _session->registerHandler(
        [&]( const dap::StackTraceRequest& ) -> dap::ResponseOrError<dap::StackTraceResponse>
        { return dap::StackTraceResponse{}; } );

    _session->registerHandler(
        [&]( const dap::ScopesRequest& ) -> dap::ResponseOrError<dap::ScopesResponse>
        { return dap::ScopesResponse{}; } );

    _session->registerHandler(
        [&]( const dap::VariablesRequest& ) -> dap::ResponseOrError<dap::VariablesResponse>
        { return dap::VariablesResponse{}; } );
  };

  // Session event handlers added before initialization
  _session->registerHandler(
      [&]( const dap::PolInitializeRequest& request )
          -> dap::ResponseOrError<dap::InitializeResponse>
      {
        if ( !Plib::systemstate.config.debug_password.empty() &&
             request.password.value( "" ) != Plib::systemstate.config.debug_password )
          return dap::Error( "Password not recognized." );

        attach_authorized_handlers();
        return dap::InitializeResponse{};
      } );

  _session->registerHandler(
      [&]( const dap::DisconnectRequest& )
      {
        PolLock lock;
        if ( _uoexec_wptr.exists() )
        {
          UOExecutor* exec = _uoexec_wptr.get_weakptr();
          exec->detach_debugger();
        }

        return dap::DisconnectResponse();
      } );

  _session->registerSentHandler(
      [&]( const dap::ResponseOrError<dap::DisconnectResponse>& response ) { _rw->close(); } );

  _session->registerSentHandler(
      [&]( const dap::ResponseOrError<dap::InitializeResponse>& response )
      {
        if ( response.error )
          _rw->close();
        else
          _session->send( dap::InitializedEvent() );
      } );

  // Error handler
  _session->onError(
      [&]( const char* msg )
      {
        POLLOG_ERROR << "Debugger session error: " << msg << "\n";
        _rw->close();
      } );

  // Attach the SocketReaderWriter to the Session and begin processing events.
  _session->bind( _rw, []() { POLLOG_INFO << "Debug session endpoint closed.\n"; } );

  while ( !Clib::exit_signalled && _rw->isOpen() )
  {
    pol_sleep_ms( 1000 );
  }

  {
    // Detach debugger in case a DisconnectRequest was not sent.
    PolLock lock;
    if ( _uoexec_wptr.exists() )
    {
      UOExecutor* exec = _uoexec_wptr.get_weakptr();
      exec->detach_debugger();
    }
  }

  _session.reset();

  // Close the socket endpoint if necessary.
  if ( _rw->isOpen() )
  {
    _rw->close();
  }

  POLLOG_INFO << "Debug client thread closing.\n";
}

void DapDebugServer::dap_debug_listen_thread( void )
{
  if ( Plib::systemstate.config.dap_debug_port )
  {
    auto server = dap::net::Server::create();

    auto onClientConnected = [&]( const std::shared_ptr<dap::ReaderWriter>& rw )
    {
      auto client = std::make_shared<DapDebugClientThread>( rw );
      Core::networkManager.auxthreadpool->push( [=]() { client->run(); } );
    };

    // If DebugLocalOnly, bind to localhost which allows connections only from local addresses.
    // Otherwise, bind to any address to also allow remote connections.
    auto address = Plib::systemstate.config.debug_local_only ? "localhost" : "0.0.0.0";

    auto started =
        server->start( address, Plib::systemstate.config.dap_debug_port, onClientConnected );

    if ( !started )
    {
      POLLOG_ERROR << "Failed to start DAP server.\n";
      return;
    }

    while ( !Clib::exit_signalled )
    {
      pol_sleep_ms( 1000 );
    }
  }
}
}  // namespace Core
}  // namespace Pol