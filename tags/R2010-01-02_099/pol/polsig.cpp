/*
History
=======
2005/09/16 Shinigami: scripts_thread_script* moved to passert

Notes
=======

*/

#include "../clib/stl_inc.h"

#include "../clib/esignal.h"
#include "../clib/threadhelp.h"

#include "../bscript/eprog.h"

#include "polsig.h"
volatile bool reload_configuration_signalled = false;
unsigned scripts_thread_checkpoint;
unsigned tasks_thread_checkpoint;
unsigned active_client_thread_checkpoint;
unsigned check_attack_after_move_function_checkpoint;
volatile bool report_status_signalled = false;

ref_ptr<EScriptProgram> scripts_thread_last_script;
#ifdef _WIN32

void install_signal_handlers()
{
    enable_exit_signaller();
}
void signal_catch_thread()
{
}
#else

#include <signal.h>

pthread_t main_pthread;


void sigpipe_handler( int x )
{
    cout << "SIGPIPE! param=" << x << endl;
}

void handle_HUP( int x )
{
    reload_configuration_signalled = true;
}

void handle_SIGUSR1( int x )//LINUXTEST
{
    report_status_signalled = true;
}

void null_handler( int x )
{
}

void install_null_handler( int sig )
{
    struct sigaction sa_null;
    sa_null.sa_handler = null_handler;
    sigemptyset( &sa_null.sa_mask );
    sa_null.sa_flags = 0;
    if (sigaction(sig, &sa_null, NULL) == -1)
    {
        perror("sigaction failed");
        exit(1);
    }
}

void install_signal_handlers()
{
    install_null_handler( SIGPIPE );
    
    install_null_handler( SIGHUP );
    install_null_handler( SIGUSR1 );
    install_null_handler( SIGINT );
    install_null_handler( SIGQUIT );
    install_null_handler( SIGTERM );

    sigset_t blocked_sigs;
    sigemptyset( &blocked_sigs );
    sigfillset( &blocked_sigs );
    sigdelset( &blocked_sigs, SIGSEGV );
    sigdelset( &blocked_sigs, SIGUSR2 );

    int res = pthread_sigmask( SIG_BLOCK, &blocked_sigs, NULL );
    if (res)
    {
        cerr << "pthread_sigmask failed: " << res << endl;
        exit(1);
    }

    //main_pid = getpid();
    main_pthread = pthread_self();
}

void signal_catch_thread()
{
    int res = pthread_kill( main_pthread, SIGTERM );
    if (res)
    {
        cerr << "pthread_kill failed: " << res << endl;
    }
}

void catch_signals_thread( void )
{
    sigset_t expected_signals;
    sigemptyset( &expected_signals );
    sigaddset( &expected_signals, SIGHUP ); 
    sigaddset( &expected_signals, SIGUSR1 ); 
    sigaddset( &expected_signals, SIGINT );
    sigaddset( &expected_signals, SIGQUIT );
    sigaddset( &expected_signals, SIGTERM );

    //cerr << "catch_signals_thread running" << endl;
    // this thread should be the second last out, so it can still respond to SIGUSR1
    // this is called from main(), so it can exit when there is one thread left
    // (that will be the thread_status thread)
    while (!exit_signalled || threadhelp::child_threads>1)
    {
        int caught = 0;
        sigwait( &expected_signals, &caught );
        switch( caught )
        {
            case SIGHUP: 
                cerr << "SIGHUP: reload configuration." << endl;
                reload_configuration_signalled = true;
                break;
            case SIGUSR1: 
                cerr << "SIGUSR1: report thread status." << endl;
                report_status_signalled = true;
                break;
            case SIGINT: 
                cerr << "SIGINT: exit." << endl;
                exit_signalled = true;
                break;
            case SIGQUIT: 
                cerr << "SIGQUIT: exit." << endl;
                exit_signalled = true;
                break;
            case SIGTERM: 
                cerr << "SIGTERM: exit." << endl;
                exit_signalled = true;
                break;
            default: 
                cerr << "Unexpected signal: " << caught << endl;
                break;
        }
    }
    //cerr << "catch_signals thread exits" << endl;
}

#endif
