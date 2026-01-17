/** @file
 *
 * @par History
 * - 2006/09/23 Shinigami: Script_Cycles, Sleep_Cycles and Script_passes uses 64bit now
 */


#include "escriptv.h"

#include "../clib/rawtypes.h"


namespace Pol::Bscript
{
int include_debug;

int executor_count = 0;
std::atomic<int> eobject_imp_count( 0 );
std::atomic<int> eobject_imp_constructions( 0 );
int escript_program_count = 0;
u64 escript_instr_cycles = 0;
int escript_execinstr_calls = 0;
}  // namespace Pol::Bscript
