/*
History
=======


Notes
=======

*/

#include "polclock.h"

#ifdef _WIN32
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif

#include "../clib/passert.h"

#include "uvars.h"


namespace Pol {
  namespace Core {
#ifdef _WIN32

	void pol_sleep_ms( unsigned int millis )
	{
	  Sleep( millis );
	}

	static clock_t polclock_base;

	void start_polclock()
	{
	  polclock_base = clock();
	}

	void pause_polclock()
	{
	  gamestate.polclock_paused_at = clock();
	}

	void restart_polclock()
	{
	  clock_t polclock_diff = clock() - gamestate.polclock_paused_at;

	  polclock_base += polclock_diff;
	  gamestate.polclock_paused_at = 0;
	}

#ifndef POLCLOCK_STRETCH
	polclock_t polclock()
	{
	  return ( clock() - polclock_base ) / POLCLOCK_DIV;
	}
#else
	polclock_t polclock()
	{
	  return (clock() - polclock_base) / (POLCLOCK_DIV * POLCLOCK_STRETCH);
	}
#endif // POLCLOCK_STRETCH

#else




	void pol_sleep_ms( unsigned int millis )
	{
	  usleep( millis * 1000L );
	}

	static int base_seconds = 0;

	void start_polclock()
	{
	  struct timeval tmv;
	  struct timezone tz;
	  gettimeofday( &tmv, &tz );
	  base_seconds = tmv.tv_sec;
	}

	void pause_polclock()
	{
	  struct timeval tmv;
	  struct timezone tz;
	  gettimeofday( &tmv, &tz );
	  gamestate.polclock_paused_at = tmv.tv_sec;
	}

	void restart_polclock()
	{
	  struct timeval tmv;
	  struct timezone tz;
	  gettimeofday( &tmv, &tz );

	  int polclock_diff = tmv.tv_sec - gamestate.polclock_paused_at;

	  base_seconds += polclock_diff;
	  gamestate.polclock_paused_at = 0;
	}

	polclock_t polclock()
	{
	  struct timeval tmv;
	  struct timezone tz;
	  gettimeofday( &tmv, &tz );
	  return (tmv.tv_sec - base_seconds) * 100 + tmv.tv_usec /(1000L * 10L);
	}

#endif




	static time_t poltime_base;
	static time_t poltime_paused_at;

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

#ifndef POLCLOCK_STRETCH
	time_t poltime()
	{
	  return time( NULL ) - poltime_base;
	}
#else
	time_t poltime()
	{
	  return (time(NULL) - poltime_base) / POLCLOCK_STRETCH;
	}
#endif // POLCLOCK_STRETCH






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