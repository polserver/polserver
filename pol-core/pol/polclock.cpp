/** @file
 *
 * @par History
 */


#include "polclock.h"

#include <atomic>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif

#include "globals/state.h"

namespace Pol
{
namespace Core
{
#ifdef _WIN32
typedef clock_t polclock_base_type;
#else
typedef int polclock_base_type;
#endif
static std::atomic<polclock_base_type> polclock_base( 0 );
static std::atomic<time_t> poltime_base( 0 );
static std::atomic<time_t> poltime_paused_at( 0 );

polclock_base_type getCurrentPolClockSeconds()
{
#ifdef _WIN32
  return clock();
#else
  struct timeval tmv;
  struct timezone tz;
  gettimeofday( &tmv, &tz );
  return tmv.tv_sec;
#endif
}

void pol_sleep_ms( unsigned int millis )
{
#ifdef _WIN32
  Sleep( millis );
#else
  usleep( millis * 1000L );
#endif
}

void start_polclock()
{
  polclock_base = getCurrentPolClockSeconds();
}

void pause_polclock()
{
  stateManager.polclock_paused_at = getCurrentPolClockSeconds();
}

void restart_polclock()
{
  polclock_base_type polclock_diff = getCurrentPolClockSeconds() - stateManager.polclock_paused_at;
  polclock_base += polclock_diff;
  stateManager.polclock_paused_at = 0;
}

polclock_t polclock()
{
#ifdef _WIN32
#ifndef POLCLOCK_STRETCH
  return ( clock() - polclock_base ) / POLCLOCK_DIV;
#else
  return ( clock() - polclock_base ) / ( POLCLOCK_DIV * POLCLOCK_STRETCH );
#endif
#else
  struct timeval tmv;
  struct timezone tz;
  gettimeofday( &tmv, &tz );
  return ( tmv.tv_sec - polclock_base ) * 100 + tmv.tv_usec / ( 1000L * 10L );
#endif
}

void start_poltime()
{
  poltime_base = time( NULL );
}
void pause_poltime()
{
  poltime_paused_at = time( NULL );
}
void restart_poltime()
{
  time_t poltime_diff = time( NULL ) - poltime_paused_at;
  poltime_base += poltime_diff;
}

time_t poltime()
{
#ifndef POLCLOCK_STRETCH
  return time( NULL ) - poltime_base;
#else
  return ( time( NULL ) - poltime_base ) / POLCLOCK_STRETCH;
#endif
}

void start_pol_clocks()
{
  static bool inited = false;
  if ( !inited )
  {
    inited = true;
    start_polclock();
    start_poltime();
  }
}
void pause_pol_clocks()
{
  pause_polclock();
  pause_poltime();
}
void restart_pol_clocks()
{
  restart_polclock();
  restart_poltime();
}
}
}
