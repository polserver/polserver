/** @file
 *
 * @par History
 * - 2005/01/17 Shinigami: passert_failed extended by param "reason" (see passert*_r macros)
 * - 2005/09/16 Shinigami: scripts_thread_script* moved to passert
 * - 2006/11/26 Shinigami: fixed a crash in passert_failed
 */


#include "passert.h"

#include <format/format.h>

#include "Debugging/ExceptionParser.h"
#include "esignal.h"
#include "logfacility.h"
#include "stlutil.h"

#ifdef WINDOWS
#include "Header_Windows.h"
#include "mdumpimp.h"
#else
#include <cstdlib>
#endif

namespace Pol
{
namespace Clib
{
bool passert_dump_stack = false;
bool passert_shutdown = false;
bool passert_abort = true;
bool passert_nosave = false;
bool passert_shutdown_due_to_assertion = false;

std::string scripts_thread_script;
unsigned scripts_thread_scriptPC;

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
  fmt::Writer tmp;
  tmp << "=== Stack Backtrace ===\n" << stack_trace;

  POLLOG_ERROR << tmp.str() << "\n";
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
  if ( reason != "" )
    POLLOG_ERROR << "Assertion Failed: " << expr << " (" << reason << "), " << file << ", line "
                 << line << "\n";
  else
    POLLOG_ERROR << "Assertion Failed: " << expr << ", " << file << ", line " << line << "\n";

  if ( passert_dump_stack )
  {
    POLLOG_ERROR << "Forcing stack backtrace.\n";
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
    if ( sprintf( reportedReason, "ASSERT(%s, reason: \"%s\") failed in %s:%d", expr,
                  reason.c_str(), file, line ) > 0 )
      ExceptionParser::reportProgramAbort( ExceptionParser::getTrace(),
                                           std::string( reportedReason ) );
    else
      ExceptionParser::reportProgramAbort( ExceptionParser::getTrace(), "ASSERT failed" );
  }


  if ( passert_shutdown )
  {
    POLLOG_ERROR << "Shutting down due to assertion failure.\n";
    exit_signalled = true;
    passert_shutdown_due_to_assertion = true;
  }
  if ( passert_abort )
  {
    POLLOG_ERROR << "Aborting due to assertion failure.\n";
    abort();
  }

  if ( reason != "" )
  {
    throw std::runtime_error( "Assertion Failed: " + std::string( expr ) + " (" +
                              std::string( reason ) + "), " + std::string( file ) + ", line " +
                              tostring( line ) );
  }
  else
  {
    throw std::runtime_error( "Assertion Failed: " + std::string( expr ) + ", " +
                              std::string( file ) + ", line " + tostring( line ) );
  }
}
}
}
