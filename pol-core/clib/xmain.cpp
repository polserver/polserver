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
#include "clib/Header_Windows.h"

#pragma comment( lib, "psapi.lib" )  // 32bit is a bit dumb..
#elif defined( __APPLE__ )
#include <mach/mach.h>
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

#elif defined( __APPLE__ )
  // macOS has no /proc, the resident size comes from the mach task info.
  mach_task_basic_info_data_t info;
  mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
  if ( task_info( mach_task_self(), MACH_TASK_BASIC_INFO, reinterpret_cast<task_info_t>( &info ),
                  &count ) != KERN_SUCCESS )
    return (size_t)0L;
  return (size_t)info.resident_size;

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
