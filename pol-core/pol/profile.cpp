/*
History
=======
2006/09/23 Shinigami: Script_Cycles, Sleep_Cycles and Script_passes uses 64bit now

Notes
=======

*/

#include "../clib/stl_inc.h"
#include "profile.h"

size_t rotations;
size_t last_rotations;
size_t last_rpm;

//unsigned int instructions; escript_instr_cycles
u64 last_instructions;
size_t last_sipm;

u64 sleep_cycles;
u64 last_sleep_cycles;
size_t last_scpm;

u64 script_passes;
u64 last_script_passes;
size_t last_sppm;

//unsigned int scheduler_passes;
//unsigned int last_scheduler_passes;
//unsigned int last_schm;

size_t script_passes_activity;
size_t script_passes_noactivity;
size_t last_script_passes_activity;
size_t last_script_passes_noactivity;

size_t mapcache_hits;
size_t mapcache_misses;
size_t last_mapcache_hits;
size_t last_mapcache_misses;

size_t busy_sysload_cycles, last_busy_sysload_cycles;
size_t nonbusy_sysload_cycles, last_nonbusy_sysload_cycles;
size_t sysload_nprocs, last_sysload_nprocs;
size_t last_sysload;

size_t last_cputime;
u64 last_cpu_total;

ProfileVars profilevars;
