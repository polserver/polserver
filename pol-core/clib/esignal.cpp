/** @file
 *
 * @par History
 */


#include "esignal.h"
#ifdef WINDOWS
#include "Header_Windows.h"
#include "logfacility.h"
#else
#include <signal.h>
#endif
namespace Pol
{
namespace Clib
{
std::atomic<bool> exit_signalled( false );
std::atomic<int> exit_code( 0 );

void signal_exit( int code )
{
  if ( exit_code == 0 )  // do not allow to "fix" the error code
    exit_code = code;
  exit_signalled = true;
}

#ifdef WINDOWS
BOOL WINAPI control_handler( DWORD dwCtrlType )
{
  switch ( dwCtrlType )
  {
  case CTRL_BREAK_EVENT:  // use Ctrl+C or Ctrl+Break to simulate
  case CTRL_C_EVENT:      // SERVICE_CONTROL_STOP in debug mode
    ERROR_PRINT << "Ctrl-C detected.  Stopping...\n";
    signal_exit();
    return TRUE;

// Consider these three new signals?
#if 0
    case CTRL_CLOSE_EVENT:    // console window closing
          ERROR_PRINT << "Console window closing. Stopping...\n";
      signal_exit();
      return TRUE;

    case CTRL_LOGOFF_EVENT:    // User logging off system
          ERROR_PRINT << "User logging off system. Stopping...\n";
      signal_exit();
      return TRUE;

    case CTRL_SHUTDOWN_EVENT:  // User shutting down system
          ERROR_PRINT << "System shutting down. Stopping...\n";
      signal_exit();
      return TRUE;
#endif
  }
  return FALSE;
}

void enable_exit_signaller()
{
  SetConsoleCtrlHandler( control_handler, TRUE );
}

#else


void ctrlc_handler( int x )
{
  (void)x;
  signal_exit();
}
void enable_exit_signaller()
{
  signal( SIGINT, ctrlc_handler );
  signal( SIGQUIT, ctrlc_handler );
  signal( SIGTERM, ctrlc_handler );
}
#endif
}  // namespace Clib
}  // namespace Pol
