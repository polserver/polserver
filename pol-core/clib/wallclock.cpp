/*
History
=======

Notes
=======

*/

#include "stl_inc.h"

#include <time.h>

#include "wallclock.h"

#ifndef _WIN32
#include <sys/time.h>
#include <unistd.h>
#endif

namespace Pol {
  namespace Clib {
#ifdef _WIN32
	wallclock_t wallclock()
	{
	  return clock();
	}
	wallclock_diff_t wallclock_diff_ms( wallclock_t start, wallclock_t finish )
	{
	  return ( ( finish - start ) * 1000L / CLOCKS_PER_SEC );
	}

#else

	wallclock_t wallclock()
	{
	  struct timeval tmv;
	  struct timezone tz;
	  gettimeofday( &tmv, &tz );
	  return (tmv.tv_sec) * 1000 + tmv.tv_usec /1000L;
	}
	wallclock_diff_t wallclock_diff_ms( wallclock_t start, wallclock_t finish )
	{
	  return finish - start;
	}

#endif
  }
}
