/*
History
=======


Notes
=======

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

#include <time.h>

typedef long polclock_t;
typedef long polticks_t;

const polclock_t POLCLOCKS_PER_SEC = 100;
const unsigned POLCLOCK_DIV = (CLOCKS_PER_SEC / POLCLOCKS_PER_SEC);

void start_pol_clocks();
void pause_pol_clocks();
void restart_pol_clocks();

void pol_sleep_ms( unsigned long millis );

inline bool timer_expired( polclock_t timer_until, polclock_t now )
{
    long remaining = timer_until - now;
    return (remaining < 0);
}
inline polclock_t earliest_timer( polclock_t timer1_until, polclock_t timer2_until )
{
    long diff = timer1_until - timer2_until;
    if (diff < 0)
        return timer1_until;
    else
        return timer2_until;
}

polclock_t polclock();
time_t poltime();
extern clock_t polclock_paused_at;

inline polticks_t polticks_t_to_ms( polticks_t ticks )
{
    return ticks * 10;
}

class PolClockPauser
{
public:
    PolClockPauser() { pause_pol_clocks(); }
    ~PolClockPauser() { restart_pol_clocks(); }
};

void polclock_checkin();

#endif
