/** @file
 *
 * @par History
 * - 2005/09/16 Shinigami: scripts_thread_script* moved to passert
 */


#include "polsig.h"

#include <stdio.h>
#include <stdlib.h>

#include "../clib/esignal.h"
#include "../clib/logfacility.h"
#include "../clib/threadhelp.h"
#include "globals/state.h"

#ifndef _WIN32
#include <signal.h>
#endif

namespace Pol
{
namespace Core
{
PolSig::PolSig()
    : reload_configuration_signalled( false ),
      report_status_signalled( false ),
      scripts_thread_checkpoint( 0 ),
      tasks_thread_checkpoint( 0 ),
      active_client_thread_checkpoint( 0 ),
      check_attack_after_move_function_checkpoint( 0 )
{
}

#ifdef _WIN32

void install_signal_handlers()
{
  Clib::enable_exit_signaller();
}
void signal_catch_thread() {}
#else

pthread_t main_pthread;


void sigpipe_handler( int x )
{
  INFO_PRINTLN( "SIGPIPE! param={}", x );
}

void handle_HUP( int /*x*/ )
{
  stateManager.polsig.reload_configuration_signalled = true;
}

void handle_SIGUSR1( int /*x*/ )  // LINUXTEST
{
  stateManager.polsig.report_status_signalled = true;
}

void null_handler( int /*x*/ ) {}

void install_null_handler( int sig )
{
  struct sigaction sa_null;
  sa_null.sa_handler = null_handler;
  sigemptyset( &sa_null.sa_mask );
  sa_null.sa_flags = 0;
  if ( sigaction( sig, &sa_null, nullptr ) == -1 )
  {
    perror( "sigaction failed" );
    if ( Clib::Logging::global_logger )
      Clib::Logging::global_logger->wait_for_empty_queue();
    exit( 1 );
  }
}

void install_signal_handlers()
{
  install_null_handler( SIGPIPE );

  install_null_handler( SIGHUP );
  // install_null_handler( SIGUSR1 );
  install_null_handler( SIGINT );
  install_null_handler( SIGQUIT );
  install_null_handler( SIGTERM );

  sigset_t blocked_sigs;
  sigemptyset( &blocked_sigs );
  sigfillset( &blocked_sigs );
  sigdelset( &blocked_sigs, SIGSEGV );
  sigdelset( &blocked_sigs, SIGUSR2 );
  sigdelset( &blocked_sigs, SIGUSR1 );

  int res = pthread_sigmask( SIG_BLOCK, &blocked_sigs, nullptr );
  if ( res )
  {
    ERROR_PRINTLN( "pthread_sigmask failed: {}", res );
    if ( Clib::Logging::global_logger )
      Clib::Logging::global_logger->wait_for_empty_queue();
    exit( 1 );
  }

  // main_pid = getpid();
  main_pthread = pthread_self();
}

void signal_catch_thread()
{
  int res = pthread_kill( main_pthread, SIGTERM );
  if ( res )
  {
    ERROR_PRINTLN( "pthread_kill failed: {}", res );
  }
}

void catch_signals_thread()
{
  sigset_t expected_signals;
  sigemptyset( &expected_signals );
  sigaddset( &expected_signals, SIGHUP );
  // sigaddset( &expected_signals, SIGUSR1 );
  sigaddset( &expected_signals, SIGINT );
  sigaddset( &expected_signals, SIGQUIT );
  sigaddset( &expected_signals, SIGTERM );

  // cerr << "catch_signals_thread running" << endl;
  // this thread should be the second last out, so it can still respond to SIGUSR1
  // this is called from main(), so it can exit when there is one thread left
  // (that will be the thread_status thread)
  while ( !Clib::exit_signalled || threadhelp::child_threads > 1 )
  {
    int caught = 0;
    sigwait( &expected_signals, &caught );
    switch ( caught )
    {
    case SIGHUP:
      ERROR_PRINTLN( "SIGHUP: reload configuration." );
      stateManager.polsig.reload_configuration_signalled = true;
      break;
    case SIGUSR1:
      ERROR_PRINTLN( "SIGUSR1: report thread status." );
      stateManager.polsig.report_status_signalled = true;
      break;
    case SIGINT:
      ERROR_PRINTLN( "SIGINT: exit." );
      Clib::signal_exit();
      break;
    case SIGQUIT:
      ERROR_PRINTLN( "SIGQUIT: exit." );
      Clib::signal_exit();
      break;
    case SIGTERM:
      ERROR_PRINTLN( "SIGTERM: exit." );
      Clib::signal_exit();
      break;
    default:
      ERROR_PRINTLN( "Unexpected signal: {}", caught );
      break;
    }
  }
  // cerr << "catch_signals thread exits" << endl;
}

#endif
}  // namespace Core
}  // namespace Pol
