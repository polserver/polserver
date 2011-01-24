/*
History
=======
2006/09/23 Shinigami: Script_Cycles, Sleep_Cycles and Script_passes uses 64bit now

Notes
=======

*/

#include "../clib/stl_inc.h"
#include "profile.h"

unsigned int rotations;
unsigned int last_rotations;
unsigned int last_rpm;

//unsigned int instructions; escript_instr_cycles
u64 last_instructions;
unsigned int last_sipm;

u64 sleep_cycles;
u64 last_sleep_cycles;
unsigned int last_scpm;

u64 script_passes;
u64 last_script_passes;
unsigned int last_sppm;

//unsigned int scheduler_passes;
//unsigned int last_scheduler_passes;
//unsigned int last_schm;

unsigned int script_passes_activity;
unsigned int script_passes_noactivity;
unsigned int last_script_passes_activity;
unsigned int last_script_passes_noactivity;

unsigned int mapcache_hits;
unsigned int mapcache_misses;
unsigned int last_mapcache_hits;
unsigned int last_mapcache_misses;

unsigned int busy_sysload_cycles, last_busy_sysload_cycles;
unsigned int nonbusy_sysload_cycles, last_nonbusy_sysload_cycles;
unsigned int sysload_nprocs, last_sysload_nprocs;
unsigned int last_sysload;

unsigned int last_cputime;
u64 last_cpu_total;

ProfileVars profilevars;
