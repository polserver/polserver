/*
History
=======

Notes
=======

*/


#include "esignal.h"
#include "logfacility.h"

#ifdef WINDOWS
#include <Header_Windows.h>
#else
#include <signal.h>
#endif
namespace Pol {
  namespace Clib {
	
      std::atomic<bool> exit_signalled(false);

#ifdef WINDOWS
	BOOL WINAPI control_handler( DWORD dwCtrlType )
	{
	  switch( dwCtrlType )
	  {
		case CTRL_BREAK_EVENT:  // use Ctrl+C or Ctrl+Break to simulate
		case CTRL_C_EVENT:      // SERVICE_CONTROL_STOP in debug mode
		  ERROR_PRINT << "Ctrl-C detected.  Stopping...\n";
		  exit_signalled = true;
		  return TRUE;

		  // Consider these three new signals?
#if 0
		case CTRL_CLOSE_EVENT:		// console window closing
          ERROR_PRINT << "Console window closing. Stopping...\n";
		  exit_signalled = true;
		  return TRUE;

		case CTRL_LOGOFF_EVENT:		// User logging off system
          ERROR_PRINT << "User logging off system. Stopping...\n";
		  exit_signalled = true;
		  return TRUE;

		case CTRL_SHUTDOWN_EVENT:	// User shutting down system
          ERROR_PRINT << "System shutting down. Stopping...\n";
		  exit_signalled = true;
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
  exit_signalled = true;
}
void enable_exit_signaller()
{
  signal( SIGINT, ctrlc_handler );
  signal( SIGQUIT, ctrlc_handler );
  signal( SIGTERM, ctrlc_handler );
}
#endif
}
}
