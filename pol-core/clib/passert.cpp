/*
History
=======
2005/01/17 Shinigami: passert_failed extended by param "reason" (see passert*_r macros)
2005/09/16 Shinigami: scripts_thread_script* moved to passert
2006/11/26 Shinigami: fixed a crash in passert_failed

Notes
=======

*/

#include "passert.h"

#include "../plib/systemstate.h"
#include "Debugging/ExceptionParser.h"

#include "esignal.h"
#include "logfacility.h"
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
#include <iostream>
#include <cstdlib>
#include <signal.h>
#include <pthread.h>
#endif

namespace Pol {
  namespace Clib {
	bool passert_disabled = true;
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
	  __except( ( HiddenMiniDumper::TopLevelFilter( GetExceptionInformation() ) ), EXCEPTION_EXECUTE_HANDLER )
	  {}
	}
#else
    void GetCallstack( pthread_t threadId );

    void force_backtrace(bool complete)
	{
      std::string stack_trace = Clib::ExceptionParser::getTrace();
      fmt::Writer tmp;
      tmp << "=== Stack Backtrace ===\nBuild: " << progverstr << " (" << buildtagstr << ")\nStack Backtrace:\n";
	  tmp << stack_trace;
	  
      POLLOG_ERROR << tmp.c_str() << "\n";
      if (complete)
      {
        threadhelp::ThreadMap::Contents contents;
        threadhelp::threadmap.CopyContents( contents );
        POLLOG_ERROR << "thread size " << contents.size() << "\n";
        for ( const auto& threads : contents)
        {
          GetCallstack((pthread_t)threads.first);
        }
      }
      if ( Clib::Logging::global_logger )
        Clib::Logging::global_logger->wait_for_empty_queue( ); // wait here for logging facility to make sure everything was printed
      POLLOG_ERROR << "=======================\n";
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

      /**
       * use the program abort reporting system
       */
      if(Plib::systemstate.config.report_program_aborts)
      {
    	  char reason[512];
    	  if(sprintf(reason, "ASSERT(%s) failed in %s:%d", expr, file, line) > 0)
    		  ExceptionParser::reportProgramAbort(ExceptionParser::getTrace(), std::string(reason));
    	  else
    		  ExceptionParser::reportProgramAbort(ExceptionParser::getTrace(), "ASSERT failed");
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

	  throw std::runtime_error( "Assertion Failed: "
						   + std::string( expr ) + ", "
                           + std::string(file) + ", line "
						   + tostring( line ) );
	}

    void passert_failed(const char *expr, const std::string& reason, const char *file, unsigned line)
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

      throw std::runtime_error("Assertion Failed: "
          + std::string(expr) + " ("
          + std::string(reason) + "), "
          + std::string(file) + ", line "
          + tostring(line));
	}


#ifndef _WIN32 
  #define CALLSTACK_SIG SIGUSR1

    static pthread_t callingThread = 0;
    static pthread_t targetThread = 0;

    static void _callstack_signal_handler( int signr, siginfo_t *info, void *context )
    {
      if ( pthread_self() != targetThread )
        return;

      fmt::Writer tmp;
      std::string stack_trace = Clib::ExceptionParser::getTrace();
      threadhelp::ThreadMap::Contents contents;
      threadhelp::threadmap.CopyContents( contents );
      tmp << "Thread ID " << pthread_self() << " (" << contents[pthread_self()] << ")\n";
	  tmp << stack_trace;

      POLLOG_ERROR << tmp.c_str() << "\n";
      if ( Clib::Logging::global_logger )
        Clib::Logging::global_logger->wait_for_empty_queue(); // wait for finish
    }

    static void _setup_callstack_signal_handler()
    {
      struct sigaction sa;
      sigfillset( &sa.sa_mask );
      sa.sa_flags = SA_SIGINFO;
      sa.sa_sigaction = _callstack_signal_handler;
      sigaction( CALLSTACK_SIG, &sa, NULL );
    }
    static std::mutex callstack_mutex;
    void GetCallstack( pthread_t threadId )
    {
      std::lock_guard<std::mutex> lock( callstack_mutex );
      callingThread = pthread_self();
      targetThread = threadId;
      if ( callingThread == targetThread )
      {
        return;
      }
      _setup_callstack_signal_handler();
      // call _callstack_signal_handler in target thread
      if ( pthread_kill( threadId, CALLSTACK_SIG ) != 0 )
      {
        POLLOG_ERROR << "kill failed\n";
        return;
      }
    }
#endif
  }
}
