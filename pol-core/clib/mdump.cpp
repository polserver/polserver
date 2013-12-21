/*
History
=======
2005/09/16 Shinigami: added scripts_thread_script* to support better debugging
2009-07-18 MuadDib: Updated dump messages for bug tracker

Notes
=======

*/


#include "stl_inc.h"
#include <windows.h>
#include <assert.h>
#include <time.h>
#include "mdump.h"
#include "progver.h"
#include "logfile.h"
#include "strexcpt.h"
#include "passert.h"

	// FIXME: 2008 Upgrades needed here? Need to check dbg headers to ensure compatibility
#if _MSC_VER < 1300
#	define DECLSPEC_DEPRECATED
	// VC6: change this path to your Platform SDK headers
#	include "C:\\Program Files\\Microsoft Visual Studio\\PlatformSDK\\include\\dbghelp.h"			// must be XP version of file
#else
	// VC7: ships with updated headers
#	include "dbghelp.h"
#endif

// based on dbghelp.h
typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
									CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
									CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
									CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam
									);


#include "mdumpimp.h"
namespace Pol {
  namespace Clib {
	void MiniDumper::Initialize()
	{
	  HiddenMiniDumper::Initialize();
	}
	void MiniDumper::SetMiniDumpType( const string& dumptype )
	{
	  HiddenMiniDumper::_MiniDumpType = dumptype;
	}

	bool HiddenMiniDumper::_Initialized;
	unsigned HiddenMiniDumper::_DumpCount;
	string HiddenMiniDumper::_MiniDumpType;
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

	  time_t now = time( NULL );
	  strftime( _StartTimestamp, sizeof _StartTimestamp, "%Y%m%d%H%M%S", localtime( &now ) );

	  // appname will be obtained from progver

	  // find a better value for your app
	  //HWND hParent = NULL;

	  /*
		  firstly see if dbghelp.dll is around and has the function we need
		  look next to the EXE first, as the one in System32 might be old
		  (e.g. Windows 2000)
		  */
	  char szDbgHelpPath[_MAX_PATH];
	  char* szResult = NULL;

	  if( GetModuleFileName( NULL, szDbgHelpPath, _MAX_PATH ) )
	  {
		char *pSlash = strchr( szDbgHelpPath, '\\' );
		if( pSlash )
		{
		  strcpy( pSlash + 1, "DBGHELP.DLL" );
		  hDbgHelpDll = ::LoadLibrary( szDbgHelpPath );
		}
	  }

	  if( hDbgHelpDll == NULL )
	  {
		// load any version we can
		hDbgHelpDll = ::LoadLibrary( "DBGHELP.DLL" );
	  }
	  if( hDbgHelpDll )
	  {
		MINIDUMPWRITEDUMP pDump = ( MINIDUMPWRITEDUMP )::GetProcAddress( hDbgHelpDll, "MiniDumpWriteDump" );
		if( pDump )
		  ::SetUnhandledExceptionFilter( TopLevelFilter );
		else
		  szResult = "Warning: DBGHELP.DLL too old, version 5.1+ required.";
	  }
	  else
		szResult = "Warning: DBGHELP.DLL not found, version 5.1+ required in POL directory.";

	  if( szResult )
	  {
		Log( szResult );
		printf( "%s\n", szResult );
		InstallOldStructuredExceptionHandler();
	  }
	}

	LONG HiddenMiniDumper::TopLevelFilter( struct _EXCEPTION_POINTERS *pExceptionInfo )
	{
	  LONG retval = EXCEPTION_CONTINUE_SEARCH;
	  char szDumpPath[_MAX_PATH];
	  char* szResult = NULL;
	  char szScratch[_MAX_PATH];

	  MINIDUMPWRITEDUMP pDump = ( MINIDUMPWRITEDUMP )::GetProcAddress( hDbgHelpDll, "MiniDumpWriteDump" );
	  if( pDump )
	  {
		strcpy( szDumpPath, progverstr );
		strcat( szDumpPath, "-" );
		strcat( szDumpPath, _StartTimestamp );
		strcat( szDumpPath, "-" );
		strcat( szDumpPath, itoa( _DumpCount++, szScratch, 16 ) );
		strcat( szDumpPath, ".dmp" );

		HANDLE hFile = ::CreateFile( szDumpPath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
									 FILE_ATTRIBUTE_NORMAL, NULL );

		if( hFile != INVALID_HANDLE_VALUE )
		{
		  _MINIDUMP_EXCEPTION_INFORMATION ExInfo;

		  ExInfo.ThreadId = ::GetCurrentThreadId();
		  ExInfo.ExceptionPointers = pExceptionInfo;
		  ExInfo.ClientPointers = NULL;

		  // write the dump
		  MINIDUMP_TYPE dumptype;
		  if( _MiniDumpType == "large" )
			dumptype = MiniDumpWithFullMemory;
		  else if( _MiniDumpType == "variable" )
			dumptype = MiniDumpWithDataSegs;
		  else
			dumptype = MiniDumpNormal;

		  printf( "Unhandled Exception! Minidump started...\n" );
		  BOOL bOK = pDump( GetCurrentProcess(), GetCurrentProcessId(), hFile, dumptype, &ExInfo, NULL, NULL );
		  if( bOK )
		  {
			sprintf( szScratch, "Unhandled Exception! Writing Minidump file. \nPost this file with explanation and last lines from log files on http://forums.polserver.com/tracker.php for the development team.\nSaved dump file to '%s'\n", szDumpPath );
			szResult = szScratch;
			retval = EXCEPTION_EXECUTE_HANDLER;
		  }
		  else
		  {
			sprintf( szScratch, "Failed to save dump file to '%s' (error %u)", szDumpPath, GetLastError() );
			szResult = szScratch;
		  }
		  ::CloseHandle( hFile );
		}
		else
		{
		  sprintf( szScratch, "Failed to create dump file '%s' (error %u)", szDumpPath, GetLastError() );
		  szResult = szScratch;
		}
	  }

	  FreeLibrary( hDbgHelpDll );

	  if( szResult )
	  {
		Log( szResult );
		Log( "Last Script: %s PC: %u\n", scripts_thread_script.c_str(), scripts_thread_scriptPC );
		cerr << "##########################################################" << endl;
		printf( "%s", szResult );
		cerr << "Last Script: " << scripts_thread_script << " PC: " << scripts_thread_scriptPC << endl;
		cerr << "##########################################################" << endl;
	  }
	  return retval;
	}
  }
}