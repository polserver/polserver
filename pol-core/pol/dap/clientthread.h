#ifndef DAP_CLIENTTHREAD_H
#define DAP_CLIENTTHREAD_H

#include "handles.h"
#include "proto.h"

#include "../uoexec.h"

#include <dap/session.h>
#include <memory>

namespace dap
{
class ReaderWriter;
}

namespace Pol
{
namespace Core
{
}
namespace Bscript
{
class EScriptProgram;

}
namespace Network
{
namespace DAP
{
class DebugClientThread : public Bscript::ExecutorDebugListener,
                          public std::enable_shared_from_this<Bscript::ExecutorDebugListener>
{
public:
  DebugClientThread( const std::shared_ptr<dap::ReaderWriter>& rw );
  virtual ~DebugClientThread() {}

  void run();

  void on_halt() override;

  void on_destroy() override;

private:
  // Handlers
  dap::ConfigurationDoneResponse handle_configurationDone( const dap::ConfigurationDoneRequest& );
  dap::ResponseOrError<dap::AttachResponse> handle_attach( const dap::PolAttachRequest& );
  dap::ResponseOrError<dap::LaunchResponse> handle_launch( const dap::PolLaunchRequest& );
  dap::ResponseOrError<dap::PolProcessesResponse> handle_processes(
      const dap::PolProcessesRequest& );
  dap::ResponseOrError<dap::ThreadsResponse> handle_threads( const dap::ThreadsRequest& );
  dap::ResponseOrError<dap::SetExceptionBreakpointsResponse> handle_setExceptionBreakpoints(
      const dap::SetExceptionBreakpointsRequest& );
  dap::ResponseOrError<dap::ContinueResponse> handle_continue( const dap::ContinueRequest& );
  dap::ResponseOrError<dap::PauseResponse> handle_pause( const dap::PauseRequest& );
  dap::ResponseOrError<dap::NextResponse> handle_next( const dap::NextRequest& );
  dap::ResponseOrError<dap::StepInResponse> handle_stepIn( const dap::StepInRequest& );
  dap::ResponseOrError<dap::SetBreakpointsResponse> handle_setBreakpoints(
      const dap::SetBreakpointsRequest& );
  dap::ResponseOrError<dap::SourceResponse> handle_source( const dap::SourceRequest& );
  dap::ResponseOrError<dap::StackTraceResponse> handle_stackTrace( const dap::StackTraceRequest& );
  dap::ResponseOrError<dap::ScopesResponse> handle_scopes( const dap::ScopesRequest& );
  dap::ResponseOrError<dap::VariablesResponse> handle_variables( const dap::VariablesRequest& );
  dap::ResponseOrError<dap::StepOutResponse> handle_stepOut( const dap::StepOutRequest& );
  dap::ResponseOrError<dap::InitializeResponse> handle_initialize(
      const dap::PolInitializeRequest& );
  dap::ResponseOrError<dap::DisconnectResponse> handle_disconnect( const dap::DisconnectRequest& );

  // Sent handlers
  void after_pause( const dap::ResponseOrError<dap::PauseResponse>& );
  void after_initialize( const dap::ResponseOrError<dap::InitializeResponse>& );
  void after_disconnect( const dap::ResponseOrError<dap::DisconnectResponse>& );
  void after_attach( const dap::ResponseOrError<dap::AttachResponse>& );

  // Other handlers
  void on_error( const char* msg );

private:
  std::shared_ptr<dap::ReaderWriter> _rw;
  std::unique_ptr<dap::Session> _session;
  weak_ptr<Core::UOExecutor> _uoexec_wptr;
  ref_ptr<Bscript::EScriptProgram> _script;
  Handles _variable_handles;
  int _global_scope_handle;
};

}  // namespace DAP
}  // namespace Network
}  // namespace Pol

#endif
