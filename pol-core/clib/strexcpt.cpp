/** @file
 *
 * @par History
 * - 2005/09/16 Shinigami: added scripts_thread_script* to support better debugging
 *                         backtrace will be printed in pol.log too
 * - 2009-07-18 MuadDib: Updated sigfault messages to point to bug tracker rather than mail to old
 * yahoo groups.
 */


#include "Program/ProgramConfig.h"
#include "logfacility.h"
#include "pol_global_config.h"

#if defined( WINDOWS )
#include "Header_Windows.h"
#include "msjexhnd.h"
#include "strexcpt.h"
#include <stdio.h>
#else
#include "passert.h"
#include <execinfo.h>
#include <signal.h>
#include <unistd.h>
#endif

namespace Pol
{
namespace Clib
{
#ifdef _WIN32
#ifndef _M_X64
// TODO: move the following parts to ExceptionParser.cpp

void se_trans_func( unsigned int n, EXCEPTION_POINTERS* ex )
{
  std::string tmp = fmt::format(
      "Structured Exception Detected: {:#X}\n"
      "ExceptionRecord:\n"
      "  Exception Code:    {:#X}\n"
      "  Exception Flags:   {:#X}\n"
      "  Exception Record:  {:#X}\n"
      "  Exception Address: {:#X}\n"
      "  NumberParameters:  {:#X}\n",
      n, ex->ExceptionRecord->ExceptionCode, ex->ExceptionRecord->ExceptionFlags,
      (long long)( reinterpret_cast<const void*>( ex->ExceptionRecord->ExceptionRecord ) ),
      (long long)( reinterpret_cast<const void*>( ex->ExceptionRecord->ExceptionAddress ) ),
      ex->ExceptionRecord->NumberParameters );

  if ( ex->ExceptionRecord->NumberParameters )
  {
    tmp += "  Exception Information:";
    for ( DWORD i = 0; i < ex->ExceptionRecord->NumberParameters; i++ )
    {
      fmt::format_to( std::back_inserter( tmp ), "{}{:#X}",
                      ( ( i & 7 ) == 0 ) ? "\n  " : "  ",  // print newline every 8 numbers
                      ex->ExceptionRecord->ExceptionInformation[i] );
    }
    tmp += '\n';
  }
  POLLOG_INFO( tmp );


  POLLOG_INFOLN(
      "ContextRecord:\n"
      "  ContextFlags: {:#X}",
      ex->ContextRecord->ContextFlags );
  if ( ex->ContextRecord->ContextFlags & CONTEXT_DEBUG_REGISTERS )
  {
    POLLOG_INFOLN(
        "  CONTEXT_DEBUG_REGISTERS:\n"
        "       Dr0: {:#X}\n"
        "       Dr1: {:#X}\n"
        "       Dr2: {:#X}\n"
        "       Dr3: {:#X}\n"
        "       Dr6: {:#X}\n"
        "       Dr7: {:#X}",
        ex->ContextRecord->Dr0, ex->ContextRecord->Dr1, ex->ContextRecord->Dr2,
        ex->ContextRecord->Dr3, ex->ContextRecord->Dr6, ex->ContextRecord->Dr7 );
  }
  if ( ex->ContextRecord->ContextFlags & CONTEXT_SEGMENTS )
  {
    POLLOG_INFOLN(
        "  CONTEXT_SEGMENTS:\n"
        "     SegGs: {:#X}\n"
        "     SegFs: {:#X}\n"
        "     SegEs: {:#X}\n"
        "     SegDs: {:#X}",
        ex->ContextRecord->SegGs, ex->ContextRecord->SegFs, ex->ContextRecord->SegEs,
        ex->ContextRecord->SegDs );
  }
  if ( ex->ContextRecord->ContextFlags & CONTEXT_INTEGER )
  {
    POLLOG_INFOLN(
        "  CONTEXT_INTEGER:\n"
        "       Edi: {:#X}\n"
        "       Esi: {:#X}\n"
        "       Ebx: {:#X}\n"
        "       Edx: {:#X}\n"
        "       Ecx: {:#X}\n"
        "       Eax: {:#X}",
        ex->ContextRecord->Edi, ex->ContextRecord->Esi, ex->ContextRecord->Ebx,
        ex->ContextRecord->Edx, ex->ContextRecord->Ecx, ex->ContextRecord->Eax );
  }
  if ( ex->ContextRecord->ContextFlags & CONTEXT_CONTROL )
  {
    POLLOG_INFOLN(
        "  CONTEXT_CONTROL:\n"
        "       Ebp: {:#X}\n"
        "       Eip: {:#X}\n"
        "     SegCs: {:#X}\n"
        "    EFlags: {:#X}\n"
        "       Esp: {:#X}\n"
        "     SegSs: {:#X}",
        ex->ContextRecord->Ebp, ex->ContextRecord->Eip, ex->ContextRecord->SegCs,
        ex->ContextRecord->EFlags, ex->ContextRecord->Esp, ex->ContextRecord->SegSs );

    POLLOG_INFOLN( "    Stack Backtrace:" );
    // int frames = 3;
    DWORD* ebp = (DWORD*)ex->ContextRecord->Ebp;
    DWORD eip = ex->ContextRecord->Eip;
    DWORD* new_ebp;
    DWORD new_eip;


    std::string tmp = "    EIP        RETADDR\n";
    int levelsleft = 100;
    while ( ebp && levelsleft-- )
    {
      fmt::format_to( std::back_inserter( tmp ),

                      "    {:#X}", eip );

      if ( !IsBadReadPtr( ebp, sizeof *ebp ) )
      {
        new_ebp = (DWORD*)*ebp;
      }
      else
      {
        tmp += "[Can't read stack!]\n";
        break;
      }
      if ( !IsBadReadPtr( ebp + 1, sizeof *ebp ) )
      {
        fmt::format_to( std::back_inserter( tmp ), "    {:#X}", *( ebp + 1 ) );
        new_eip = *( ebp + 1 );
      }
      else
      {
        tmp += "[Can't read stack!]\n";
        break;
      }

      ebp = new_ebp;
      eip = new_eip;
      tmp += "\n";
    }
    POLLOG_INFO( tmp );
  }

  if ( n == EXCEPTION_ACCESS_VIOLATION )
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
  INFO_PRINTLN( "In trans_func." );
  if ( in_ex_handler )
  {
    POLLOG_INFOLN( "recursive structured exception" );
  }
  else
  {
    in_ex_handler = true;
    POLLOG_INFOLN( "Structured exception compiled on {}", ProgramConfig::build_datetime() );

    MSJExceptionHandler::MSJUnhandledExceptionFilter( pExp );
    in_ex_handler = false;
  }
  throw structured_exception( u );
}

void InstallOldStructuredExceptionHandler( void )
{
  if ( !IsDebuggerPresent() )
  {
    _set_se_translator( alt_se_trans_func );
  }
}

#endif
}  // namespace Clib
}  // namespace Pol
