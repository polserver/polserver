/** @file
 *
 * @par History
 * - 2005/01/17 Shinigami: passert_failed extended by param "reason" (see passert*_r macros)
 * - 2005/09/16 Shinigami: scripts_thread_script* moved to passert
 * - 2006/11/26 Shinigami: fixed a crash in passert_failed
 */


#include "clib/passert.h"

#include "clib/Debugging/ExceptionParser.h"
#include "clib/esignal.h"
#include "clib/logfacility.h"
#include "clib/stlutil.h"
#include "clib/strutil.h"
#include "clib/threadhelp.h"

#ifdef WINDOWS
#include "clib/Header_Windows.h"
#include "clib/mdumpimp.h"
#else
#include <cstdlib>
#endif


namespace Pol::Clib
{
bool passert_dump_stack = false;
bool passert_shutdown = false;
bool passert_abort = true;
bool passert_nosave = false;
bool passert_shutdown_due_to_assertion = false;

#ifdef _WIN32
void force_backtrace( bool /*complete*/ )
{
  __try
  {
    int* p = 0;
    *p = 7;  // dump the stack
  }
  __except ( ( HiddenMiniDumper::TopLevelFilter( GetExceptionInformation() ) ),
             EXCEPTION_EXECUTE_HANDLER )
  {
  }
}
#else
void force_backtrace( bool complete )
{
  std::string stack_trace = Clib::ExceptionParser::getTrace();
  POLLOG_ERRORLN( "=== Stack Backtrace ({}) ===\n{}", threadhelp::current_thread_name(),
                  stack_trace );
  if ( complete )
    ExceptionParser::logAllStackTraces();
}
#endif

void passert_failed( const char* expr, const char* file, unsigned line )
{
  passert_failed( expr, "", file, line );
}

void passert_failed( const char* expr, const std::string& reason, const char* file, unsigned line )
{
  // This line is the one that ends up quoted in bug reports, so it carries the
  // thread itself rather than leaving it to the backtrace below -- which is not
  // always printed, and on Windows goes to a separate minidump.
  if ( !reason.empty() )
    POLLOG_ERRORLN( "Assertion Failed in {}: {} ({}), {}, line {}",
                    threadhelp::current_thread_name(), expr, reason, file, line );
  else
    POLLOG_ERRORLN( "Assertion Failed in {}: {}, {}, line {}", threadhelp::current_thread_name(),
                    expr, file, line );

  if ( passert_dump_stack )
  {
    POLLOG_ERRORLN( "Forcing stack backtrace." );
    force_backtrace();
  }
  else
  {
#ifdef _WIN32
    HiddenMiniDumper::print_backtrace();
#endif
  }

  /**
   * use the program abort reporting system
   */
  if ( Pol::Clib::ExceptionParser::programAbortReporting() )
  {
    char reportedReason[512];
    if ( snprintf( reportedReason, arsize( reportedReason ),
                   "ASSERT(%s, reason: \"%s\") failed in %s:%d", expr, reason.c_str(), file,
                   line ) > 0 )
      ExceptionParser::reportProgramAbort( ExceptionParser::getTrace(),
                                           std::string( reportedReason ) );
    else
      ExceptionParser::reportProgramAbort( ExceptionParser::getTrace(), "ASSERT failed" );
  }


  if ( passert_shutdown )
  {
    POLLOG_ERRORLN( "Shutting down due to assertion failure." );
    Clib::signal_exit( 1 );
    passert_shutdown_due_to_assertion = true;
  }
  if ( passert_abort )
  {
    POLLOG_ERRORLN( "Aborting due to assertion failure." );
    Logging::global_logger->wait_for_empty_queue();
    abort();
  }

  if ( !reason.empty() )
  {
    throw std::runtime_error( "Assertion Failed: " + std::string( expr ) + " (" +
                              std::string( reason ) + "), " + std::string( file ) + ", line " +
                              tostring( line ) );
  }

  throw std::runtime_error( "Assertion Failed: " + std::string( expr ) + ", " +
                            std::string( file ) + ", line " + tostring( line ) );
}
}  // namespace Pol::Clib
