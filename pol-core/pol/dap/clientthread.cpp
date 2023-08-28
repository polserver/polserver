#include "clientthread.h"

#include "../../bscript/bstruct.h"
#include "../../bscript/dict.h"
#include "../../bscript/eprog.h"
#include "../../bscript/impstr.h"
#include "../../clib/esignal.h"
#include "../../clib/logfacility.h"
#include "../../plib/systemstate.h"
#include "../module/uomod.h"
#include "../polsem.h"
#include "../scrdef.h"
#include "../scrsched.h"

#include <boost/algorithm/string/predicate.hpp>
#include <fstream>

namespace Pol
{
using namespace Core;
using namespace Bscript;
using namespace EscriptGrammar;
using namespace Compiler;

namespace Network
{
namespace DAP
{
DebugClientThread::DebugClientThread( const std::shared_ptr<dap::ReaderWriter>& rw )
    : _rw( rw ),
      _session( dap::Session::create() ),
      _uoexec_wptr( nullptr ),
      _expression_evaluator(),
      _global_scope_handle( 0 )
{
}

void DebugClientThread::on_halt()
{
  dap::StoppedEvent event;
  event.reason = "pause";
  event.threadId = 1;
  _session->send( event );
}

void DebugClientThread::on_destroy()
{
  dap::ExitedEvent event;
  event.exitCode = 0;
  _session->send( event );

  // threadsafe...? Is the event flushed to the socket before closing?
  _rw->close();
}

dap::ConfigurationDoneResponse DebugClientThread::handle_configurationDone(
    const dap::ConfigurationDoneRequest& )
{
  return dap::ConfigurationDoneResponse{};
}

dap::ResponseOrError<dap::AttachResponse> DebugClientThread::handle_attach(
    const dap::PolAttachRequest& request )
{
  PolLock lock;
  UOExecutor* uoexec;

  if ( _uoexec_wptr.exists() )
  {
    uoexec = _uoexec_wptr.get_weakptr();
    if ( uoexec->in_debugger_holdlist() )
      uoexec->revive_debugged();

    _variable_handles.reset();
    _global_scope_handle = 0;
    uoexec->detach_debugger();
    _uoexec_wptr.clear();
  }

  auto pid = static_cast<unsigned int>( request.pid );
  if ( find_uoexec( pid, &uoexec ) )
  {
    EScriptProgram* prog = const_cast<EScriptProgram*>( uoexec->prog() );

    if ( prog->read_dbg_file() == 0 )
    {
      if ( !uoexec->attach_debugger( weak_from_this() ) )
        return dap::Error( "Debugger already attached." );

      _uoexec_wptr = uoexec->weakptr;
      _script.set( prog );

      return dap::AttachResponse{};
    }
    return dap::Error( "No debug information available." );
  }
  return dap::Error( "Unknown process id '%d'", int( pid ) );
}

dap::ResponseOrError<dap::LaunchResponse> DebugClientThread::handle_launch(
    const dap::PolLaunchRequest& request )
{
  PolLock lock;
  ScriptDef sd;
  if ( !sd.config_nodie( request.program, nullptr, "scripts/" ) )
    return dap::Error( "Error in script name." );
  if ( !sd.exists() )
    return dap::Error( "Script " + sd.name() + " does not exist." );

  Module::UOExecutorModule* new_uoemod;

  if ( request.arg.has_value() && request.arg->length() > 0 )
  {
    new_uoemod = Core::start_script( sd, BObjectImp::unpack( request.arg->c_str() ) );
  }
  else
  {
    new_uoemod = Core::start_script( sd, nullptr );
  }

  if ( new_uoemod == nullptr )
  {
    return dap::Error( "Unable to start script" );
  }

  UOExecutor* uoexec = static_cast<UOExecutor*>( &new_uoemod->exec );
  EScriptProgram* prog = const_cast<EScriptProgram*>( uoexec->prog() );

  if ( prog->read_dbg_file() == 0 )
  {
    bool stopAtEntry = request.stopAtEntry.value( false );
    if ( !uoexec->attach_debugger( weak_from_this(), stopAtEntry ) )
      return dap::Error( "Debugger already attached." );

    _uoexec_wptr = uoexec->weakptr;
    _script.set( prog );

    return dap::LaunchResponse{};
  }
  return dap::Error( "No debug information available." );
}

dap::ResponseOrError<dap::PolProcessesResponse> DebugClientThread::handle_processes(
    const dap::PolProcessesRequest& request )
{
  PolLock lock;
  dap::PolProcessesResponse response;

  for ( const auto& [pid, uoexec] : scriptScheduler.getPidlist() )
  {
    if ( !request.filter.has_value() ||
         boost::icontains( uoexec->scriptname(), request.filter.value() ) )
    {
      dap::PolProcess entry;
      entry.id = pid;
      entry.program = uoexec->scriptname();

      if ( uoexec->halt() )
        entry.state = 2;  // debugging
      else if ( uoexec->in_hold_list() == NO_LIST )
        entry.state = 1;  // running
      else
        entry.state = 0;  // sleeping

      response.processes.push_back( entry );
    }
  }

  return response;
}

dap::ResponseOrError<dap::ThreadsResponse> DebugClientThread::handle_threads(
    const dap::ThreadsRequest& )
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
}


dap::ResponseOrError<dap::ContinueResponse> DebugClientThread::handle_continue(
    const dap::ContinueRequest& )
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

  _variable_handles.reset();
  _global_scope_handle = 0;
  uoexec->dbg_run();
  uoexec->revive_debugged();
  return dap::ContinueResponse{};
}

dap::ResponseOrError<dap::PauseResponse> DebugClientThread::handle_pause( const dap::PauseRequest& )
{
  PolLock lock;
  if ( !_uoexec_wptr.exists() )
  {
    return dap::Error( "No script attached." );
  }

  UOExecutor* exec = _uoexec_wptr.get_weakptr();
  exec->dbg_break();
  return dap::PauseResponse{};
}

dap::ResponseOrError<dap::EvaluateResponse> DebugClientThread::handle_evaluate(
    const dap::EvaluateRequest& request )
{
  PolLock lock;
  if ( !_uoexec_wptr.exists() )
  {
    return dap::Error( "No script attached." );
  }

  BObjectRef result;
  try
  {
    result = _expression_evaluator.evaluate( _uoexec_wptr.get_weakptr(), _script.get(),
                                             request.expression );
  }
  catch ( std::exception& ex )
  {
    return dap::Error( ex.what() );
  }

  dap::EvaluateResponse response;
  _variable_handles.set_response_details( result, response );
  return response;
}

dap::ResponseOrError<dap::SetVariableResponse> DebugClientThread::handle_setVariable(
    const dap::SetVariableRequest& request )
{
  PolLock lock;
  if ( !_uoexec_wptr.exists() )
  {
    return dap::Error( "No script attached." );
  }

  BObjectRef value;
  try
  {
    value =
        _expression_evaluator.evaluate( _uoexec_wptr.get_weakptr(), _script.get(), request.value );
  }
  catch ( std::exception& ex )
  {
    return dap::Error( ex.what() );
  }

  dap::SetVariableResponse response;

  auto reference_ptr = _variable_handles.get( static_cast<int>( request.variablesReference ) );

  if ( reference_ptr == nullptr )
  {
    return dap::Error( "Invalid variablesReference id %d", int( request.variablesReference ) );
  }

  std::visit(
      [&]( auto&& arg )
      {
        using T = std::decay_t<decltype( arg )>;
        if constexpr ( (std::is_same_v<T, GlobalReference> || std::is_same_v<T, FrameReference>))
        {
          for ( auto& [key, member] : arg.contents )
          {
            if ( key == request.name )
            {
              *member = value;
              _variable_handles.set_response_details( value, response );
            }
          }
        }
        else if constexpr ( std::is_same_v<T, VariableReference> )
        {
          auto value_set = _variable_handles.set_index_or_member( arg, request.name, value );
          _variable_handles.set_response_details( value_set, response );
        }
      },
      *reference_ptr );

  return response;
}

dap::ResponseOrError<dap::NextResponse> DebugClientThread::handle_next( const dap::NextRequest& )
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

  _variable_handles.reset();
  _global_scope_handle = 0;
  uoexec->dbg_step_over();
  uoexec->revive_debugged();
  return dap::NextResponse{};
}

dap::ResponseOrError<dap::StepInResponse> DebugClientThread::handle_stepIn(
    const dap::StepInRequest& )
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

  _variable_handles.reset();
  _global_scope_handle = 0;
  uoexec->dbg_step_into();
  uoexec->revive_debugged();
  return dap::StepInResponse{};
}

dap::ResponseOrError<dap::SetBreakpointsResponse> DebugClientThread::handle_setBreakpoints(
    const dap::SetBreakpointsRequest& request )
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
    filenum = static_cast<unsigned int>( request.source.sourceReference.value() );
  }
  else
  {
    if ( !request.source.path.has_value() )
    {
      return dap::Error( "No source location provided." );
    }

    auto filename_iter = std::find( _script->dbg_filenames.begin(), _script->dbg_filenames.end(),
                                    request.source.path.value() );

    if ( filename_iter == _script->dbg_filenames.end() )
    {
      return dap::Error( "File not in scope" );
    }

    filenum =
        static_cast<unsigned int>( std::distance( _script->dbg_filenames.begin(), filename_iter ) );
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

  for ( size_t breakpoint_index = 0; breakpoint_index < breakpoints.size(); breakpoint_index++ )
  {
    // Conditional breakpoints are not supported.
    if ( breakpoints[breakpoint_index].condition )
    {
      response.breakpoints[breakpoint_index].verified = false;
      continue;
    }

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
}

dap::ResponseOrError<dap::SourceResponse> DebugClientThread::handle_source(
    const dap::SourceRequest& request )
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
}

dap::ResponseOrError<dap::StackTraceResponse> DebugClientThread::handle_stackTrace(
    const dap::StackTraceRequest& )
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
  auto frameId = exec->ControlStack.size() + 1;

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
    auto dbgFunction = std::find_if( _script->dbg_functions.begin(), _script->dbg_functions.end(),
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
}

dap::ResponseOrError<dap::ScopesResponse> DebugClientThread::handle_scopes(
    const dap::ScopesRequest& request )
{
  PolLock lock;
  if ( !_uoexec_wptr.exists() )
  {
    return dap::Error( "No script attached." );
  }

  UOExecutor* uoexec = _uoexec_wptr.get_weakptr();
  if ( !uoexec->halt() )
  {
    return dap::Error( "Script must be halted to retrieve scopes." );
  }

  size_t frameId = request.frameId;

  // Frame IDs start a 1
  if ( frameId > uoexec->ControlStack.size() + 1 )
  {
    return dap::Error( "Unknown frameId '%d'", frameId );
  }

  if ( !_global_scope_handle )
  {
    _global_scope_handle = _variable_handles.create( GlobalReference( uoexec, _script.get() ) );
  }

  dap::ScopesResponse response;

  {
    dap::Scope scope;
    scope.name = "Locals @ " + Clib::tostring( frameId );
    scope.presentationHint = "locals";
    scope.variablesReference =
        _variable_handles.create( FrameReference( uoexec, _script.get(), frameId - 1 ) );
    response.scopes.push_back( scope );
  }

  {
    dap::Scope scope;
    scope.name = "Globals";
    scope.variablesReference = _global_scope_handle;
    response.scopes.push_back( scope );
  }

  return response;
}

dap::ResponseOrError<dap::VariablesResponse> DebugClientThread::handle_variables(
    const dap::VariablesRequest& request )
{
  PolLock lock;
  if ( !_uoexec_wptr.exists() )
  {
    return dap::Error( "No script attached." );
  }

  UOExecutor* uoexec = _uoexec_wptr.get_weakptr();
  if ( !uoexec->halt() )
  {
    return dap::Error( "Script must be halted to retrieve variables." );
  }

  dap::VariablesResponse response;

  auto reference_ptr = _variable_handles.get( static_cast<int>( request.variablesReference ) );

  if ( reference_ptr == nullptr )
  {
    return dap::Error( "Invalid variablesReference id %d", int( request.variablesReference ) );
  }

  std::visit(
      [&]( auto&& arg )
      {
        using T = std::decay_t<decltype( arg )>;
        if constexpr ( (std::is_same_v<T, GlobalReference>) || (std::is_same_v<T, FrameReference>))
        {
          for ( auto const& [key, member] : arg.contents )
          {
            dap::Variable current_var;
            current_var.name = key;
            _variable_handles.set_response_details( *member, current_var );
            response.variables.push_back( std::move( current_var ) );
          }
        }
        else if constexpr ( std::is_same_v<T, VariableReference> )
        {
          response.variables = _variable_handles.to_variables( arg );
        }
      },
      *reference_ptr );

  return response;
}

dap::ResponseOrError<dap::InitializeResponse> DebugClientThread::handle_initialize(
    const dap::PolInitializeRequest& request )
{
  if ( !Plib::systemstate.config.debug_password.empty() &&
       request.password.value( "" ) != Plib::systemstate.config.debug_password )
    return dap::Error( "Password not recognized." );

  dap::InitializeResponse response;
  response.supportsSetVariable = true;
  return response;
}

dap::ResponseOrError<dap::StepOutResponse> DebugClientThread::handle_stepOut(
    const dap::StepOutRequest& )
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

  _variable_handles.reset();
  _global_scope_handle = 0;
  uoexec->dbg_step_out();
  uoexec->revive_debugged();
  return dap::StepOutResponse{};
}

dap::ResponseOrError<dap::DisconnectResponse> DebugClientThread::handle_disconnect(
    const dap::DisconnectRequest& )
{
  PolLock lock;
  if ( _uoexec_wptr.exists() )
  {
    UOExecutor* exec = _uoexec_wptr.get_weakptr();
    if ( exec->in_debugger_holdlist() )
      exec->revive_debugged();

    _variable_handles.reset();
    _global_scope_handle = 0;
    exec->detach_debugger();
    _uoexec_wptr.clear();
  }

  return dap::DisconnectResponse{};
}

void DebugClientThread::after_pause( const dap::ResponseOrError<dap::PauseResponse>& )
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
}

void DebugClientThread::after_initialize(
    const dap::ResponseOrError<dap::InitializeResponse>& response )
{
  if ( response.error )
    _rw->close();
  else
    _session->send( dap::InitializedEvent() );
}

void DebugClientThread::after_disconnect( const dap::ResponseOrError<dap::DisconnectResponse>& )
{
  _rw->close();
}

void DebugClientThread::after_attach( const dap::ResponseOrError<dap::AttachResponse>& )
{
  PolLock lock;
  if ( _uoexec_wptr.exists() )
    if ( _uoexec_wptr.get_weakptr()->halt() )
    {
      on_halt();
    }
}

void DebugClientThread::on_error( const char* msg )
{
  POLLOG_ERROR << "Debugger session error: " << msg << "\n";
  _rw->close();
}

void DebugClientThread::run()
{
  POLLOG_INFO << "Debug client thread started.\n";

  // Session event handlers that are only attached once initialized with the password (if
  // required).
  auto attach_authorized_handlers = [this]()
  {
    _session->registerHandler( [this]( const dap::ConfigurationDoneRequest& request )
                               { return handle_configurationDone( request ); } );

    _session->registerHandler( [this]( const dap::PolAttachRequest& request )
                               { return handle_attach( request ); } );

    // After sending an AttachResponse, check if executor is halted. If so, trigger the on_halt()
    // event.
    _session->registerSentHandler(
        [this]( const dap::ResponseOrError<dap::AttachResponse>& response )
        { after_attach( response ); } );

    _session->registerHandler( [this]( const dap::PolLaunchRequest& request )
                               { return handle_launch( request ); } );

    _session->registerHandler( [this]( const dap::PolProcessesRequest& request )
                               { return handle_processes( request ); } );

    _session->registerHandler( [this]( const dap::ThreadsRequest& request )
                               { return handle_threads( request ); } );

    _session->registerHandler( [this]( const dap::ContinueRequest& request )
                               { return handle_continue( request ); } );

    _session->registerHandler( [this]( const dap::PauseRequest& request )
                               { return handle_pause( request ); } );

    _session->registerHandler( [this]( const dap::EvaluateRequest& request )
                               { return handle_evaluate( request ); } );

    _session->registerHandler( [this]( const dap::SetVariableRequest& request )
                               { return handle_setVariable( request ); } );

    // After sending a PauseResponse, check if the script is paused. If so, send a StoppedEvent.
    _session->registerSentHandler(
        [this]( const dap::ResponseOrError<dap::PauseResponse>& response )
        { after_pause( response ); } );

    _session->registerHandler( [this]( const dap::NextRequest& request )
                               { return handle_next( request ); } );

    _session->registerHandler( [this]( const dap::StepInRequest& request )
                               { return handle_stepIn( request ); } );

    _session->registerHandler( [this]( const dap::SetBreakpointsRequest& request )
                               { return handle_setBreakpoints( request ); } );

    _session->registerHandler( [this]( const dap::SourceRequest& request )
                               { return handle_source( request ); } );

    _session->registerHandler( [this]( const dap::StackTraceRequest& request )
                               { return handle_stackTrace( request ); } );

    _session->registerHandler( [this]( const dap::ScopesRequest& request )
                               { return handle_scopes( request ); } );

    _session->registerHandler( [this]( const dap::VariablesRequest& request )
                               { return handle_variables( request ); } );

    _session->registerHandler( [this]( const dap::StepOutRequest& request )
                               { return handle_stepOut( request ); } );
  };

  // Session event handlers added before initialization
  _session->registerHandler(
      [&]( const dap::PolInitializeRequest& request )
      {
        auto response = handle_initialize( request );
        if ( !response.error )
        {
          attach_authorized_handlers();
        }
        return response;
      } );

  _session->registerHandler( [this]( const dap::DisconnectRequest& request )
                             { return handle_disconnect( request ); } );

  _session->registerSentHandler(
      [this]( const dap::ResponseOrError<dap::DisconnectResponse>& response )
      { after_disconnect( response ); } );

  _session->registerSentHandler(
      [this]( const dap::ResponseOrError<dap::InitializeResponse>& response )
      { after_initialize( response ); } );

  // Error handler
  _session->onError( [this]( const char* msg ) { on_error( msg ); } );

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

      _variable_handles.reset();
      _global_scope_handle = 0;
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
}  // namespace DAP
}  // namespace Network
}  // namespace Pol