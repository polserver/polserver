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


#ifdef _WIN32
#include <windows.h>
#include <eh.h>
#include <stdio.h>

#include "logfile.h"
#include "strexcpt.h"
#include "msjexhnd.h"

#ifndef _M_X64
void se_trans_func( unsigned int n, EXCEPTION_POINTERS *ex )
{
    printf( "Structured Exception Detected: 0x%x\n", n );
    printf( "ExceptionRecord:\n" );
    printf( "  Exception Code:    0x%lx\n", ex->ExceptionRecord->ExceptionCode );
    printf( "  Exception Flags:   0x%lx\n", ex->ExceptionRecord->ExceptionFlags );
    printf( "  Exception Record:  0x%lx\n", ex->ExceptionRecord->ExceptionRecord );
    printf( "  Exception Address: 0x%lx\n", ex->ExceptionRecord->ExceptionAddress );
    printf( "  NumberParameters:  0x%lx\n", ex->ExceptionRecord->NumberParameters );

            Log( "Structured Exception Detected: 0x%x\n", n );
            Log( "ExceptionRecord:\n" );
            Log( "  Exception Code:    0x%lx\n", ex->ExceptionRecord->ExceptionCode );
            Log( "  Exception Flags:   0x%lx\n", ex->ExceptionRecord->ExceptionFlags );
            Log( "  Exception Record:  0x%lx\n", ex->ExceptionRecord->ExceptionRecord );
            Log( "  Exception Address: 0x%lx\n", ex->ExceptionRecord->ExceptionAddress );
            Log( "  NumberParameters:  0x%lx\n", ex->ExceptionRecord->NumberParameters );

    if (ex->ExceptionRecord->NumberParameters)
    {
        printf( "  Exception Information:" );
        for( DWORD i = 0; i < ex->ExceptionRecord->NumberParameters; i++)
        {
            printf( "%s0x%08.08x", 
                        ( (i&7)==0)?"\n  ":"  ", // print newline every 8 numbers
                        ex->ExceptionRecord->ExceptionInformation[i] );
        }
        printf( "\n" );
    }

            if (ex->ExceptionRecord->NumberParameters)
            {
                Log( "  Exception Information:" );
                for( DWORD i = 0; i < ex->ExceptionRecord->NumberParameters; i++)
                {
                    Log( "%s0x%08.08x", 
                                ( (i&7)==0)?"\n  ":"  ", // print newline every 8 numbers
                                ex->ExceptionRecord->ExceptionInformation[i] );
                }
                Log( "\n" );
            }

    
    printf( "ContextRecord:\n" );
    printf( "  ContextFlags: 0x%lx\n", ex->ContextRecord->ContextFlags );
            Log( "ContextRecord:\n" );
            Log( "  ContextFlags: 0x%lx\n", ex->ContextRecord->ContextFlags );
    if (ex->ContextRecord->ContextFlags & CONTEXT_DEBUG_REGISTERS)
    {
        printf( "  CONTEXT_DEBUG_REGISTERS:\n" );
        printf( "       Dr0: 0x%lx\n", ex->ContextRecord->Dr0 );
        printf( "       Dr1: 0x%lx\n", ex->ContextRecord->Dr1 );
        printf( "       Dr2: 0x%lx\n", ex->ContextRecord->Dr2 );
        printf( "       Dr3: 0x%lx\n", ex->ContextRecord->Dr3 );
        printf( "       Dr6: 0x%lx\n", ex->ContextRecord->Dr6 );
        printf( "       Dr7: 0x%lx\n", ex->ContextRecord->Dr7 );
                Log( "  CONTEXT_DEBUG_REGISTERS:\n" );
                Log( "       Dr0: 0x%lx\n", ex->ContextRecord->Dr0 );
                Log( "       Dr1: 0x%lx\n", ex->ContextRecord->Dr1 );
                Log( "       Dr2: 0x%lx\n", ex->ContextRecord->Dr2 );
                Log( "       Dr3: 0x%lx\n", ex->ContextRecord->Dr3 );
                Log( "       Dr6: 0x%lx\n", ex->ContextRecord->Dr6 );
                Log( "       Dr7: 0x%lx\n", ex->ContextRecord->Dr7 );
    }
    if (ex->ContextRecord->ContextFlags & CONTEXT_SEGMENTS)
    {
        printf( "  CONTEXT_SEGMENTS:\n" );
        printf( "     SegGs: 0x%lx\n", ex->ContextRecord->SegGs );
        printf( "     SegFs: 0x%lx\n", ex->ContextRecord->SegFs );
        printf( "     SegEs: 0x%lx\n", ex->ContextRecord->SegEs );
        printf( "     SegDs: 0x%lx\n", ex->ContextRecord->SegDs );
                Log( "  CONTEXT_SEGMENTS:\n" );
                Log( "     SegGs: 0x%lx\n", ex->ContextRecord->SegGs );
                Log( "     SegFs: 0x%lx\n", ex->ContextRecord->SegFs );
                Log( "     SegEs: 0x%lx\n", ex->ContextRecord->SegEs );
                Log( "     SegDs: 0x%lx\n", ex->ContextRecord->SegDs );
    }
    if (ex->ContextRecord->ContextFlags & CONTEXT_INTEGER)
    {
        printf( "  CONTEXT_INTEGER:\n" );
        printf( "       Edi: 0x%lx\n", ex->ContextRecord->Edi );
        printf( "       Esi: 0x%lx\n", ex->ContextRecord->Esi );
        printf( "       Ebx: 0x%lx\n", ex->ContextRecord->Ebx );
        printf( "       Edx: 0x%lx\n", ex->ContextRecord->Edx );
        printf( "       Ecx: 0x%lx\n", ex->ContextRecord->Ecx );
        printf( "       Eax: 0x%lx\n", ex->ContextRecord->Eax );
                Log( "  CONTEXT_INTEGER:\n" );
                Log( "       Edi: 0x%lx\n", ex->ContextRecord->Edi );
                Log( "       Esi: 0x%lx\n", ex->ContextRecord->Esi );
                Log( "       Ebx: 0x%lx\n", ex->ContextRecord->Ebx );
                Log( "       Edx: 0x%lx\n", ex->ContextRecord->Edx );
                Log( "       Ecx: 0x%lx\n", ex->ContextRecord->Ecx );
                Log( "       Eax: 0x%lx\n", ex->ContextRecord->Eax );
    }
    if (ex->ContextRecord->ContextFlags & CONTEXT_CONTROL)
    {
        printf( "  CONTEXT_CONTROL:\n" );
        printf( "       Ebp: 0x%lx\n", ex->ContextRecord->Ebp );
        printf( "       Eip: 0x%lx\n", ex->ContextRecord->Eip );
        printf( "     SegCs: 0x%lx\n", ex->ContextRecord->SegCs );
        printf( "    EFlags: 0x%lx\n", ex->ContextRecord->EFlags );
        printf( "       Esp: 0x%lx\n", ex->ContextRecord->Esp );
        printf( "     SegSs: 0x%lx\n", ex->ContextRecord->SegSs );
           Log( "  CONTEXT_CONTROL:\n" );
           Log( "       Ebp: 0x%lx\n", ex->ContextRecord->Ebp );
           Log( "       Eip: 0x%lx\n", ex->ContextRecord->Eip );
           Log( "     SegCs: 0x%lx\n", ex->ContextRecord->SegCs );
           Log( "    EFlags: 0x%lx\n", ex->ContextRecord->EFlags );
           Log( "       Esp: 0x%lx\n", ex->ContextRecord->Esp );
           Log( "     SegSs: 0x%lx\n", ex->ContextRecord->SegSs );
        
        printf( "    Stack Backtrace:\n" );
           Log( "    Stack Backtrace:\n" );
        //int frames = 3;
        DWORD *ebp = (DWORD *) ex->ContextRecord->Ebp;
        DWORD eip = ex->ContextRecord->Eip;
        DWORD *new_ebp;
        DWORD new_eip;


        printf( "    EIP        RETADDR\n" );
           Log( "    EIP        RETADDR\n" );
        int levelsleft = 100;
        while (ebp && levelsleft--)
        {
            printf( "    0x%08.08lx", eip );
               Log( "    0x%08.08lx", eip );
            
            if (!IsBadReadPtr( ebp, sizeof *ebp ))
            {
                new_ebp = (DWORD *) *ebp;
            }
            else
            {
                printf( "[Can't read stack!]\n" );
                   Log( "[Can't read stack!]\n" );
                break;

            }
            if (!IsBadReadPtr( ebp+1, sizeof *ebp ))
            {
                printf( "    0x%08.08lx", *(ebp+1) );
                   Log( "    0x%08.08lx", *(ebp+1) );
                new_eip = *(ebp+1);
            }
            else
            {
                printf( "[Can't read stack!]\n" );
                   Log( "[Can't read stack!]\n" );
                break;

            }

            ebp = new_ebp;
            eip = new_eip;
            printf( "\n" );
               Log( "\n" );
        }

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
    printf( "In trans_func.\n" ); 
    if (in_ex_handler)
    {
        printf( "recursive structured exception\n" );
        Log( "recursive structured exception\n" );
    }
    else
    {
        in_ex_handler = true;
        Log( "Structured exception in %s compiled on %s at %s\n",
             progverstr,
             compiledate,
             compiletime );
              
        MSJExceptionHandler::MSJUnhandledExceptionFilter( pExp );
        in_ex_handler = false;
    }        
    throw structured_exception( u );
}

void InstallStructuredExceptionHandler(void)
{
    static bool init = false;
    if (!init)
    {
        init = true;
    }
    if (!IsDebuggerPresent())
    {
        // if there's a debugger, better to let it take control
        // homegrown: _set_se_translator( se_trans_func );
		// _set_se_translator( alt_se_trans_func );
    }
}

void InstallOldStructuredExceptionHandler(void)
{
    if (!IsDebuggerPresent())
    {
		_set_se_translator( alt_se_trans_func );
	}
}

#else

#include <signal.h>
#include <unistd.h>
#include <execinfo.h>
#include "logfile.h"
#include "passert.h"

void segv_handler(int signal)
{
    if (signal == SIGSEGV)
	{
        Log( "Caught SIGSEGV (Segfault).  Please post the following on http://forums.polserver.com/tracker.php :\n" );
        cerr << "Caught SIGSEGV (Segfault).  Please post the following on http://forums.polserver.com/tracker.php :" << endl;
	}
    else
	{
        Log( "Caught SIGUSR2 (On-demand backtrace).  Please post the following with explanation and last lines from log files on http://forums.polserver.com/tracker.php :\n" );
        cerr << "Caught SIGUSR2 (On-demand backtrace).  Please post the following with explanation and last lines from log files on http://forums.polserver.com/tracker.php :" << endl;
	}
	Log( "=== CUT ===\n" );
	Log( "Build: %s (%s)\n", progverstr, buildtagstr );
	Log( "Last Script: %s PC: %d\n", scripts_thread_script.c_str(), scripts_thread_scriptPC );
    Log( "Stack Backtrace:\n" );
	cerr << "=== CUT ===" << endl;
	cerr << "Build: " << progverstr << " (" << buildtagstr << ")" << endl;
	cerr << "Last Script: " << scripts_thread_script << " PC: " << scripts_thread_scriptPC << endl;
    cerr << "Stack Backtrace:" << endl;

    void* bt[ 200 ];
    int n = backtrace( bt, 200 );
	char** strings = backtrace_symbols( bt, n );
	for (int i = 0; i < n; i++)
	{
		Log( "%s\n", strings[ i ] );
		cerr << strings[ i ] << endl;
	}
	free( strings );

	Log( "=== CUT ===\n" );
    cerr << "=== CUT ===" << endl;

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
