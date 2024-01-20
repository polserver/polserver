/** @file
 *
 * @par History
 * - 2005/09/16 Shinigami: added scripts_thread_script* to support better debugging
 * - 2009-07-18 MuadDib: Updated dump messages for bug tracker
 */

#include "mdump.h"
#include "clib.h"
#include "logfacility.h"
#include "passert.h"
#include "strexcpt.h"
#include "threadhelp.h"
#include <iterator>

#pragma warning( disable : 4091 )  // unused typedef
#include "../../lib/StackWalker/StackWalker.h"

#include "Header_Windows.h"
#include <assert.h>
#include <time.h>

// FIXME: 2008 Upgrades needed here? Need to check dbg headers to ensure compatibility
#if _MSC_VER < 1300
#define DECLSPEC_DEPRECATED
// VC6: change this path to your Platform SDK headers
// must be XP version of file
#include "C:\\Program Files\\Microsoft Visual Studio\\PlatformSDK\\include\\dbghelp.h"
#else
// VC7: ships with updated headers
#include "dbghelp.h"
#endif

#ifdef _MSC_VER
#pragma warning( disable : 4100 )  // TODO: This file needs some serious rewrite, so I'm just
                                   // ignoring the unreferenced parameters for now
#endif

// based on dbghelp.h
typedef BOOL( WINAPI* MINIDUMPWRITEDUMP )( HANDLE hProcess, DWORD dwPid, HANDLE hFile,
                                           MINIDUMP_TYPE DumpType,
                                           CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
                                           CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
                                           CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam );
typedef BOOL( WINAPI* __SymInitialize )( _In_ HANDLE hProcess, _In_opt_ PCSTR UserSearchPath,
                                         _In_ BOOL fInvadeProcess );
typedef BOOL( WINAPI* __SymFromAddr )( _In_ HANDLE hProcess, _In_ DWORD64 Address,
                                       _Out_opt_ PDWORD64 Displacement,
                                       _Inout_ PSYMBOL_INFO Symbol );
typedef DWORD( WINAPI* __SymGetOptions )( VOID );
typedef DWORD( WINAPI* __SymSetOptions )( _In_ DWORD SymOptions );
typedef BOOL( WINAPI* __SymGetLineFromAddr64 )( _In_ HANDLE hProcess, _In_ DWORD64 qwAddr,
                                                _Out_ PDWORD pdwDisplacement,
                                                _Out_ PIMAGEHLP_LINE64 Line64 );

#include "mdumpimp.h"
namespace Pol
{
namespace Clib
{
void MiniDumper::Initialize()
{
  HiddenMiniDumper::Initialize();
}
void MiniDumper::SetMiniDumpType( const std::string& dumptype )
{
  HiddenMiniDumper::_MiniDumpType = dumptype;
}

bool HiddenMiniDumper::_Initialized;
unsigned HiddenMiniDumper::_DumpCount;
std::string HiddenMiniDumper::_MiniDumpType;
char HiddenMiniDumper::_StartTimestamp[32];

HMODULE hDbgHelpDll;

void HiddenMiniDumper::Initialize()
{
  /*
      if this assert fires then you have initialized HiddenMiniDumper twice
      which is not allowed
      */
  assert( !_Initialized );
  _Initialized = true;

  auto time_tm = Clib::localtime( time( nullptr ) );
  strftime( _StartTimestamp, sizeof _StartTimestamp, "%Y%m%d%H%M%S", &time_tm );

  // appname will be obtained from progver

  // find a better value for your app
  // HWND hParent = nullptr;

  /*
      firstly see if dbghelp.dll is around and has the function we need
      look next to the EXE first, as the one in System32 might be old
      (e.g. Windows 2000)
      */
  char szDbgHelpPath[_MAX_PATH];
  char* szResult = nullptr;

  if ( GetModuleFileName( nullptr, szDbgHelpPath, _MAX_PATH ) )
  {
    char* pSlash = strchr( szDbgHelpPath, '\\' );
    if ( pSlash )
    {
      strcpy( pSlash + 1, "DBGHELP.DLL" );
      hDbgHelpDll = ::LoadLibrary( szDbgHelpPath );
    }
  }

  if ( hDbgHelpDll == nullptr )
  {
    // load any version we can
    hDbgHelpDll = ::LoadLibrary( "DBGHELP.DLL" );
  }
  if ( hDbgHelpDll )
  {
    MINIDUMPWRITEDUMP pDump =
        ( MINIDUMPWRITEDUMP )::GetProcAddress( hDbgHelpDll, "MiniDumpWriteDump" );
    if ( pDump )
      ::SetUnhandledExceptionFilter( TopLevelFilter );
    else
      szResult = "Warning: DBGHELP.DLL too old, version 5.1+ required.";
  }
  else
    szResult = "Warning: DBGHELP.DLL not found, version 5.1+ required in POL directory.";

  if ( szResult )
  {
    POLLOG_INFOLN( szResult );
    InstallOldStructuredExceptionHandler();
  }
}


LONG HiddenMiniDumper::TopLevelFilter( struct _EXCEPTION_POINTERS* pExceptionInfo )
{
  LONG retval = EXCEPTION_CONTINUE_SEARCH;
  std::string result;
  std::string dumppath;
  if ( !_Initialized )
    Initialize();

  MINIDUMPWRITEDUMP pDump =
      ( MINIDUMPWRITEDUMP )::GetProcAddress( hDbgHelpDll, "MiniDumpWriteDump" );
  if ( pDump )
  {
    dumppath = fmt::format( "{}-{:x}.dmp", _StartTimestamp, _DumpCount++ );

    HANDLE hFile = ::CreateFile( dumppath.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, nullptr,
                                 CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr );

    if ( hFile != INVALID_HANDLE_VALUE )
    {
      _MINIDUMP_EXCEPTION_INFORMATION ExInfo;

      ExInfo.ThreadId = ::GetCurrentThreadId();
      ExInfo.ExceptionPointers = pExceptionInfo;
      ExInfo.ClientPointers = 0;

      // write the dump
      MINIDUMP_TYPE dumptype;
      if ( _MiniDumpType == "large" )
        dumptype = MiniDumpWithFullMemory;
      else if ( _MiniDumpType == "variable" )
        dumptype = MiniDumpWithDataSegs;
      else
        dumptype = MiniDumpNormal;

      ERROR_PRINTLN( "Unhandled Exception! Minidump started..." );
      BOOL bOK = pDump( GetCurrentProcess(), GetCurrentProcessId(), hFile, dumptype, &ExInfo,
                        nullptr, nullptr );
      if ( bOK )
      {
        result = fmt::format(
            "Unhandled Exception! Writing Minidump file. \nPost this file with explanation and "
            "last lines from log files on http://forums.polserver.com/tracker.php for the "
            "development team.\nSaved dump file to '{}'\n",
            dumppath );
        retval = EXCEPTION_EXECUTE_HANDLER;
      }
      else
      {
        result =
            fmt::format( "Failed to save dump file to '{}' (error {})", dumppath, GetLastError() );
      }
      ::CloseHandle( hFile );
    }
    else
    {
      result =
          fmt::format( "Failed to create dump file '{}' (error {})", dumppath, GetLastError() );
    }
  }
  print_backtrace();
  FreeLibrary( hDbgHelpDll );
  _Initialized = false;

  if ( !result.empty() )
  {
    POLLOG_ERRORLN(
        "##########################################################\n"
        "{}\n"
        "Last Script: {} PC: {}\n"
        "##########################################################",
        result, scripts_thread_script, scripts_thread_scriptPC );
  }
  if ( Clib::Logging::global_logger )
    Clib::Logging::global_logger->wait_for_empty_queue();  // wait here for logging facility to make
                                                           // sure everything was printed
  return retval;
}

class StackWalkerLogger : public StackWalker
{
public:
  StackWalkerLogger( int options ) : StackWalker( options ){};
  virtual ~StackWalkerLogger()
  {
    if ( !_log.empty() )
      POLLOG_ERROR( _log );
  }
  std::string _log;

protected:
  // no output
  virtual void OnSymInit( LPCSTR szSearchPath, DWORD symOptions, LPCSTR szUserName ) override {}
  virtual void OnLoadModule( LPCSTR img, LPCSTR mod, DWORD64 baseAddr, DWORD size, DWORD result,
                             LPCSTR symType, LPCSTR pdbName, ULONGLONG fileVersion ) override
  {
  }
  virtual void OnDbgHelpErr( LPCSTR szFuncName, DWORD gle, DWORD64 addr ) override{};
  virtual void OnOutput( LPCSTR szText ) override { _log += szText; }
  virtual void OnCallstackEntry( CallstackEntryType eType, CallstackEntry& entry ) override
  {
    try
    {
      if ( ( eType != lastEntry ) && ( entry.offset != 0 ) )
      {
        if ( entry.undFullName[0] != 0 )
          _log += entry.undFullName;
        else if ( entry.undName[0] != 0 )
          _log += entry.undName;
        else
          _log += "(function-name not available)";
        fmt::format_to( std::back_inserter( _log ), " - {:p}\n", (LPVOID)entry.offset );

        if ( entry.lineFileName[0] == 0 )
        {
          _log += "  (filename not available)\n";
        }
        else
        {
          fmt::format_to( std::back_inserter( _log ), "  {} : {}\n", entry.lineFileName,
                          entry.lineNumber );
        }
      }
    }
    catch ( std::exception& e )
    {
      POLLOG_ERRORLN( "failed to format backtrace {}", e.what() );
    }
  }
};

void HiddenMiniDumper::print_backtrace()
{
  {
    StackWalkerLogger sw( StackWalker::RetrieveLine );
    sw._log +=
        "\n##########################################################\nCurrent StackBackTrace\n";
    sw.ShowCallstack();
    // threadhelp::ThreadMap::Contents contents;
    // threadhelp::threadmap.CopyContents( contents );
    // for ( auto &content : contents )
    //{
    //  if ( content.second == "Main" ) // fixme main thread seems to be not suspendable
    //    continue;
    //  HANDLE handle = threadhelp::threadmap.getThreadHandle( content.first );
    //  if ( handle == GetCurrentThread() )
    //    continue;
    //  sw._log << "\nThread ID " << content.first << " (" << content.second << ")\n";
    //  if ( handle )
    //    sw.ShowCallstack( handle);
    //}
    sw._log += "##########################################################\n";
  }
}
}  // namespace Clib
}  // namespace Pol
