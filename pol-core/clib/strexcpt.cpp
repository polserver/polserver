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
  fmt::Writer tmp;
  tmp << "Structured Exception Detected: 0x" << fmt::hex( n ) << "\n"
      << "ExceptionRecord:\n"
      << "  Exception Code:    0x" << fmt::hex( ex->ExceptionRecord->ExceptionCode ) << "\n"
      << "  Exception Flags:   0x" << fmt::hex( ex->ExceptionRecord->ExceptionFlags ) << "\n"
      << "  Exception Record:  0x"
      << fmt::hex(
             (long long)( reinterpret_cast<const void*>( ex->ExceptionRecord->ExceptionRecord ) ) )
      << "\n"
      << "  Exception Address: 0x"
      << fmt::hex(
             (long long)( reinterpret_cast<const void*>( ex->ExceptionRecord->ExceptionAddress ) ) )
      << "\n"
      << "  NumberParameters:  0x" << fmt::hex( ex->ExceptionRecord->NumberParameters ) << "\n";

  if ( ex->ExceptionRecord->NumberParameters )
  {
    tmp << "  Exception Information:";
    for ( DWORD i = 0; i < ex->ExceptionRecord->NumberParameters; i++ )
    {
      fmt::Writer _tmp;
      _tmp.Format( "{}{:#X}" ) << ( ( ( i & 7 ) == 0 ) ? "\n  "
                                                       : "  " )  // print newline every 8 numbers
                               << ex->ExceptionRecord->ExceptionInformation[i];
      tmp << _tmp.str();
    }
    tmp << "\n";
  }
  POLLOG_INFO << tmp.str();


  POLLOG_INFO << "ContextRecord:\n"
              << "  ContextFlags: 0x" << fmt::hex( ex->ContextRecord->ContextFlags ) << "\n";
  if ( ex->ContextRecord->ContextFlags & CONTEXT_DEBUG_REGISTERS )
  {
    POLLOG_INFO << "  CONTEXT_DEBUG_REGISTERS:\n"
                << "       Dr0: 0x" << fmt::hex( ex->ContextRecord->Dr0 ) << "\n"
                << "       Dr1: 0x" << fmt::hex( ex->ContextRecord->Dr1 ) << "\n"
                << "       Dr2: 0x" << fmt::hex( ex->ContextRecord->Dr2 ) << "\n"
                << "       Dr3: 0x" << fmt::hex( ex->ContextRecord->Dr3 ) << "\n"
                << "       Dr6: 0x" << fmt::hex( ex->ContextRecord->Dr6 ) << "\n"
                << "       Dr7: 0x" << fmt::hex( ex->ContextRecord->Dr7 ) << "\n";
  }
  if ( ex->ContextRecord->ContextFlags & CONTEXT_SEGMENTS )
  {
    POLLOG_INFO << "  CONTEXT_SEGMENTS:\n"
                << "     SegGs: 0x" << fmt::hex( ex->ContextRecord->SegGs ) << "\n"
                << "     SegFs: 0x" << fmt::hex( ex->ContextRecord->SegFs ) << "\n"
                << "     SegEs: 0x" << fmt::hex( ex->ContextRecord->SegEs ) << "\n"
                << "     SegDs: 0x" << fmt::hex( ex->ContextRecord->SegDs ) << "\n";
  }
  if ( ex->ContextRecord->ContextFlags & CONTEXT_INTEGER )
  {
    POLLOG_INFO << "  CONTEXT_INTEGER:\n"
                << "       Edi: 0x" << fmt::hex( ex->ContextRecord->Edi ) << "\n"
                << "       Esi: 0x" << fmt::hex( ex->ContextRecord->Esi ) << "\n"
                << "       Ebx: 0x" << fmt::hex( ex->ContextRecord->Ebx ) << "\n"
                << "       Edx: 0x" << fmt::hex( ex->ContextRecord->Edx ) << "\n"
                << "       Ecx: 0x" << fmt::hex( ex->ContextRecord->Ecx ) << "\n"
                << "       Eax: 0x" << fmt::hex( ex->ContextRecord->Eax ) << "\n";
  }
  if ( ex->ContextRecord->ContextFlags & CONTEXT_CONTROL )
  {
    POLLOG_INFO << "  CONTEXT_CONTROL:\n"
                << "       Ebp: 0x" << fmt::hex( ex->ContextRecord->Ebp ) << "\n"
                << "       Eip: 0x" << fmt::hex( ex->ContextRecord->Eip ) << "\n"
                << "     SegCs: 0x" << fmt::hex( ex->ContextRecord->SegCs ) << "\n"
                << "    EFlags: 0x" << fmt::hex( ex->ContextRecord->EFlags ) << "\n"
                << "       Esp: 0x" << fmt::hex( ex->ContextRecord->Esp ) << "\n"
                << "     SegSs: 0x" << fmt::hex( ex->ContextRecord->SegSs ) << "\n";

    POLLOG_INFO << "    Stack Backtrace:\n";
    // int frames = 3;
    DWORD* ebp = (DWORD*)ex->ContextRecord->Ebp;
    DWORD eip = ex->ContextRecord->Eip;
    DWORD* new_ebp;
    DWORD new_eip;


    fmt::Writer tmp;
    tmp << "    EIP        RETADDR\n";
    int levelsleft = 100;
    while ( ebp && levelsleft-- )
    {
      tmp << "    0x" << fmt::hex( eip );

      if ( !IsBadReadPtr( ebp, sizeof *ebp ) )
      {
        new_ebp = (DWORD*)*ebp;
      }
      else
      {
        tmp << "[Can't read stack!]\n";
        break;
      }
      if ( !IsBadReadPtr( ebp + 1, sizeof *ebp ) )
      {
        tmp << "    0x" << fmt::hex( *( ebp + 1 ) );
        new_eip = *( ebp + 1 );
      }
      else
      {
        tmp << "[Can't read stack!]\n";
        break;
      }

      ebp = new_ebp;
      eip = new_eip;
      tmp << "\n";
    }
    POLLOG_INFO << tmp.str();
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
  INFO_PRINT2( "In trans_func." );
  if ( in_ex_handler )
  {
    POLLOG_INFO << "recursive structured exception\n";
  }
  else
  {
    in_ex_handler = true;
    POLLOG_INFO.Format( "Structured exception in {} compiled on {} at {}\n" )
        << ProgramConfig::build_datetime();

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
