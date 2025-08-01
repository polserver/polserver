/** @file
 *
 * @par History
 */


#pragma warning( disable : 4189 )  // local variable is initialized but not referenced
#pragma warning( disable : 4312 )  // cast trunc
#pragma warning( disable : 4311 )  // cast trunc
#pragma warning( disable : 4302 )  // cast trunc

//==========================================
// Matt Pietrek
// Microsoft Systems Journal, May 1997
// FILE: MSJEXHND.CPP
//==========================================

#include "msjexhnd.h"
#include "logfacility.h"
#include <algorithm>
#include <imagehlp.h>
#include <stdio.h>
#include <tchar.h>

namespace Pol
{
namespace Clib
{
//============================== Global Variables =============================

//
// Declare the static variables of the MSJExceptionHandler class
//
TCHAR MSJExceptionHandler::m_szLogFileName[MAX_PATH];
LPTOP_LEVEL_EXCEPTION_FILTER MSJExceptionHandler::m_previousFilter;
HANDLE MSJExceptionHandler::m_hReportFile;

MSJExceptionHandler::SYMINITIALIZEPROC MSJExceptionHandler::_SymInitialize = 0;
MSJExceptionHandler::SYMCLEANUPPROC MSJExceptionHandler::_SymCleanup = 0;
MSJExceptionHandler::STACKWALKPROC MSJExceptionHandler::_StackWalk = 0;

MSJExceptionHandler::SYMFUNCTIONTABLEACCESSPROC MSJExceptionHandler::_SymFunctionTableAccess = 0;

MSJExceptionHandler::SYMGETMODULEBASEPROC MSJExceptionHandler::_SymGetModuleBase = 0;

MSJExceptionHandler::SYMGETSYMFROMADDRPROC MSJExceptionHandler::_SymGetSymFromAddr = 0;

MSJExceptionHandler g_MSJExceptionHandler;  // Declare global instance of class

//============================== Class Methods =============================

//=============
// Constructor
//=============
MSJExceptionHandler::MSJExceptionHandler()
{
  // Install the unhandled exception filter function

  // m_previousFilter = SetUnhandledExceptionFilter(MSJUnhandledExceptionFilter);

  // Figure out what the report file will be named, and store it away
  GetModuleFileName( 0, m_szLogFileName, MAX_PATH );

  // Look for the '.' before the "EXE" extension.  Replace the extension
  // with "RPT"
  PTSTR pszDot = _tcsrchr( m_szLogFileName, _T( '.' ) );
  if ( pszDot )
  {
    pszDot++;  // Advance past the '.'
    if ( _tcslen( pszDot ) >= 3 )
      _tcscpy( pszDot, _T( "RPT" ) );  // "RPT" -> "Report"
  }
}

//============
// Destructor
//============
MSJExceptionHandler::~MSJExceptionHandler()
{
  // SetUnhandledExceptionFilter( m_previousFilter );
}

//==============================================================
// Lets user change the name of the report file to be generated
//==============================================================
void MSJExceptionHandler::SetLogFileName( PTSTR pszLogFileName )
{
  if ( _tcslen( pszLogFileName ) <= 260 )
  {
    _tcscpy( m_szLogFileName, pszLogFileName );
  }
}

//===========================================================
// Entry point where control comes on an unhandled exception
//===========================================================
LONG WINAPI MSJExceptionHandler::MSJUnhandledExceptionFilter( PEXCEPTION_POINTERS pExceptionInfo )
{
  m_hReportFile =
      CreateFile( m_szLogFileName, GENERIC_WRITE, 0, 0, OPEN_ALWAYS, FILE_FLAG_WRITE_THROUGH, 0 );

  if ( m_hReportFile )
  {
    SetFilePointer( m_hReportFile, 0, 0, FILE_END );

    GenerateExceptionReport( pExceptionInfo );

    CloseHandle( m_hReportFile );
    m_hReportFile = 0;
  }

  if ( m_previousFilter )
    return m_previousFilter( pExceptionInfo );
  else
    return EXCEPTION_CONTINUE_SEARCH;
}

//===========================================================================
// Open the report file, and write the desired information to it.  Called by
// MSJUnhandledExceptionFilter
//===========================================================================
void MSJExceptionHandler::GenerateExceptionReport( PEXCEPTION_POINTERS pExceptionInfo )
{
  // Start out with a banner
  tprintf( _T( "//=====================================================\n" ) );

  PEXCEPTION_RECORD pExceptionRecord = pExceptionInfo->ExceptionRecord;

  // First print information about the type of fault
  tprintf( _T( "Exception code: %08X %s\n" ), pExceptionRecord->ExceptionCode,
           GetExceptionString( pExceptionRecord->ExceptionCode ) );

  // Now print information about where the fault occured
  TCHAR szFaultingModule[MAX_PATH];
  DWORD section, offset;
  GetLogicalAddress( pExceptionRecord->ExceptionAddress, szFaultingModule,
                     sizeof( szFaultingModule ), section, offset );

  tprintf( _T( "Fault address:  %08X %02X:%08X %s\n" ), (size_t)pExceptionRecord->ExceptionAddress,
           section, offset, szFaultingModule );

  PCONTEXT pCtx = pExceptionInfo->ContextRecord;

// Show the registers
#ifdef _M_IX86  // Intel Only!
  tprintf( _T("\nRegisters:\n") );

  tprintf( _T("EAX:%08X\nEBX:%08X\nECX:%08X\nEDX:%08X\nESI:%08X\nEDI:%08X\n"), pCtx->Eax, pCtx->Ebx,
           pCtx->Ecx, pCtx->Edx, pCtx->Esi, pCtx->Edi );

  tprintf( _T("CS:EIP:%04X:%08X\n"), pCtx->SegCs, pCtx->Eip );
  tprintf( _T("SS:ESP:%04X:%08X  EBP:%08X\n"), pCtx->SegSs, pCtx->Esp, pCtx->Ebp );
  tprintf( _T("DS:%04X  ES:%04X  FS:%04X  GS:%04X\n"), pCtx->SegDs, pCtx->SegEs, pCtx->SegFs,
           pCtx->SegGs );
  tprintf( _T("Flags:%08X\n"), pCtx->EFlags );

#endif

  if ( !InitImagehlpFunctions() )
  {
    OutputDebugString( _T( "IMAGEHLP.DLL or its exported procs not found" ) );

#ifdef _M_IX86  // Intel Only!
#ifndef _M_X64
    // Walk the stack using x86 specific code
    IntelStackWalk( pCtx );
#endif
#endif

    return;
  }
#ifndef _M_X64
  ImagehlpStackWalk( pCtx );
#endif

  _SymCleanup( GetCurrentProcess() );

  tprintf( _T( "\n" ) );
}

//======================================================================
// Given an exception code, returns a pointer to a static string with a
// description of the exception
//======================================================================
LPCTSTR MSJExceptionHandler::GetExceptionString( DWORD dwCode )
{
#define EXCEPTION( x ) \
  case EXCEPTION_##x:  \
    return _T( #x );

  switch ( dwCode )
  {
    EXCEPTION( ACCESS_VIOLATION )
    EXCEPTION( DATATYPE_MISALIGNMENT )
    EXCEPTION( BREAKPOINT )
    EXCEPTION( SINGLE_STEP )
    EXCEPTION( ARRAY_BOUNDS_EXCEEDED )
    EXCEPTION( FLT_DENORMAL_OPERAND )
    EXCEPTION( FLT_DIVIDE_BY_ZERO )
    EXCEPTION( FLT_INEXACT_RESULT )
    EXCEPTION( FLT_INVALID_OPERATION )
    EXCEPTION( FLT_OVERFLOW )
    EXCEPTION( FLT_STACK_CHECK )
    EXCEPTION( FLT_UNDERFLOW )
    EXCEPTION( INT_DIVIDE_BY_ZERO )
    EXCEPTION( INT_OVERFLOW )
    EXCEPTION( PRIV_INSTRUCTION )
    EXCEPTION( IN_PAGE_ERROR )
    EXCEPTION( ILLEGAL_INSTRUCTION )
    EXCEPTION( NONCONTINUABLE_EXCEPTION )
    EXCEPTION( STACK_OVERFLOW )
    EXCEPTION( INVALID_DISPOSITION )
    EXCEPTION( GUARD_PAGE )
    EXCEPTION( INVALID_HANDLE )
  }

  // If not one of the "known" exceptions, try to get the string
  // from NTDLL.DLL's message table.

  static TCHAR szBuffer[512] = { 0 };

  FormatMessage( FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE,
                 GetModuleHandle( _T( "NTDLL.DLL" ) ), dwCode, 0, szBuffer, sizeof( szBuffer ), 0 );

  return szBuffer;
}

//==============================================================================
// Given a linear address, locates the module, section, and offset containing
// that address.
//
// Note: the szModule paramater buffer is an output buffer of length specified
// by the len parameter (in characters!)
//==============================================================================
BOOL MSJExceptionHandler::GetLogicalAddress( PVOID addr, PTSTR szModule, DWORD len, DWORD& section,
                                             DWORD& offset )
{
  MEMORY_BASIC_INFORMATION mbi;

  if ( !VirtualQuery( addr, &mbi, sizeof( mbi ) ) )
    return FALSE;

  DWORD hMod = (DWORD)mbi.AllocationBase;

  if ( !GetModuleFileName( (HMODULE)hMod, szModule, len ) )
    return FALSE;

  // Point to the DOS header in memory
  PIMAGE_DOS_HEADER pDosHdr = (PIMAGE_DOS_HEADER)hMod;

  // From the DOS header, find the NT (PE) header
  PIMAGE_NT_HEADERS pNtHdr = (PIMAGE_NT_HEADERS)( hMod + pDosHdr->e_lfanew );

  PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION( pNtHdr );

  DWORD rva = (DWORD)addr - hMod;  // RVA is offset from module load address

  // Iterate through the section table, looking for the one that encompasses
  // the linear address.
  for ( unsigned i = 0; i < pNtHdr->FileHeader.NumberOfSections; i++, pSection++ )
  {
    DWORD sectionStart = pSection->VirtualAddress;
    DWORD sectionEnd =
        sectionStart + std::max( pSection->SizeOfRawData, pSection->Misc.VirtualSize );

    // Is the address in this section???
    if ( ( rva >= sectionStart ) && ( rva <= sectionEnd ) )
    {
      // Yes, address is in the section.  Calculate section and offset,
      // and store in the "section" & "offset" params, which were
      // passed by reference.
      section = i + 1;
      offset = rva - sectionStart;
      return TRUE;
    }
  }

  return FALSE;  // Should never get here!
}

//============================================================
// Walks the stack, and writes the results to the report file
//============================================================
#ifndef _M_X64
void MSJExceptionHandler::IntelStackWalk( PCONTEXT pContext )
{
  tprintf( _T("\nCall stack:\n") );

  tprintf( _T("Address   Frame     Logical addr  Module\n") );

  DWORD pc = pContext->Eip;
  PDWORD pFrame, pPrevFrame;

  pFrame = (PDWORD)pContext->Ebp;

  do
  {
    TCHAR szModule[MAX_PATH] = _T("");
    DWORD section = 0, offset = 0;

    GetLogicalAddress( (PVOID)pc, szModule, sizeof( szModule ), section, offset );

    tprintf( _T("%08X  %08X  %04X:%08X %s\n"), pc, pFrame, section, offset, szModule );

    pc = pFrame[1];

    pPrevFrame = pFrame;

    pFrame = (PDWORD)pFrame[0];  // proceed to next higher frame on stack

    if ( (DWORD)pFrame & 3 )  // Frame pointer must be aligned on a
      break;                  // DWORD boundary.  Bail if not so.

    if ( pFrame <= pPrevFrame )
      break;

    // Can two DWORDs be read from the supposed frame address?
    if ( IsBadWritePtr( pFrame, sizeof( PVOID ) * 2 ) )
      break;

  } while ( 1 );
}
#endif

//============================================================
// Walks the stack, and writes the results to the report file
//============================================================
#ifndef _M_X64
void MSJExceptionHandler::ImagehlpStackWalk( PCONTEXT pContext )
{
  tprintf( _T("\nCall stack:\n") );

  tprintf( _T("Address   Frame\n") );

  // Could use SymSetOptions here to add the SYMOPT_DEFERRED_LOADS flag

  STACKFRAME sf;
  memset( &sf, 0, sizeof( sf ) );

  // Initialize the STACKFRAME structure for the first call.  This is only
  // necessary for Intel CPUs, and isn't mentioned in the documentation.
  sf.AddrPC.Offset = pContext->Eip;
  sf.AddrPC.Mode = AddrModeFlat;
  sf.AddrStack.Offset = pContext->Esp;
  sf.AddrStack.Mode = AddrModeFlat;
  sf.AddrFrame.Offset = pContext->Ebp;
  sf.AddrFrame.Mode = AddrModeFlat;

  while ( 1 )
  {
    if ( !_StackWalk( IMAGE_FILE_MACHINE_I386, GetCurrentProcess(), GetCurrentThread(), &sf,
                      pContext, 0, _SymFunctionTableAccess, _SymGetModuleBase, 0 ) )
      break;

    if ( 0 == sf.AddrFrame.Offset )  // Basic sanity check to make sure
      break;                         // the frame is OK.  Bail if not.

    tprintf( _T("%08X  %08X  "), sf.AddrPC.Offset, sf.AddrFrame.Offset );

    // IMAGEHLP is wacky, and requires you to pass in a pointer to an
    // IMAGEHLP_SYMBOL structure.  The problem is that this structure is
    // variable length.  That is, you determine how big the structure is
    // at runtime.  This means that you can't use sizeof(struct).
    // So...make a buffer that's big enough, and make a pointer
    // to the buffer.  We also need to initialize not one, but TWO
    // members of the structure before it can be used.

    BYTE symbolBuffer[sizeof( IMAGEHLP_SYMBOL ) + 512];
    PIMAGEHLP_SYMBOL pSymbol = (PIMAGEHLP_SYMBOL)symbolBuffer;
    pSymbol->SizeOfStruct = sizeof( symbolBuffer );
    pSymbol->MaxNameLength = 512;

    DWORD symDisplacement = 0;  // Displacement of the input address,
    // relative to the start of the symbol

    if ( _SymGetSymFromAddr( GetCurrentProcess(), sf.AddrPC.Offset, &symDisplacement, pSymbol ) )
    {
      tprintf( _T("%hs+%X\n"), pSymbol->Name, symDisplacement );
    }
    else  // No symbol found.  Print out the logical address instead.
    {
      TCHAR szModule[MAX_PATH] = _T("");
      DWORD section = 0, offset = 0;

      GetLogicalAddress( (PVOID)sf.AddrPC.Offset, szModule, sizeof( szModule ), section, offset );

      tprintf( _T("%04X:%08X %s\n"), section, offset, szModule );
    }
  }
}
#endif

//============================================================================
// Helper function that writes to the report file, and allows the user to use
// printf style formating
//============================================================================
int __cdecl MSJExceptionHandler::tprintf( const TCHAR* format, ... )
{
  TCHAR szBuff[1024];
  int retValue;
  // DWORD cbWritten;
  va_list argptr;

  va_start( argptr, format );
  retValue = wvsprintf( szBuff, format, argptr );
  va_end( argptr );

  //    WriteFile( m_hReportFile, szBuff, retValue * sizeof(TCHAR), &cbWritten, 0 );
  POLLOG_INFO( &szBuff[0] );

  return retValue;
}


//=========================================================================
// Load IMAGEHLP.DLL and get the address of functions in it that we'll use
//=========================================================================
BOOL MSJExceptionHandler::InitImagehlpFunctions( void )
{
  HMODULE hModImagehlp = LoadLibrary( _T( "IMAGEHLP.DLL" ) );
  if ( !hModImagehlp )
    return FALSE;

  _SymInitialize = (SYMINITIALIZEPROC)GetProcAddress( hModImagehlp, "SymInitialize" );
  if ( !_SymInitialize )
  {
    FreeLibrary( hModImagehlp );
    return FALSE;
  }


  _SymCleanup = (SYMCLEANUPPROC)GetProcAddress( hModImagehlp, "SymCleanup" );
  if ( !_SymCleanup )
  {
    FreeLibrary( hModImagehlp );
    return FALSE;
  }

  _StackWalk = (STACKWALKPROC)GetProcAddress( hModImagehlp, "StackWalk" );
  if ( !_StackWalk )
  {
    FreeLibrary( hModImagehlp );
    return FALSE;
  }

  _SymFunctionTableAccess =
      (SYMFUNCTIONTABLEACCESSPROC)GetProcAddress( hModImagehlp, "SymFunctionTableAccess" );

  if ( !_SymFunctionTableAccess )
  {
    FreeLibrary( hModImagehlp );
    return FALSE;
  }

  _SymGetModuleBase = (SYMGETMODULEBASEPROC)GetProcAddress( hModImagehlp, "SymGetModuleBase" );
  if ( !_SymGetModuleBase )
  {
    FreeLibrary( hModImagehlp );
    return FALSE;
  }

  _SymGetSymFromAddr = (SYMGETSYMFROMADDRPROC)GetProcAddress( hModImagehlp, "SymGetSymFromAddr" );
  if ( !_SymGetSymFromAddr )
  {
    FreeLibrary( hModImagehlp );
    return FALSE;
  }

  if ( !_SymInitialize( GetCurrentProcess(), 0, TRUE ) )
  {
    FreeLibrary( hModImagehlp );
    return FALSE;
  }

  return TRUE;
}
}  // namespace Clib
}  // namespace Pol
