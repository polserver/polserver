/** @file
 *
 * @par History
 * - 2009/07/20 MuadDib: Removed calls and use of StackWalker for leak detection. vld is now used,
 * much better.
 *                       Removed StackWalker.cpp/.h from the vcproj files also.
 */


#include <stddef.h>
#include <stdio.h>
#if defined( WINDOWS )
#include "Header_Windows.h"

#pragma comment( lib, "psapi.lib" )  // 32bit is a bit dumb..
#else
#include <unistd.h>
#endif


namespace Pol::Clib
{
// TODO: create a system.cpp/h and put the following function with some other features in a separate
// static class "system"
size_t getCurrentMemoryUsage()
{
#if defined( _WIN32 )
  PROCESS_MEMORY_COUNTERS info;
  GetProcessMemoryInfo( GetCurrentProcess(), &info, sizeof( info ) );
  return (size_t)info.WorkingSetSize;

#else
  long rss = 0L;
  FILE* fp = nullptr;
  if ( ( fp = fopen( "/proc/self/statm", "r" ) ) == nullptr )
    return (size_t)0L; /* Can't open? */
  if ( fscanf( fp, "%*s%ld", &rss ) != 1 )
  {
    fclose( fp );
    return (size_t)0L; /* Can't read? */
  }
  fclose( fp );
  return (size_t)rss * (size_t)sysconf( _SC_PAGESIZE );
#endif
}
}  // namespace Pol::Clib
