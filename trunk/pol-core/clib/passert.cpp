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
#include "logfile.h"
#include "passert.h"
#include "progver.h"
#include "stlutil.h"
#include "threadhelp.h"

#ifdef _WIN32
#include <windows.h>
#include "mdumpimp.h"    
#endif

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
    __except( (HiddenMiniDumper::TopLevelFilter(GetExceptionInformation())), EXCEPTION_EXECUTE_HANDLER )
    {
    }
}
#else
#include <signal.h>
#include <unistd.h>
#include <execinfo.h>
void force_backtrace()
{
#define WRITE_STRING( x ) write( STDERR_FILENO, x, strlen(x) )

    WRITE_STRING( "=== Stack Backtrace ===\n" );

    WRITE_STRING( "Build: " );
    WRITE_STRING( progverstr );
    WRITE_STRING( " (" ); WRITE_STRING( buildtagstr ); WRITE_STRING( ")\n" );
    WRITE_STRING( "Stack Backtrace:\n" );

    void* bt[ 200 ];
    int n = backtrace( bt, 200 );
    backtrace_symbols_fd( bt, n, STDERR_FILENO );

    WRITE_STRING( "=======================\n" );

}
#endif

void passert_failed(const char *expr, const char *file, unsigned line)
{
    Log( "Assertion Failed: %s, %s, line %d\n", expr, file, line );
    cerr << "Assertion Failed: " << expr << ", " << file << ", line " << line << endl;

    if (passert_dump_stack)
    {
        Log( "Forcing stack backtrace.\n" );
        cerr << "Forcing stack backtrace." << endl;

        force_backtrace();
    }

    if (passert_shutdown)
    {
        Log( "Shutting down due to assertion failure.\n" );
        cerr << "Shutting down due to assertion failure." << endl;
        exit_signalled = true;
        passert_shutdown_due_to_assertion = true;
    }
    if (passert_abort)
    {
        Log( "Aborting due to assertion failure.\n" );
        cerr << "Aborting due to assertion failure." << endl;
        abort();
    }

    throw runtime_error( "Assertion Failed: "
                            + string(expr) + ", "
                            + string(file) + ", line "
                            + tostring(line) );
}

void passert_failed(const char *expr, const string& reason, const char *file, unsigned line)
{
    Log( "Assertion Failed: %s (%s), %s, line %d\n", expr, reason.c_str(), file, line );
    cerr << "Assertion Failed: " << expr << " (" << reason << "), " << file << ", line " << line << endl;

    if (passert_dump_stack)
    {
        Log( "Forcing stack backtrace.\n" );
        cerr << "Forcing stack backtrace." << endl;

        force_backtrace();
    }

    if (passert_shutdown)
    {
        Log( "Shutting down due to assertion failure.\n" );
        cerr << "Shutting down due to assertion failure." << endl;
        exit_signalled = true;
        passert_shutdown_due_to_assertion = true;
    }
    if (passert_abort)
    {
        Log( "Aborting due to assertion failure.\n" );
        cerr << "Aborting due to assertion failure." << endl;
        abort();
    }

    throw runtime_error( "Assertion Failed: "
                            + string(expr) + " ("
                            + string(reason) + "), "
                            + string(file) + ", line "
                            + tostring(line) );
}
