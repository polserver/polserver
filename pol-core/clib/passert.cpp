/*
History
=======
2005/01/17 Shinigami: passert_failed extended by param "reason" (see passert*_r macros)
2005/09/16 Shinigami: scripts_thread_script* moved to passert
2006/11/26 Shinigami: fixed a crash in passert_failed

Notes
=======

*/

#include "stl_inc.h"

#include "esignal.h"
#include "logfacility.h"
#include "passert.h"
#include "progver.h"
#include "stlutil.h"
#include "threadhelp.h"
#include <cstring>

#ifdef _WIN32
#	include <windows.h>
#	include "mdumpimp.h"    
#else
#include <signal.h>
#include <unistd.h>
#include <execinfo.h>
#endif

namespace Pol {
  namespace Clib {
	bool passert_disabled = true;
	bool passert_dump_stack = false;
	bool passert_shutdown = false;
	bool passert_abort = true;
	bool passert_nosave = false;
	bool passert_shutdown_due_to_assertion = false;

	string scripts_thread_script;
	unsigned scripts_thread_scriptPC;

#ifdef _WIN32    
	void force_backtrace()
	{
	  __try
	  {
		int* p = 0;
		*p = 7;  // dump the stack
	  }
	  __except( ( HiddenMiniDumper::TopLevelFilter( GetExceptionInformation() ) ), EXCEPTION_EXECUTE_HANDLER )
	  {}
	}
#else

	void force_backtrace()
	{
      fmt::Writer tmp;
      tmp << "=== Stack Backtrace ===\nBuild: " << progverstr << " (" << buildtagstr << ")\nStack Backtrace:\n";
      void* bt[ 200 ];
      char **strings;
      int n = backtrace( bt, 200 );
      strings = backtrace_symbols(bt, n);
      for (int j = 0; j < n; j++)
        tmp << strings[j] << "\n";

      free( strings );

      tmp << "=======================\n";
      POLLOG_ERROR << tmp.c_str();
      if ( Clib::Logging::global_logger )
        Clib::Logging::global_logger->wait_for_empty_queue( ); // wait here for logging facility to make sure everything was printed
	}
#endif

	void passert_failed( const char *expr, const char *file, unsigned line )
	{
	  POLLOG_ERROR << "Assertion Failed: " << expr << ", " << file << ", line " << line << "\n";

	  if( passert_dump_stack )
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

	  if( passert_shutdown )
	  {
        POLLOG_ERROR << "Shutting down due to assertion failure.\n";
		exit_signalled = true;
		passert_shutdown_due_to_assertion = true;
	  }
	  if( passert_abort )
	  {
        POLLOG_ERROR << "Aborting due to assertion failure.\n";
		abort();
	  }

	  throw runtime_error( "Assertion Failed: "
						   + string( expr ) + ", "
						   + string( file ) + ", line "
						   + tostring( line ) );
	}

	void passert_failed( const char *expr, const string& reason, const char *file, unsigned line )
	{
      POLLOG_ERROR << "Assertion Failed: " << expr << " (" << reason << "), " << file << ", line " << line << "\n";

	  if( passert_dump_stack )
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

	  if( passert_shutdown )
	  {
        POLLOG_ERROR << "Shutting down due to assertion failure.\n";
		exit_signalled = true;
		passert_shutdown_due_to_assertion = true;
	  }
	  if( passert_abort )
	  {
        POLLOG_ERROR << "Aborting due to assertion failure.\n";
		abort();
	  }

	  throw runtime_error( "Assertion Failed: "
						   + string( expr ) + " ("
						   + string( reason ) + "), "
						   + string( file ) + ", line "
						   + tostring( line ) );
	}
  }
}