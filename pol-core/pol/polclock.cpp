/** @file
 *
 * @par History
 */


#include "polclock.h"

#include <atomic>
#include <chrono>
#include <thread>

#include "../clib/logfacility.h"
#include "../clib/spinlock.h"

namespace Pol
{
namespace Core
{
using namespace std::chrono_literals;
static PolClock::time_point polclock_base = PolClock::time_point( PolClock::duration( 0 ) );
static PolClock::time_point poltime_base = PolClock::time_point( PolClock::duration( 0 ) );
static PolClock::time_point poltime_paused_at = PolClock::time_point( PolClock::duration( 0 ) );
static PolClock::time_point polclock_paused_at = PolClock::time_point( PolClock::duration( 0 ) );
static Clib::SpinLock polclock_lock;

static std::chrono::milliseconds unittest_shift = 0ms;

void pol_sleep_ms( unsigned int millis )
{
  std::this_thread::sleep_for( std::chrono::milliseconds( millis ) );
}

void start_polclock()
{
  Clib::SpinLockGuard guard( polclock_lock );
  polclock_base = PolClock::now();
}

void pause_polclock()
{
  Clib::SpinLockGuard guard( polclock_lock );
  polclock_paused_at = PolClock::now();
}

void restart_polclock()
{
  Clib::SpinLockGuard guard( polclock_lock );
  polclock_base += PolClock::now() - polclock_paused_at;
  polclock_paused_at = PolClock::time_point( PolClock::duration( 0 ) );
}

polclock_t polclock()
{
  Clib::SpinLockGuard guard( polclock_lock );
  return std::chrono::duration_cast<polclock_t_unit>( PolClock::now() - polclock_base +
                                                      unittest_shift )
             .count() /
         10;
}

void shift_clock_for_unittest( std::chrono::milliseconds milli )
{
  Clib::SpinLockGuard guard( polclock_lock );
  unittest_shift += milli;
}

void start_poltime()
{
  Clib::SpinLockGuard guard( polclock_lock );
  poltime_base = PolClock::now();
}
void pause_poltime()
{
  Clib::SpinLockGuard guard( polclock_lock );
  poltime_paused_at = PolClock::now();
}
void restart_poltime()
{
  Clib::SpinLockGuard guard( polclock_lock );
  poltime_base += PolClock::now() - poltime_paused_at;
}

poltime_t poltime()
{
  Clib::SpinLockGuard guard( polclock_lock );
  return std::chrono::duration_cast<poltime_t_unit>( PolClock::now() - poltime_base +
                                                     unittest_shift )
      .count();
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

bool is_polclock_paused_at_zero()
{
  Clib::SpinLockGuard guard( polclock_lock );
  return polclock_paused_at == PolClock::time_point( PolClock::duration( 0 ) );
}
}  // namespace Core
}  // namespace Pol
