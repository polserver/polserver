/*
History
=======

Notes
=======

*/

#include "stl_inc.h"
#include <iostream>

#include "esignal.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <signal.h>
#endif
namespace Pol {
  namespace Clib {
	volatile bool exit_signalled = false;
#ifdef _WIN32
	BOOL WINAPI control_handler( DWORD dwCtrlType )
	{
	  switch( dwCtrlType )
	  {
		case CTRL_BREAK_EVENT:  // use Ctrl+C or Ctrl+Break to simulate
		case CTRL_C_EVENT:      // SERVICE_CONTROL_STOP in debug mode
		  std::cerr << "Ctrl-C detected.  Stopping..." << std::endl;
		  exit_signalled = true;
		  return TRUE;

		  // Consider these three new signals?
#if 0
		case CTRL_CLOSE_EVENT:		// console window closing
		  std::cerr << "Console window closing. Stopping..." << std::endl;
		  exit_signalled = true;
		  return TRUE;

		case CTRL_LOGOFF_EVENT:		// User logging off system
		  std::cerr << "User logging off system. Stopping..." << std::endl;
		  exit_signalled = true;
		  return TRUE;

		case CTRL_SHUTDOWN_EVENT:	// User shutting down system
		  std::cerr << "System shutting down. Stopping..." << std::endl;
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