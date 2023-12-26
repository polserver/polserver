#include "ExceptionParser.h"

#include "../Program/ProgramConfig.h"
#include "../logfacility.h"
#include "../stlutil.h"
#include "../threadhelp.h"
#include "pol_global_config.h"
#include <format/format.h>

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <signal.h>
#include <stdio.h>

#ifndef WINDOWS
#include <arpa/inet.h>
#include <cxxabi.h>
#include <execinfo.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/mman.h>
#include <unistd.h>

#define SOCKET int
#else
#include "../Header_Windows.h"
#endif

#define MAX_STACK_TRACE_DEPTH 200
#define MAX_STACK_TRACE_STEP_LENGTH 512

namespace Pol
{
namespace Clib
{
using namespace std;

///////////////////////////////////////////////////////////////////////////////

bool ExceptionParser::m_programAbortReporting = false;
std::string ExceptionParser::m_programAbortReportingServer = "";
std::string ExceptionParser::m_programAbortReportingUrl = "";
std::string ExceptionParser::m_programAbortReportingReporter = "";
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

void doHttpPOST( const string& host, const string& url, const string& content )
{
#define MAXLINE 4096
  char request[MAXLINE + 1];
  SOCKET socketFD;
  char targetIP[INET6_ADDRSTRLEN];

/**
 * prepare the request
 */
#ifdef _MSC_VER
  _snprintf(
#else
  snprintf(
#endif
      request, MAXLINE,
      "POST %s HTTP/1.0\r\n"
      "Host: %s\r\n"
      "Content-Type: application/x-www-form-urlencoded\r\n"
      "User-Agent: POL in-app abort reporting system, %s\r\n"
      "Content-length: %d\r\n\r\n"
      "%s",
      url.c_str(), host.c_str(), POL_VERSION_ID, (int)content.size(), content.c_str() );

  /**
   * DNS lookup if needed
   */
  struct addrinfo* serverAddr;
  struct addrinfo hints;
  memset( &hints, 0, sizeof hints );
  hints.ai_family = AF_UNSPEC;  // IPv4 or IPv6
  hints.ai_flags = AI_ADDRCONFIG;
  hints.ai_socktype = SOCK_STREAM;
  int res = getaddrinfo( host.c_str(), "http", &hints, &serverAddr );
  if ( res != 0 )
  {
    fprintf( stderr, "getaddrinfo() failed for \"%s\" due to \"%s\"(code: %d)\n", host.c_str(),
             gai_strerror( res ), res );
    std::_Exit( 1 );
  }

  switch ( serverAddr->ai_addr->sa_family )
  {
  case AF_INET:
    if ( inet_ntop( AF_INET, &( (struct sockaddr_in*)serverAddr->ai_addr )->sin_addr, targetIP,
                    INET_ADDRSTRLEN ) == nullptr )
      std::_Exit( 1 );
    break;

  case AF_INET6:
    if ( inet_ntop( AF_INET6, &( (struct sockaddr_in*)serverAddr->ai_addr )->sin_addr, targetIP,
                    INET6_ADDRSTRLEN ) == nullptr )
      std::_Exit( 1 );
    break;

  default:
    fprintf( stderr, "Unknown address family found for %s\n", host.c_str() );
    std::_Exit( 1 );
  }

  // create the socket
  socketFD = socket( serverAddr->ai_family, serverAddr->ai_socktype, serverAddr->ai_protocol );

  /**
   * connect to the bug tracking server
   */
  if ( ( res = connect( socketFD, serverAddr->ai_addr, (int)serverAddr->ai_addrlen ) ) != 0 )
  {
    fprintf( stderr, "connect() failed for server \"%s\"(IP: %s) due \"%s\"(%d)\n", host.c_str(),
             targetIP, strerror( errno ), errno );
    std::_Exit( 1 );
  }

  freeaddrinfo( serverAddr );  // not needed anymore

/**
 * send the request
 */
#if !defined( _WIN32 ) && !defined( __APPLE__ )
  send( socketFD, request, strlen( request ), MSG_NOSIGNAL );
#else
  send( socketFD, request, (int)strlen( request ), 0 );
#endif
  printf( "Abort report was sent to %s%s (IP: %s)\n", host.c_str(), url.c_str(), targetIP );

  /**
   * wait for some answers and print them on the screen
   */
  ssize_t readBytes;
  char answer[MAXLINE + 1];

#if !defined( _WIN32 ) && !defined( __APPLE__ )
  while ( ( readBytes = recv( socketFD, answer, MAXLINE, MSG_NOSIGNAL ) ) > 0 )
  {
#else
  while ( ( readBytes = recv( socketFD, answer, MAXLINE, 0 ) ) > 0 )
  {
#endif
    answer[readBytes] = '\0';
    printf( "Answer from bug tracking server:\n%s\n", answer );
    // skip the received answer and proceed
  }

// close the socket to the bug tracking server
#ifndef _WIN32
  close( socketFD );
#else
  closesocket( socketFD );
#endif
}
}  // namespace

void ExceptionParser::reportProgramAbort( const string& stackTrace, const string& reason )
{
  /**
   * set some default values if the abort occurs too early and pol.cfg wasn't parsed yet
   */
  string host = "polserver.com";
  string url = "/pol/report_program_abort.php";
  if ( ( m_programAbortReportingServer.c_str() != nullptr ) &&
       ( m_programAbortReportingServer != "" ) )
  {
    host = m_programAbortReportingServer;
    if ( m_programAbortReportingUrl.c_str() != nullptr )
      url = m_programAbortReportingUrl;
  }

  // create the abort description for the subsequent POST request
  string content = "email=" + m_programAbortReportingReporter +
                   "&"
                   "bin=" +
                   PROG_CONFIG::programName() +
                   "&"
                   "start_time=" +
                   m_programStart +
                   "&"
                   "abort_time=" +
                   Pol::Clib::Logging::LogSink::getTimeStamp() +
                   "&"
                   "reason=" +
                   reason +
                   "&"
                   "trace=" +
                   stackTrace +
                   "&"
                   "comp=" +
                   getCompilerVersion() +
                   "&"
                   "comp_time=" +
                   ProgramConfig::build_datetime() +
                   "&"
                   "build_target=" +
                   ProgramConfig::build_target() +
                   "&"
                   "build_revision=" POL_VERSION_ID
                   "&"
                   "misc=";

  // execute the POST request
  doHttpPOST( host, url, content );
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
    if ( m_programAbortReporting )
      printf( "POL will exit now. The following will be sent to the POL developers:\n\n" );
    else
      printf(
          "POL will exit now. Please, post the following to the forum: "
          "http://forums.polserver.com/.\n" );
    string tStackTrace = ExceptionParser::getTrace();
    printf( "%s", tStackTrace.c_str() );
    printf( "Admin contact: %s\n", m_programAbortReportingReporter.c_str() );
    printf( "Executable: %s\n", PROG_CONFIG::programName().c_str() );
    printf( "Start time: %s\n", m_programStart.c_str() );
    printf( "Current time: %s\n", Pol::Clib::Logging::LogSink::getTimeStamp().c_str() );
    printf( "\n" );
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

    /**
     * use the program abort reporting system
     */
    if ( m_programAbortReporting )
    {
      string signalName;
      string signalDescription;

      getSignalDescription( signal, signalName, signalDescription );
      ExceptionParser::reportProgramAbort(
          tStackTrace, "CRASH caused by signal " + signalName + " (" + signalDescription + ")" );
    }

    // finally, go to hell
    std::_Exit( 1 );
  }
  break;
  default:
    break;
  }
}

///////////////////////////////////////////////////////////////////////////////

ExceptionParser::ExceptionParser() {}

ExceptionParser::~ExceptionParser() {}

///////////////////////////////////////////////////////////////////////////////

#if !defined( _WIN32 ) && !defined( __APPLE__ )
string ExceptionParser::getTrace()
{
  string result;

  void* stackTrace[MAX_STACK_TRACE_DEPTH];
  int stackTraceSize;
  char** stackTraceList;
  int stackTraceStep = 0;
  char* stringBuf = (char*)malloc( MAX_STACK_TRACE_STEP_LENGTH );

  stackTraceSize = backtrace( stackTrace, MAX_STACK_TRACE_DEPTH );
  stackTraceList = backtrace_symbols( stackTrace, stackTraceSize );

  size_t funcNameSize = 256;
  char* funcnName = (char*)malloc( funcNameSize );

  // iterate over all entries and do the demangling
  for ( int i = 0; i < stackTraceSize; i++ )
  {
    // get the pointers to the name, offset and end of offset
    char* beginFuncName = nullptr;
    char* beginFuncOffset = nullptr;
    char* endFuncOffset = nullptr;
    char* beginBinaryName = stackTraceList[i];
    char* beginBinaryOffset = nullptr;
    char* endBinaryOffset = nullptr;
    for ( char* entryPointer = stackTraceList[i]; *entryPointer; ++entryPointer )
    {
      if ( *entryPointer == '(' )
      {
        beginFuncName = entryPointer;
      }
      else if ( *entryPointer == '+' )
      {
        beginFuncOffset = entryPointer;
      }
      else if ( *entryPointer == ')' && beginFuncOffset )
      {
        endFuncOffset = entryPointer;
      }
      else if ( *entryPointer == '[' )
      {
        beginBinaryOffset = entryPointer;
      }
      else if ( *entryPointer == ']' && beginBinaryOffset )
      {
        endBinaryOffset = entryPointer;
        break;
      }
    }

    // set the default value for the output line
    sprintf( stringBuf, "\n" );

    bool parse_succeeded = beginFuncName && beginFuncOffset && endFuncOffset && beginBinaryOffset &&
                           endBinaryOffset && beginFuncName < beginFuncOffset;

    // get the detailed values for the output line if available
    if ( parse_succeeded )
    {
      // terminate the C strings
      *beginFuncName++ = '\0';
      *beginFuncOffset++ = '\0';
      *endFuncOffset = '\0';
      *beginBinaryOffset++ = '\0';
      *endBinaryOffset = '\0';

      int res;
      funcnName = abi::__cxa_demangle( beginFuncName, funcnName, &funcNameSize, &res );
      unsigned int binaryOffset = strtoul( beginBinaryOffset, nullptr, 16 );
      if ( res == 0 )
      {
        string funcnNameStr = ( funcnName ? funcnName : "" );
        if ( funcnName && strncmp( funcnName, "Pol::", 5 ) == 0 )
          funcnNameStr = ">> " + funcnNameStr;

        if ( beginBinaryName && strlen( beginBinaryName ) )
          sprintf( stringBuf, "#%02d 0x%016x in %s:[%s] from %s\n", stackTraceStep, binaryOffset,
                   funcnNameStr.c_str(), beginFuncOffset, beginBinaryName );
        else
          sprintf( stringBuf, "#%02d 0x%016x in %s from %s\n", stackTraceStep, binaryOffset,
                   funcnNameStr.c_str(), beginFuncOffset );
        stackTraceStep++;
      }
      else
      {
        if ( beginBinaryName && strlen( beginBinaryName ) )
          sprintf( stringBuf, "#%02d 0x%016x in %s:[%s] from %s\n", stackTraceStep, binaryOffset,
                   beginFuncName, beginFuncOffset, beginBinaryName );
        else
          sprintf( stringBuf, "#%02d 0x%016x in %s:[%s]\n", stackTraceStep, binaryOffset,
                   beginFuncName, beginFuncOffset );
        stackTraceStep++;
      }
    }
    else
    {
      // print the raw trace, as it is better than nothing
      sprintf( stringBuf, "#%02d %s\n", stackTraceStep, stackTraceList[i] );
      stackTraceStep++;
    }

    // append the line to the result
    result += string( stringBuf );
  }

  // memory cleanup
  free( funcnName );
  free( stackTraceList );
  free( stringBuf );

  return result;
}

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
  threadhelp::ThreadMap::Contents threadDesc;
  threadhelp::threadmap.CopyContents( threadDesc );

  fmt::Writer output;
  output << "STACK TRACE for thread \"" << threadDesc[pthread_self()] << "\"(" << pthread_self()
         << "):\n";
  output << ExceptionParser::getTrace() << "\n";

  // print to stdout
  printf( "%s", output.c_str() );

  // print to error output
  POLLOG_ERROR << output.str();

  // wait here for logging facility to make sure everything was processed
  if ( Clib::Logging::global_logger )
    Clib::Logging::global_logger->wait_for_empty_queue();
}

void ExceptionParser::logAllStackTraces()
{
  threadhelp::ThreadMap::Contents threadsDesc;
  threadhelp::threadmap.CopyContents( threadsDesc );
  for ( const auto& threadDesc : threadsDesc )
  {
    pthread_t threadID = (pthread_t)threadDesc.first;

    if ( pthread_kill( threadID, SIGUSR1 ) != 0 )
    {
      fmt::Writer output;
      output << "pthread_kill() failed to send SIGUSR1 to thread " << threadsDesc[threadID] << "("
             << threadID << ")\n";
      fprintf( stderr, "%s", output.c_str() );
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
#else   // _WIN32
string ExceptionParser::getTrace()
{
  string result;

  return result;
}

void ExceptionParser::logAllStackTraces() {}

void ExceptionParser::initGlobalExceptionCatching() {}
#endif  // _WIN32

void ExceptionParser::configureProgramAbortReportingSystem( bool active, std::string server,
                                                            std::string url, std::string reporter )
{
  m_programAbortReporting = active;
  m_programAbortReportingServer = std::move( server );
  m_programAbortReportingUrl = std::move( url );
  m_programAbortReportingReporter = std::move( reporter );
}

bool ExceptionParser::programAbortReporting()
{
  return m_programAbortReporting;
}

///////////////////////////////////////////////////////////////////////////////
}  // namespace Clib
}  // namespace Pol
