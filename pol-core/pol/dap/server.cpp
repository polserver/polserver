#include "server.h"

#include "../../bscript/eprog.h"
#include "../../clib/esignal.h"
#include "../../clib/logfacility.h"
#include "../../clib/threadhelp.h"
#include "../../clib/weakptr.h"
#include "../../plib/systemstate.h"
#include "../globals/network.h"
#include "../globals/uvars.h"
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
namespace Network
{
using namespace Core;
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
            uoexec = _uoexec_wptr.get_weakptr();
            if ( uoexec->in_debugger_holdlist() )
              uoexec->revive_debugged();

            uoexec->detach_debugger();
            _uoexec_wptr.clear();
          }

          auto pid = request.pid;
          if ( find_uoexec( pid, &uoexec ) )
          {
            EScriptProgram* prog = const_cast<EScriptProgram*>( uoexec->prog() );

            if ( prog->read_dbg_file() == 0 )
            {
              if ( !uoexec->attach_debugger( weak_from_this() ) )
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
        [&]( const dap::PauseRequest& ) -> dap::ResponseOrError<dap::PauseResponse>
        {
          PolLock lock;
          if ( !_uoexec_wptr.exists() )
          {
            return dap::Error( "No script attached." );
          }

          UOExecutor* exec = _uoexec_wptr.get_weakptr();
          exec->dbg_break();
          return dap::PauseResponse{};
        } );

    // After sending a PauseResponse, check if the script is paused. If so, send a StoppedEvent.
    _session->registerSentHandler(
        [&]( const dap::ResponseOrError<dap::PauseResponse>& )
        {
          PolLock lock;
          if ( !_uoexec_wptr.exists() )
          {
            return;
          }

          UOExecutor* uoexec = _uoexec_wptr.get_weakptr();
          if ( !uoexec->halt() )
          {
            return;
          }

          on_halt();
        } );

    _session->registerHandler(
        [&]( const dap::NextRequest& ) -> dap::ResponseOrError<dap::NextResponse>
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

          uoexec->dbg_step_over();
          uoexec->revive_debugged();
          return dap::NextResponse();
        } );

    _session->registerHandler(
        [&]( const dap::StepInRequest& ) -> dap::ResponseOrError<dap::StepInResponse>
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

          uoexec->dbg_step_into();
          uoexec->revive_debugged();
          return dap::StepInResponse();
        } );

    _session->registerHandler(
        [&]( const dap::SetBreakpointsRequest& request )
            -> dap::ResponseOrError<dap::SetBreakpointsResponse>
        {
          PolLock lock;
          if ( !_uoexec_wptr.exists() )
          {
            return dap::Error( "No script attached." );
          }

          UOExecutor* uoexec = _uoexec_wptr.get_weakptr();

          unsigned int filenum;

          if ( request.source.sourceReference.has_value() )
          {
            filenum = request.source.sourceReference.value();
          }
          else
          {
            if ( !request.source.path.has_value() )
            {
              return dap::Error( "No source location provided." );
            }

            auto filename_iter =
                std::find( _script->dbg_filenames.begin(), _script->dbg_filenames.end(),
                           request.source.path.value() );

            if ( filename_iter == _script->dbg_filenames.end() )
            {
              return dap::Error( "File not in scope" );
            }

            filenum = std::distance( _script->dbg_filenames.begin(), filename_iter );
          }

          if ( filenum >= _script->dbg_filenames.size() )
          {
            return dap::Error( "File %d out of range", filenum );
          }

          std::set<unsigned> remove_bps;
          for ( unsigned i = 0; i < _script->dbg_filenum.size(); ++i )
          {
            if ( _script->dbg_filenum[i] == filenum )
            {
              remove_bps.insert( i );
            }
          }
          uoexec->dbg_clrbps( remove_bps );

          dap::SetBreakpointsResponse response;

          auto breakpoints = request.breakpoints.value( {} );
          response.breakpoints.resize( breakpoints.size() );

          for ( size_t breakpoint_index = 0; breakpoint_index < breakpoints.size();
                breakpoint_index++ )
          {
            for ( unsigned i = 0; i < _script->dbg_filenum.size(); ++i )
            {
              if ( _script->dbg_filenum[i] == filenum &&
                   _script->dbg_linenum[i] == breakpoints[breakpoint_index].line )
              {
                response.breakpoints[breakpoint_index].verified = true;
                uoexec->dbg_setbp( i );
                break;
              }
            }
          }

          return response;
        } );

    _session->registerHandler(
        [&]( const dap::SourceRequest& request ) -> dap::ResponseOrError<dap::SourceResponse>
        {
          PolLock lock;
          if ( !_uoexec_wptr.exists() )
          {
            return dap::Error( "No script attached." );
          }

          size_t filenum = request.sourceReference;
          if ( filenum >= _script->dbg_filenames.size() )
          {
            return dap::Error( "File out of range: '%d'", int( filenum ) );
          }

          auto filepath = _script->dbg_filenames[filenum];

          std::ifstream ifs( filepath );
          if ( !ifs.is_open() )
            return dap::Error( "File # out of range" );

          std::stringstream buffer;
          buffer << ifs.rdbuf();

          dap::SourceResponse response;
          response.content = buffer.str();
          return response;
        } );

    _session->registerHandler(
        [&]( const dap::StackTraceRequest& ) -> dap::ResponseOrError<dap::StackTraceResponse>
        {
          PolLock lock;
          if ( !_uoexec_wptr.exists() )
          {
            return dap::Error( "No script attached." );
          }

          UOExecutor* exec = _uoexec_wptr.get_weakptr();

          if ( !exec->halt() )
          {
            return dap::Error( "Script must be halted to retrieve stack trace." );
          }

          std::vector<ReturnContext> stack = exec->ControlStack;
          dap::StackTraceResponse response;

          unsigned int PC;

          {
            ReturnContext rc;
            rc.PC = exec->PC;
            rc.ValueStackDepth = static_cast<unsigned int>( exec->ValueStack.size() );
            stack.push_back( rc );
          }

          // Bottom frame starts at index 1
          auto frameId = exec->ControlStack.size() + 2;

          while ( !stack.empty() )
          {
            ReturnContext& rc = stack.back();
            PC = rc.PC;
            stack.pop_back();

            dap::Source source;

            source.name = _script->dbg_filenames[_script->dbg_filenum[PC]];
            source.path = _script->dbg_filenames[_script->dbg_filenum[PC]];

            dap::StackFrame frame;
            frame.line = _script->dbg_linenum[PC];
            frame.column = 1;
            auto dbgFunction =
                std::find_if( _script->dbg_functions.begin(), _script->dbg_functions.end(),
                              [&]( auto& i ) { return i.firstPC <= PC && PC <= i.lastPC; } );

            if ( dbgFunction != _script->dbg_functions.end() )
            {
              frame.name = dbgFunction->name;
            }
            else
            {
              frame.name = "(program)";
            }

            frame.id = frameId--;
            frame.source = source;

            response.stackFrames.push_back( frame );
          }

          return response;
        } );

    _session->registerHandler(
        [&]( const dap::ScopesRequest& ) -> dap::ResponseOrError<dap::ScopesResponse>
        { return dap::ScopesResponse{}; } );

    _session->registerHandler(
        [&]( const dap::VariablesRequest& ) -> dap::ResponseOrError<dap::VariablesResponse>
        { return dap::VariablesResponse{}; } );

    _session->registerHandler(
        [&]( const dap::StepOutRequest& ) -> dap::ResponseOrError<dap::StepOutResponse>
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

          uoexec->dbg_step_out();
          uoexec->revive_debugged();
          return dap::StepOutResponse{};
        } );
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
          if ( exec->in_debugger_holdlist() )
            exec->revive_debugged();

          exec->detach_debugger();
          _uoexec_wptr.clear();
        }

        return dap::DisconnectResponse();
      } );

  _session->registerSentHandler( [&]( const dap::ResponseOrError<dap::DisconnectResponse>& )
                                 { _rw->close(); } );

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
      if ( exec->in_debugger_holdlist() )
        exec->revive_debugged();

      exec->detach_debugger();
      _uoexec_wptr.clear();
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

DapDebugServer::DapDebugServer()
{
  _server = dap::net::Server::create();

  // If DebugLocalOnly, bind to localhost which allows connections only from local addresses.
  // Otherwise, bind to any address to also allow remote connections.
  auto address = Plib::systemstate.config.debug_local_only ? "localhost" : "0.0.0.0";

  auto started =
      _server->start( address, Plib::systemstate.config.dap_debug_port,
                      []( const std::shared_ptr<dap::ReaderWriter>& rw )
                      {
                        auto client = std::make_shared<DapDebugClientThread>( rw );
                        Core::networkManager.auxthreadpool->push( [=]() { client->run(); } );
                      } );

  if ( !started )
  {
    POLLOG_ERROR << "Failed to start DAP server.\n";
    _server.reset();
  }
}

DapDebugServer::~DapDebugServer() = default;

}  // namespace Network
}  // namespace Pol