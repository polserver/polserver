/*
History
=======
2006/09/23 Shinigami: Script_Cycles, Sleep_Cycles and Script_passes uses 64bit now

Notes
=======

*/

#include "../clib/stl_inc.h"
#include "profile.h"

unsigned long rotations;
unsigned long last_rotations;
unsigned long last_rpm;

//unsigned long instructions; escript_instr_cycles
u64 last_instructions;
unsigned long last_sipm;

u64 sleep_cycles;
u64 last_sleep_cycles;
unsigned long last_scpm;

u64 script_passes;
u64 last_script_passes;
unsigned long last_sppm;

//unsigned long scheduler_passes;
//unsigned long last_scheduler_passes;
//unsigned long last_schm;

unsigned long script_passes_activity;
unsigned long script_passes_noactivity;
unsigned long last_script_passes_activity;
unsigned long last_script_passes_noactivity;

unsigned long mapcache_hits;
unsigned long mapcache_misses;
unsigned long last_mapcache_hits;
unsigned long last_mapcache_misses;

unsigned long busy_sysload_cycles, last_busy_sysload_cycles;
unsigned long nonbusy_sysload_cycles, last_nonbusy_sysload_cycles;
unsigned long sysload_nprocs, last_sysload_nprocs;
unsigned long last_sysload;

unsigned long last_cputime;
u64 last_cpu_total;

ProfileVars profilevars;
