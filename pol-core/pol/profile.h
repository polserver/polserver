/*
History
=======
2006/09/23 Shinigami: Script_Cycles, Sleep_Cycles and Script_passes uses 64bit now

Notes
=======

*/

#ifndef PROFILE_H
#define PROFILE_H

#include "../clib/rawtypes.h"
#include <cstddef>
#include <ctime>

namespace Pol {
  namespace Core {
#define DEF_PROFILEVAR( counter ) \
  unsigned int prf_##counter, \
  prf_last_##counter, \
  prf_last_##counter##_per_min

#define CLOCK_PROFILEVAR( timer ) \
  clock_t tmr_##timer##_clocks_this_min, \
  tmr_##timer##_clocks_last_min, \
  tmr_##timer##_clock_start

	struct ProfileVars
	{
	  DEF_PROFILEVAR( events );
	  DEF_PROFILEVAR( skill_checks );
	  DEF_PROFILEVAR( combat_operations );
	  DEF_PROFILEVAR( los_checks );
	  DEF_PROFILEVAR( polmap_walkheight_calculations );
	  DEF_PROFILEVAR( uomap_walkheight_calculations );
	  DEF_PROFILEVAR( mobile_movements );
	  DEF_PROFILEVAR( error_creations );
	  DEF_PROFILEVAR( tasks_ontime );
	  DEF_PROFILEVAR( tasks_late );
	  DEF_PROFILEVAR( tasks_late_ticks );
	  DEF_PROFILEVAR( scripts_ontime );
	  DEF_PROFILEVAR( scripts_late );
	  DEF_PROFILEVAR( scripts_late_ticks );
	  DEF_PROFILEVAR( scheduler_passes );
	  DEF_PROFILEVAR( noactivity_scheduler_passes );
	  DEF_PROFILEVAR( npc_searches );
	  DEF_PROFILEVAR( container_adds );
	  DEF_PROFILEVAR( container_removes );

	  CLOCK_PROFILEVAR( npc_search );
	};
	extern ProfileVars profilevars;

	extern size_t rotations, last_rotations;
	extern size_t last_rpm;

	//extern unsigned int instructions;
	extern u64 last_instructions;
	extern size_t last_sipm;

	extern u64 sleep_cycles;
	extern u64 last_sleep_cycles;
	extern size_t last_scpm;

	extern size_t busy_sysload_cycles, last_busy_sysload_cycles;
	extern size_t nonbusy_sysload_cycles, last_nonbusy_sysload_cycles;
	extern size_t sysload_nprocs, last_sysload_nprocs;
	extern size_t last_sysload;

	extern u64 last_cpu_total;
	extern size_t last_cputime;

	extern u64 script_passes;
	extern u64 last_script_passes;
	extern size_t last_sppm;

	//extern unsigned int scheduler_passes, last_scheduler_passes;
	//extern unsigned int last_schm;

	extern size_t script_passes_activity;
	extern size_t script_passes_noactivity;
	extern size_t last_script_passes_activity;
	extern size_t last_script_passes_noactivity;

	extern size_t mapcache_hits, last_mapcache_hits;
	extern size_t mapcache_misses, last_mapcache_misses;
  }
#define INC_PROFILEVAR( counter ) ++Core::profilevars.prf_##counter
#define SET_PROFILEVAR( counter, newvalue ) Core::profilevars.prf_##counter = newvalue
#define INC_PROFILEVAR_BY( counter, amount ) Core::profilevars.prf_##counter += amount

#define TICK_PROFILEVAR( counter )                      \
  do \
  {  \
  Core::profilevars.prf_last_##counter##_per_min = Core::profilevars.prf_##counter - Core::profilevars.prf_last_##counter;  \
  Core::profilevars.prf_last_##counter = Core::profilevars.prf_##counter;                      \
  } while ( 0 )

#define GET_PROFILEVAR( counter ) Core::profilevars.prf_##counter
#define GET_PROFILEVAR_PER_MIN( counter ) Core::profilevars.prf_last_##counter##_per_min

#define START_PROFILECLOCK( timer ) Core::profilevars.tmr_##timer##_clock_start = clock()
#define STOP_PROFILECLOCK( timer )  Core::profilevars.tmr_##timer##_clocks_this_min += clock() - Core::profilevars.tmr_##timer##_clock_start
#define ROLL_PROFILECLOCK( timer )  \
  do \
  { \
  Core::profilevars.tmr_##timer##_clocks_last_min = Core::profilevars.tmr_##timer##_clocks_this_min; \
  Core::profilevars.tmr_##timer##_clocks_this_min = 0; \
  } while ( 0 )
#define GET_PROFILECLOCK( timer ) Core::profilevars.tmr_##timer##_clocks_last_min
#define GET_PROFILECLOCK_MS( timer ) ( static_cast<unsigned int>( Core::profilevars.tmr_##timer##_clocks_last_min * 1000.0 / Core::CLOCKS_PER_SEC ) )

}
#endif
