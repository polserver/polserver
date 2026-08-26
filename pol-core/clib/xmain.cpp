/** @file
 *
 * @par History
 * - 2009/07/20 MuadDib: Removed calls and use of StackWalker for leak detection. vld is now used,
 * much better.
 *                       Removed StackWalker.cpp/.h from the vcproj files also.
 */


#include <algorithm>
#include <stddef.h>
#include <stdio.h>
#include <thread>
#if defined( WINDOWS )
#include "clib/Header_Windows.h"

#pragma comment( lib, "psapi.lib" )  // 32bit is a bit dumb..
#elif defined( __APPLE__ )
#include <mach/mach.h>
#else
#include <sched.h>
#include <unistd.h>
#endif

#include "clib/clib.h"


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

#if defined( __linux__ )
namespace
{
/// The cgroup cpu quota as a whole number of cpus, or 0 when there is no quota to speak of.
/// cgroup v2 states it as "<quota> <period>" or "max <period>" in cpu.max; v1 splits it over
/// cpu.cfs_quota_us (-1 for none) and cpu.cfs_period_us.
unsigned int cgroup_cpu_quota()
{
  if ( FILE* fp = fopen( "/sys/fs/cgroup/cpu.max", "r" ) )
  {
    char quota[32] = { 0 };
    long period = 0;
    const int read = fscanf( fp, "%31s %ld", quota, &period );
    fclose( fp );
    if ( read == 2 && period > 0 && quota[0] != 'm' )  // "max" means unlimited
    {
      const long limit = atol( quota );
      if ( limit > 0 )
        return static_cast<unsigned int>( ( limit + period - 1 ) / period );
    }
    return 0;
  }

  long quota = -1;
  long period = 0;
  if ( FILE* fp = fopen( "/sys/fs/cgroup/cpu/cpu.cfs_quota_us", "r" ) )
  {
    if ( fscanf( fp, "%ld", &quota ) != 1 )
      quota = -1;
    fclose( fp );
  }
  if ( FILE* fp = fopen( "/sys/fs/cgroup/cpu/cpu.cfs_period_us", "r" ) )
  {
    if ( fscanf( fp, "%ld", &period ) != 1 )
      period = 0;
    fclose( fp );
  }
  if ( quota > 0 && period > 0 )
    return static_cast<unsigned int>( ( quota + period - 1 ) / period );
  return 0;
}

/// How many cpus the affinity mask leaves us, or 0 if it cannot be read. sysconf() and
/// hardware_concurrency() both ignore this, so a cpuset is invisible to them.
unsigned int affinity_cpus()
{
  cpu_set_t set;
  CPU_ZERO( &set );
  if ( sched_getaffinity( 0, sizeof( set ), &set ) != 0 )
    return 0;
  const int count = CPU_COUNT( &set );
  return count > 0 ? static_cast<unsigned int>( count ) : 0;
}
}  // namespace
#endif

unsigned int available_cpus()
{
  unsigned int cpus = std::thread::hardware_concurrency();
  if ( !cpus )
    cpus = 1;
#if defined( __linux__ )
  if ( const unsigned int affinity = affinity_cpus() )
    cpus = std::min( cpus, affinity );
  if ( const unsigned int quota = cgroup_cpu_quota() )
    cpus = std::min( cpus, quota );
#endif
  return std::max( 1u, cpus );
}
}  // namespace Pol::Clib
