/*
History
=======
2006/09/23 Shinigami: Script_Cycles, Sleep_Cycles and Script_passes uses 64bit now

Notes
=======

*/

#ifndef BSCRIPT_ESCRIPTV_H
#define BSCRIPT_ESCRIPTV_H

#include "../clib/rawtypes.h"
namespace Pol {
  namespace Bscript {
	extern int include_debug;

	extern int executor_count;

	extern int eobject_imp_count;
	extern int eobject_imp_constructions;

	extern int escript_program_count;

	extern u64 escript_instr_cycles;
	extern int escript_execinstr_calls;

	extern bool ecompile_include_debug_sections;
  }
}
#endif
