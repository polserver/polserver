#ifndef CLIB_WALLCLOCK_H
#define CLIB_WALLCLOCK_H

namespace Pol {
  namespace Clib {
	typedef unsigned int wallclock_t;
	typedef unsigned int wallclock_diff_t;

	wallclock_t wallclock();
	wallclock_diff_t wallclock_diff_ms( wallclock_t start, wallclock_t finish );
  }
}
#endif //CLIB_WALLCLOCK_H
