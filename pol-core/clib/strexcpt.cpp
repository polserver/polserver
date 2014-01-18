/*
History
=======
2005/09/16 Shinigami: added scripts_thread_script* to support better debugging
						backtrace will be printed in pol.log too
2009-07-18 MuadDib: Updated sigfault messages to point to bug tracker rather than mail to old yahoo groups.

Notes
=======

*/

#include "stl_inc.h"
#include "progver.h"
#include "logfacility.h"


#ifdef _WIN32
#include <windows.h>
#include <eh.h>
#include <stdio.h>
#include "strexcpt.h"
#include "msjexhnd.h"
#else
#include <signal.h>
#include <unistd.h>
#include <execinfo.h>
#include "passert.h"
#endif

namespace Pol {
  namespace Clib {
#ifdef _WIN32
#ifndef _M_X64
	void se_trans_func( unsigned int n, EXCEPTION_POINTERS *ex )
	{
      fmt::Writer tmp;
      tmp << "Structured Exception Detected: 0x" << fmt::hex(n) << "\n"
        << "ExceptionRecord:\n"
        << "  Exception Code:    0x" << fmt::hex(ex->ExceptionRecord->ExceptionCode )<<"\n"
        << "  Exception Flags:   0x" << fmt::hex(ex->ExceptionRecord->ExceptionFlags)<<"\n"
        << "  Exception Record:  0x" << fmt::hex((long long)(reinterpret_cast<const void*>(ex->ExceptionRecord->ExceptionRecord)))<<"\n"
        << "  Exception Address: 0x" << fmt::hex( ( long long )( reinterpret_cast<const void*>( ex->ExceptionRecord->ExceptionAddress )) ) << "\n"
        << "  NumberParameters:  0x" << fmt::hex(ex->ExceptionRecord->NumberParameters) <<"\n";

	  if (ex->ExceptionRecord->NumberParameters)
	  {
        tmp<< "  Exception Information:";
		for( DWORD i = 0; i < ex->ExceptionRecord->NumberParameters; i++)
		{
          fmt::Writer _tmp;
          _tmp.Format( "{}{:#X}" ) << ( ( ( i & 7 ) == 0 ) ? "\n  " : "  " ) // print newline every 8 numbers
            << ex->ExceptionRecord->ExceptionInformation[i];
          tmp << _tmp.c_str();
		}
        tmp << "\n";
	  }
      POLLOG_INFO << tmp.c_str();


      POLLOG_INFO << "ContextRecord:\n"
        << "  ContextFlags: 0x" << fmt::hex( ex->ContextRecord->ContextFlags ) << "\n";
	  if (ex->ContextRecord->ContextFlags & CONTEXT_DEBUG_REGISTERS)
	  {
        POLLOG_INFO << "  CONTEXT_DEBUG_REGISTERS:\n"
          << "       Dr0: 0x" << fmt::hex( ex->ContextRecord->Dr0 ) << "\n"
          << "       Dr1: 0x" << fmt::hex( ex->ContextRecord->Dr1 ) << "\n"
          << "       Dr2: 0x" << fmt::hex( ex->ContextRecord->Dr2 ) << "\n"
          << "       Dr3: 0x" << fmt::hex( ex->ContextRecord->Dr3 ) << "\n"
          << "       Dr6: 0x" << fmt::hex( ex->ContextRecord->Dr6 ) << "\n"
          << "       Dr7: 0x" << fmt::hex( ex->ContextRecord->Dr7 ) << "\n";
	  }
	  if (ex->ContextRecord->ContextFlags & CONTEXT_SEGMENTS)
	  {
        POLLOG_INFO << "  CONTEXT_SEGMENTS:\n"
          << "     SegGs: 0x" << fmt::hex( ex->ContextRecord->SegGs ) << "\n"
          << "     SegFs: 0x" << fmt::hex( ex->ContextRecord->SegFs ) << "\n"
          << "     SegEs: 0x" << fmt::hex( ex->ContextRecord->SegEs ) << "\n"
          << "     SegDs: 0x" << fmt::hex( ex->ContextRecord->SegDs ) << "\n";
	  }
	  if (ex->ContextRecord->ContextFlags & CONTEXT_INTEGER)
	  {
        POLLOG_INFO << "  CONTEXT_INTEGER:\n"
          << "       Edi: 0x" << fmt::hex( ex->ContextRecord->Edi ) << "\n"
          << "       Esi: 0x" << fmt::hex( ex->ContextRecord->Esi ) << "\n"
          << "       Ebx: 0x" << fmt::hex( ex->ContextRecord->Ebx ) << "\n"
          << "       Edx: 0x" << fmt::hex( ex->ContextRecord->Edx ) << "\n"
          << "       Ecx: 0x" << fmt::hex( ex->ContextRecord->Ecx ) << "\n"
          << "       Eax: 0x" << fmt::hex( ex->ContextRecord->Eax ) << "\n";
	  }
	  if (ex->ContextRecord->ContextFlags & CONTEXT_CONTROL)
	  {
        POLLOG_INFO << "  CONTEXT_CONTROL:\n"
          << "       Ebp: 0x" << fmt::hex( ex->ContextRecord->Ebp ) << "\n"
          << "       Eip: 0x" << fmt::hex( ex->ContextRecord->Eip ) << "\n"
          << "     SegCs: 0x" << fmt::hex( ex->ContextRecord->SegCs ) << "\n"
          << "    EFlags: 0x" << fmt::hex( ex->ContextRecord->EFlags ) << "\n"
          << "       Esp: 0x" << fmt::hex( ex->ContextRecord->Esp ) << "\n"
          << "     SegSs: 0x" << fmt::hex( ex->ContextRecord->SegSs ) << "\n";

        POLLOG_INFO << "    Stack Backtrace:\n";
		//int frames = 3;
		DWORD *ebp = (DWORD *) ex->ContextRecord->Ebp;
		DWORD eip = ex->ContextRecord->Eip;
		DWORD *new_ebp;
		DWORD new_eip;


        fmt::Writer tmp;
        tmp << "    EIP        RETADDR\n";
		int levelsleft = 100;
		while (ebp && levelsleft--)
		{
          tmp << "    0x" << fmt::hex( eip );

		  if (!IsBadReadPtr( ebp, sizeof *ebp ))
		  {
			new_ebp = (DWORD *) *ebp;
		  }
		  else
		  {
            tmp << "[Can't read stack!]\n";
			break;

		  }
		  if (!IsBadReadPtr( ebp+1, sizeof *ebp ))
		  {
            tmp << "    0x" << fmt::hex( *( ebp + 1 ) );
			new_eip = *(ebp+1);
		  }
		  else
		  {
            tmp << "[Can't read stack!]\n";
			break;

		  }

		  ebp = new_ebp;
		  eip = new_eip;
          tmp << "\n";
		}
        POLLOG_INFO << tmp.c_str();

	  }

	  if (n == EXCEPTION_ACCESS_VIOLATION)
	  {
		throw access_violation();
	  }
	  else
	  {
		throw structured_exception( n );
	  }
	}
#endif

	static bool in_ex_handler = false;
	void alt_se_trans_func( unsigned int u, _EXCEPTION_POINTERS* pExp )
	{
      INFO_PRINT << "In trans_func.\n";
	  if( in_ex_handler )
	  {
        POLLOG_INFO << "recursive structured exception\n";
	  }
	  else
	  {
		in_ex_handler = true;
        POLLOG_INFO.Format( "Structured exception in {} compiled on {} at {}\n" )
          << progverstr
          << compiledate
          << compiletime;

		MSJExceptionHandler::MSJUnhandledExceptionFilter( pExp );
		in_ex_handler = false;
	  }
	  throw structured_exception( u );
	}

	void InstallStructuredExceptionHandler( void )
	{
	  static bool init = false;
	  if( !init )
	  {
		init = true;
	  }
	  if( !IsDebuggerPresent() )
	  {
		// if there's a debugger, better to let it take control
		// homegrown: _set_se_translator( se_trans_func );
		// _set_se_translator( alt_se_trans_func );
	  }
	}

	void InstallOldStructuredExceptionHandler( void )
	{
	  if( !IsDebuggerPresent() )
	  {
		_set_se_translator( alt_se_trans_func );
	  }
	}

#else


void segv_handler(int signal)
{
  if (signal == SIGSEGV)
  {
    POLLOG_ERROR << "Caught SIGSEGV (Segfault).  Please post the following on http://forums.polserver.com/tracker.php :\n";
  }
  else
  {
    POLLOG_ERROR << "Caught SIGUSR2 (On-demand backtrace).  Please post the following with explanation and last lines from log files on http://forums.polserver.com/tracker.php :\n";
  }
  fmt::Writer tmp;
  tmp << "=== CUT ===\n";
  tmp << "Build: " << progverstr << " (" << buildtagstr << ")\n";
  tmp << "Last Script: " << scripts_thread_script << " PC: " << scripts_thread_scriptPC << "\n";
  tmp << "Stack Backtrace:\n";

  void* bt[ 200 ];
  int n = backtrace( bt, 200 );
  char** strings = backtrace_symbols( bt, n );
  for (int i = 0; i < n; i++)
  {
    tmp << strings[ i ] << "\n";
  }
  free( strings );

  tmp << "=== CUT ===\n";
  POLLOG_ERROR << tmp.c_str();

  if (signal == SIGSEGV)
  {
	struct sigaction sa;
	// Pass on the signal (so that a core file is produced)
	// FIXME: this doesn't seem to work - just outputs 'Killed'
	sa.sa_handler = SIG_DFL;
	sigemptyset (&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction (signal, &sa, NULL);
	raise (signal);
  }
}
void InstallStructuredExceptionHandler(void)
{
  struct sigaction sa;
  sa.sa_handler = segv_handler;
  sigemptyset (&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGSEGV, &sa, NULL);

  sigaction(SIGUSR2, &sa, NULL );
}
#endif
}
}