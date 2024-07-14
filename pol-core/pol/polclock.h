/** @file
 *
 * @par History
 */


/////////////////////////////////////////////////////////////////////////////
//
// polclock.h - clock() and time() wrappers for clock stretching
//
// This is mostly for profiling, so that timed actions (ie attacks) don't
// overwhelm everything else.  Profiled runs seem to run at about 1/20th
// normal speed.  This way, you can run the system such that even though
// the RPMs are, say, 20, the profiling is similar to an RPM=400 system.
//
/////////////////////////////////////////////////////////////////////////////
#ifndef POLCLOCK_H
#define POLCLOCK_H

#include <chrono>

namespace Pol
{
namespace Core
{
typedef std::chrono::milliseconds polclock_t_unit;  // in 10ms
typedef polclock_t_unit::rep polclock_t;            // in 10ms

typedef std::chrono::seconds poltime_t_unit;
typedef poltime_t_unit::rep poltime_t;

typedef std::chrono::steady_clock PolClock;

const polclock_t POLCLOCKS_PER_SEC = 100;

void start_pol_clocks();
void pause_pol_clocks();
void restart_pol_clocks();

// not for production! ;)
void shift_clock_for_unittest( std::chrono::milliseconds milli );

void pol_sleep_ms( unsigned int millis );

inline bool timer_expired( polclock_t timer_until, polclock_t now )
{
  return ( ( timer_until - now ) < 0 );
}
inline polclock_t earliest_timer( polclock_t timer1_until, polclock_t timer2_until )
{
  auto diff = timer1_until - timer2_until;
  if ( diff < 0 )
    return timer1_until;
  else
    return timer2_until;
}

polclock_t polclock();  // unit 10ms
poltime_t poltime();    // unit seconds

bool is_polclock_paused_at_zero();

inline polclock_t polclock_t_to_ms( polclock_t clock )
{
  return clock * 10;
}

class PolClockPauser
{
public:
  PolClockPauser() { pause_pol_clocks(); }
  ~PolClockPauser() { restart_pol_clocks(); }
};

void polclock_checkin();
}  // namespace Core
}  // namespace Pol
#endif
