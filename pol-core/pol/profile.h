/** @file
 *
 * @par History
 * - 2006/09/23 Shinigami: Script_Cycles, Sleep_Cycles and Script_passes uses 64bit now
 */


#ifndef PROFILE_H
#define PROFILE_H

#include "../clib/clib.h"
#include "../clib/rawtypes.h"
#include <atomic>
#include <cstddef>
#include <ctime>

namespace Pol
{
namespace Core
{
#define DEF_PROFILEVAR( counter ) \
  size_t prf_##counter, prf_last_##counter, prf_last_##counter##_per_min

#define CLOCK_PROFILEVAR( timer ) \
  clock_t tmr_##timer##_clocks_this_min, tmr_##timer##_clocks_last_min, tmr_##timer##_clock_start

struct ProfileVars
{
  DEF_PROFILEVAR( events );
  DEF_PROFILEVAR( skill_checks );
  DEF_PROFILEVAR( combat_operations );
  DEF_PROFILEVAR( error_creations );
  DEF_PROFILEVAR( tasks_ontime );
  DEF_PROFILEVAR( tasks_late );
  DEF_PROFILEVAR( tasks_late_ticks );
  DEF_PROFILEVAR( scripts_ontime );
  DEF_PROFILEVAR( scripts_late );
  DEF_PROFILEVAR( scripts_late_ticks );
  DEF_PROFILEVAR( task_passes );
  DEF_PROFILEVAR( noactivity_task_passes );
  DEF_PROFILEVAR( container_adds );
  DEF_PROFILEVAR( container_removes );

  u64 last_instructions;
  size_t last_instructions_pm;

  DEF_PROFILEVAR( sleep_cycles );

  size_t busy_sysload_cycles, last_busy_sysload_cycles;
  size_t nonbusy_sysload_cycles, last_nonbusy_sysload_cycles;
  size_t sysload_nprocs, last_sysload_nprocs;
  size_t last_sysload;

  u64 last_cpu_total;
  size_t last_cputime;

  DEF_PROFILEVAR( script_passes );

  std::atomic<size_t> script_passes_activity;
  std::atomic<size_t> script_passes_noactivity;
  size_t last_script_passes_activity;
  size_t last_script_passes_noactivity;
  Clib::OnlineStatistics script_passes_duration{};
  Clib::OnlineStatistics script_passes_delay{};
  Clib::OnlineStatistics script_runlist_statistic{};
};
}  // namespace Core
#define INC_PROFILEVAR( counter ) ++Core::stateManager.profilevars.prf_##counter
#define SET_PROFILEVAR( counter, newvalue ) Core::stateManager.profilevars.prf_##counter = newvalue
#define INC_PROFILEVAR_BY( counter, amount ) Core::stateManager.profilevars.prf_##counter += amount

#define TICK_PROFILEVAR( counter )                                \
  do                                                              \
  {                                                               \
    Core::stateManager.profilevars.prf_last_##counter##_per_min = \
        Core::stateManager.profilevars.prf_##counter -            \
        Core::stateManager.profilevars.prf_last_##counter;        \
    Core::stateManager.profilevars.prf_last_##counter =           \
        Core::stateManager.profilevars.prf_##counter;             \
  } while ( 0 )

#define GET_PROFILEVAR( counter ) Core::stateManager.profilevars.prf_##counter
#define GET_PROFILEVAR_PER_MIN( counter ) \
  Core::stateManager.profilevars.prf_last_##counter##_per_min

#define START_PROFILECLOCK( timer ) \
  Core::stateManager.profilevars.tmr_##timer##_clock_start = clock()
#define STOP_PROFILECLOCK( timer )                                \
  Core::stateManager.profilevars.tmr_##timer##_clocks_this_min += \
      clock() - Core::stateManager.profilevars.tmr_##timer##_clock_start
#define ROLL_PROFILECLOCK( timer )                                    \
  do                                                                  \
  {                                                                   \
    Core::stateManager.profilevars.tmr_##timer##_clocks_last_min =    \
        Core::stateManager.profilevars.tmr_##timer##_clocks_this_min; \
    Core::stateManager.profilevars.tmr_##timer##_clocks_this_min = 0; \
  } while ( 0 )
#define GET_PROFILECLOCK( timer ) Core::stateManager.profilevars.tmr_##timer##_clocks_last_min
#define GET_PROFILECLOCK_MS( timer )                                                          \
  ( static_cast<unsigned int>( Core::stateManager.profilevars.tmr_##timer##_clocks_last_min * \
                               1000.0 / Core::CLOCKS_PER_SEC ) )
}  // namespace Pol
#endif
