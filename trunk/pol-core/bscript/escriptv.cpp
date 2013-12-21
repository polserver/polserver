/*
History
=======
2006/09/23 Shinigami: Script_Cycles, Sleep_Cycles and Script_passes uses 64bit now

Notes
=======

*/

#include "../clib/stl_inc.h"

#include "escriptv.h"
namespace Pol {
  namespace Bscript {
	bool ecompile_include_debug_sections;
	int include_debug;

	int executor_count;
	int eobject_imp_count;
	int eobject_imp_constructions;
	int escript_program_count;
	u64 escript_instr_cycles;
	int escript_execinstr_calls;
  }
}