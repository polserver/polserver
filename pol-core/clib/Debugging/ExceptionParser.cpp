#include "clib/Debugging/ExceptionParser.h"

#include "clib/Program/ProgramConfig.h"
#include "clib/logfacility.h"
#include "clib/scriptstatus.h"
#include "clib/stlutil.h"
#include "clib/threadhelp.h"
#include <pol_global_config.h>

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string_view>

#ifndef WINDOWS
#include <cxxabi.h>
#include <execinfo.h>
#include <sys/mman.h>
#include <unistd.h>
#else
#include "clib/Header_Windows.h"
#include <shlwapi.h>
#endif

#include <boost/stacktrace.hpp>

#define MAX_STACK_TRACE_DEPTH 200
#define MAX_STACK_TRACE_STEP_LENGTH 512


namespace Pol::Clib
{
using namespace std;

///////////////////////////////////////////////////////////////////////////////

std::atomic<const std::atomic<size_t>*> ExceptionParser::m_worldLockOwner{ nullptr };
std::string ExceptionParser::m_programStart = Pol::Clib::Logging::LogSink::getTimeStamp();

///////////////////////////////////////////////////////////////////////////////

namespace
{
void getSignalDescription( int signal, string& signalName, string& signalDescription )
{
  switch ( signal )
  {
  case 1:
    signalName = "SIGHUP";
    signalDescription = "hangup detected on controlling terminal or death of controlling process";
    break;
  case 2:
    signalName = "SIGINT";
    signalDescription = "interrupt from keyboard";
    break;
  case 3:
    signalName = "SIGQUIT";
    signalDescription = "quit from keyboard";
    break;
  case 4:
    signalName = "SIGILL";
    signalDescription = "illegal Instruction";
    break;
  case 6:
    signalName = "SIGABRT";
    signalDescription = "abort signal from abort()";
    break;
  case 8:
    signalName = "SIGFPE";
    signalDescription = "floating point exception";
    break;
  case 9:
    signalName = "SIGKILL";
    signalDescription = "kill signal";
    break;
  case 10:
    signalName = "SIGBUS";
    signalDescription = "bus error";
    break;
  case 11:
    signalName = "SIGSEGV";
    signalDescription = "invalid memory reference";
    break;
  case 12:
    signalName = "SIGSYS";
    signalDescription = "bad argument to system call";
    break;
  case 13:
    signalName = "SIGPIPE";
    signalDescription = "broken pipe: write to pipe with no readers";
    break;
  case 14:
    signalName = "SIGALRM";
    signalDescription = "timer signal from alarm()";
    break;
  case 15:
    signalName = "SIGTERM";
    signalDescription = "termination signal";
    break;
  case 18:
    signalName = "SIGCONT";
    signalDescription = "continue signal from tty";
    break;
  case 19:
    signalName = "SIGSTOP";
    signalDescription = "stop signal from tty";
    break;
  case 20:
    signalName = "SIGTSTP";
    signalDescription = "stop signal from user (keyboard)";
    break;
  case 16:
  case 30:
    signalName = "SIGUSR1";
    signalDescription = "user-defined signal 1";
    break;
  case 17:
  case 31:
    signalName = "SIGUSR2";
    signalDescription = "user-defined signal 2";
    break;
  default:
    signalName = "unsupported signal";
    signalDescription = "unsupported signal occurred";
    break;
  }
}

#if !defined( _WIN32 ) && !defined( __APPLE__ )
void logExceptionSignal( int signal )
{
  string signalName;
  string signalDescription;

  getSignalDescription( signal, signalName, signalDescription );
  printf( "Signal \"%s\"(%d: %s) detected.\n", signalName.c_str(), signal,
          signalDescription.c_str() );
}
#endif

string getCompilerVersion()
{
#ifdef LINUX
  char result[256];
#ifdef __clang__
  snprintf( result, arsize( result ), "clang %d.%d.%d", __clang_major__, __clang_minor__,
            __clang_patchlevel__ );
#else
  snprintf( result, arsize( result ), "gcc %d.%d.%d", __GNUC__, __GNUC_MINOR__,
            __GNUC_PATCHLEVEL__ );
#endif
#endif
#ifdef WINDOWS
  string result;
  switch ( _MSC_VER )
  {
  case 1900:
    result = "MSVC++ 14.0 (Visual Studio 2015)";
    break;
  case 1800:
    result = "MSVC++ 12.0 (Visual Studio 2013)";
    break;
  case 1700:
    result = "MSVC++ 11.0 (Visual Studio 2012)";
    break;
  case 1600:
    result = "MSVC++ 10.0 (Visual Studio 2010)";
    break;
  case 1500:
    result = "MSVC++ 9.0 (Visual Studio 2008)";
    break;
  case 1400:
    result = "MSVC++ 8.0 (Visual Studio 2005)";
    break;
  case 1310:
    result = "MSVC++ 7.1 (Visual Studio 2003)";
    break;
  case 1300:
    result = "MSVC++ 7.0";
    break;
  case 1200:
    result = "MSVC++ 6.0";
    break;
  case 1100:
    result = "MSVC++ 5.0";
    break;
  default:
#if ( _MSC_VER > 1800 )
    result = "MSVC++ newer than version 12.0";
#elif ( _MSC_VER < 1100 )
    result = "MSVC++ older than version 5.0";
#else
    result = "MSVC++ (some unsupported version)";
#endif
    break;
  }
#endif

  return result;
}
}  // namespace

namespace
{
// The script this thread was last known to be running, for a report to print beside its
// native trace. Taken with the non-blocking snapshot: a report that waits on a lock held by
// the very thread it describes has traded the report for a hang.
//
// "Last" is meant literally -- nothing clears the value when a script finishes, so this can
// name a script the thread has already moved on from. Labelled accordingly, matching the
// minidump and the watchdog dump.
std::string last_script_line()
{
  std::string name;
  unsigned pc;
  Clib::script_status.snapshot( name, pc );
  return fmt::format( "Last Script: {} PC: {}", name, pc );
}
}  // namespace

void ExceptionParser::registerWorldLockOwner( const std::atomic<size_t>* owner )
{
  m_worldLockOwner.store( owner, std::memory_order_relaxed );
}

void ExceptionParser::handleExceptionSignal( int signal )
{
  switch ( signal )
  {
  case SIGILL:
  case SIGFPE:
  case SIGSEGV:
  case SIGTERM:
  case SIGABRT:
  {
    /**
     * inform the user about the program abort
     */
    printf(
        "########################################################################################"
        "\n" );
    printf(
        "POL will exit now. Please, post the following to the forum: "
        "http://forums.polserver.com/.\n" );
    // Everything cheap goes out and is flushed before the stack trace is walked. That walk
    // symbolizes and allocates, and the crashes where it matters most -- a corrupted heap, a
    // fault taken inside the allocator -- are exactly the ones where it hangs or faults again.
    // Gathering it first, as this did, meant those crashes printed nothing at all.
    //
    // Which thread died is the first thing anyone reading this needs, and the trace alone
    // rarely says: the top frames are usually library code shared by every thread. Name comes
    // from the per-thread buffer, so no lock is taken here. SIGSEGV and SIGABRT are both
    // delivered to the thread that caused them, so this is the faulting thread and not a
    // bystander.
    printf( "Thread: %s (%zu)\n", threadhelp::current_thread_name(), threadhelp::thread_pid() );
    printf( "%s\n", last_script_line().c_str() );
    const std::atomic<size_t>* owner_slot = m_worldLockOwner.load( std::memory_order_relaxed );
    if ( owner_slot != nullptr )
    {
      const size_t owner = owner_slot->load( std::memory_order_relaxed );
      if ( owner != 0 )
        printf( "World lock held by tid: %zu\n", owner );
      else
        printf( "World lock: free\n" );
    }
    printf( "Executable: %s\n", PROG_CONFIG::programName().c_str() );
    printf( "Start time: %s\n", m_programStart.c_str() );
    printf( "Current time: %s\n", Pol::Clib::Logging::LogSink::getTimeStamp().c_str() );
    printf( "\n" );
    fflush( stdout );

    string tStackTrace = ExceptionParser::getTrace();
    printf( "Stack trace:\n%s", tStackTrace.c_str() );
    printf( "\n" );
    printf( "Compiler: %s\n", getCompilerVersion().c_str() );
    printf( "Compile time: %s\n", ProgramConfig::build_datetime().c_str() );
    printf( "Build target: %s\n", ProgramConfig::build_target().c_str() );
    printf( "Build revision: %s\n", POL_VERSION_ID );
#if defined( __GLIBC__ )
    printf( "GNU C library (compile time): %d.%d\n", __GLIBC__, __GLIBC_MINOR__ );
#endif
    printf( "\n" );
    printf(
        "########################################################################################"
        "\n" );
    fflush( stdout );

    // finally, go to hell
    std::_Exit( 1 );
  }
  break;
  default:
    break;
  }
}

///////////////////////////////////////////////////////////////////////////////

ExceptionParser::ExceptionParser() = default;

ExceptionParser::~ExceptionParser() {}

///////////////////////////////////////////////////////////////////////////////

#if !defined( _WIN32 ) && !defined( __APPLE__ )
static void handleSignalLinux( int signal, siginfo_t* signalInfo, void* arg )
{
  (void)arg;
  logExceptionSignal( signal );
  if ( signalInfo != nullptr )
  {
    if ( signal == SIGSEGV )
    {
      if ( signalInfo->si_addr != nullptr )
        printf( "Segmentation fault detected - faulty memory reference at location: %p\n",
                signalInfo->si_addr );
      else
        printf( "Segmentation fault detected - null pointer reference\n" );
    }
    if ( signalInfo->si_errno != 0 )
      printf( "This signal occurred because \"%s\"(%d)\n", strerror( signalInfo->si_errno ),
              signalInfo->si_errno );
    if ( signalInfo->si_code != 0 )
      printf( "Signal code is %d\n", signalInfo->si_code );
  }
  ExceptionParser::handleExceptionSignal( signal );
}

static void handleStackTraceRequestLinux( int signal, siginfo_t* signalInfo, void* arg )
{
  (void)signal;
  (void)signalInfo;
  (void)arg;
  // Name from the per-thread buffer rather than the ThreadMap: this runs in a
  // signal handler, and logAllStackTraces() below raises SIGUSR1 on every thread
  // at once, so each of them would be contending for the map's spinlock -- and
  // deadlocking outright against a thread stopped while holding it.
  std::string output =
      fmt::format( "STACK TRACE for thread \"{}\"({}):\n{}\n", threadhelp::current_thread_name(),
                   threadhelp::thread_pid(), last_script_line() );
  output += ExceptionParser::getTrace() + '\n';

  // print to stdout
  printf( "%s", output.c_str() );

  // print to error output
  POLLOG_ERROR( output );

  // wait here for logging facility to make sure everything was processed
  if ( Clib::Logging::global_logger )
    Clib::Logging::global_logger->wait_for_empty_queue();
}

void ExceptionParser::logAllStackTraces()
{
  threadhelp::ThreadMap::Contents threadsDesc;
  threadhelp::threadmap_instance().CopyContents( threadsDesc );
  for ( const auto& threadDesc : threadsDesc )
  {
    pthread_t threadID = (pthread_t)threadDesc.first;

    if ( pthread_kill( threadID, SIGUSR1 ) != 0 )
    {
      fprintf( stderr, "%s",
               fmt::format( "pthread_kill() failed to send SIGUSR1 to thread {}({})\n",
                            threadsDesc[threadID], threadID )
                   .c_str() );
    }
  }
}

void ExceptionParser::initGlobalExceptionCatching()
{
  struct sigaction sigAction;

  memset( &sigAction, 0, sizeof( sigAction ) );
  sigemptyset( &sigAction.sa_mask );
  sigAction.sa_sigaction = handleSignalLinux;
  sigAction.sa_flags = SA_SIGINFO;
  sigaction( SIGINT, &sigAction, nullptr );
  sigaction( SIGTERM, &sigAction, nullptr );
  sigaction( SIGSEGV, &sigAction, nullptr );
  sigaction( SIGABRT, &sigAction, nullptr );
  sigAction.sa_sigaction = handleStackTraceRequestLinux;
  sigaction( SIGUSR1, &sigAction, nullptr );

  // set handler stack
  stack_t tStack;
  // mmap: no false positives for leak, plus guardpages to get SIGSEGV on memory overwrites
  char* mem = static_cast<char*>( mmap( nullptr, SIGSTKSZ + 2 * getpagesize(),
                                        PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0 ) );
  mprotect( mem, getpagesize(), PROT_NONE );
  mprotect( mem + getpagesize() + SIGSTKSZ, getpagesize(), PROT_NONE );
  tStack.ss_sp = mem + getpagesize();
  tStack.ss_size = SIGSTKSZ;
  tStack.ss_flags = 0;
  if ( sigaltstack( &tStack, nullptr ) == -1 )
  {
    printf( "Could not set signal handler stack\n" );
    std::exit( 1 );
  }
}
#else  // _WIN32 || Apple

void ExceptionParser::logAllStackTraces() {}

void ExceptionParser::initGlobalExceptionCatching()
{
#if defined( _WIN32 )
  // see compile_defs.cmake
  // in addition add the executable path as searchpath
  wchar_t path[MAX_PATH];
  GetModuleFileNameW( nullptr, path, MAX_PATH );
  PathRemoveFileSpecW( path );
  SetEnvironmentVariableW( L"_NT_ALT_SYMBOL_PATH", path );
#endif
}
#endif  // _WIN32 || Apple

string ExceptionParser::getTrace()
{
  auto stack = boost::stacktrace::stacktrace::from_current_exception();
  // current_exception does not always work, eg no active exception
  // and needs linking with libboost_stacktrace_backtrace, which as of now is not possible with
  // Apple. Current stacktrace as fallback, which should give enough infos
  if ( stack.empty() )
    stack = boost::stacktrace::stacktrace();
  return boost::stacktrace::to_string( stack );
}

std::string indent_stack_block( const std::string& block, size_t max_lines )
{
  std::string_view remaining( block );
  while ( !remaining.empty() && ( remaining.back() == '\n' || remaining.back() == '\r' ) )
    remaining.remove_suffix( 1 );

  std::string result;
  size_t kept = 0, dropped = 0;

  while ( !remaining.empty() )
  {
    auto eol = remaining.find( '\n' );
    auto line = remaining.substr( 0, eol );
    remaining = eol == std::string_view::npos ? std::string_view() : remaining.substr( eol + 1 );

    if ( kept < max_lines )
    {
      fmt::format_to( std::back_inserter( result ), "  {}\n", line );
      ++kept;
    }
    else
      ++dropped;
  }

  if ( dropped )
    fmt::format_to( std::back_inserter( result ), "  ... ({} more)\n", dropped );

  return result;
}

///////////////////////////////////////////////////////////////////////////////
}  // namespace Pol::Clib
