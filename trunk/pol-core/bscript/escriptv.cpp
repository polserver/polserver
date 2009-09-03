/*
History
=======
2006/09/23 Shinigami: Script_Cycles, Sleep_Cycles and Script_passes uses 64bit now

Notes
=======

*/

#include "clib/stl_inc.h"

#include "escriptv.h"
bool ecompile_include_debug_sections;
int include_debug;

long executor_count;
long eobject_imp_count;
long eobject_imp_constructions;
long escript_program_count;
u64 escript_instr_cycles;
long escript_execinstr_calls;
