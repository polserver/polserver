/** @file
 *
 * @par History
 * - 2006/09/23 Shinigami: Script_Cycles, Sleep_Cycles and Script_passes uses 64bit now
 */


#ifndef BSCRIPT_ESCRIPTV_H
#define BSCRIPT_ESCRIPTV_H

#include "clib/rawtypes.h"
#include <atomic>


namespace Pol::Bscript
{
extern int include_debug;

extern int executor_count;

extern std::atomic<int> eobject_imp_count;
extern std::atomic<int> eobject_imp_constructions;

extern int escript_program_count;

/**
 * Instructions executed since startup. Incremented by whichever thread is running a script
 * and read by the tasks thread (sysload) and the stuck-thread watchdog, so it is atomic.
 *
 * Deliberately not a fetch_add: the increment sits in execInstr, the hottest loop in the
 * server, and a read-modify-write there would cost a locked instruction per eScript
 * instruction. A relaxed load-add-store is a plain move on every platform we build. Script
 * execution is serialized by the world lock, so nothing is lost in practice, and this is an
 * advisory counter besides.
 */
extern std::atomic<u64> escript_instr_cycles;
inline void count_instr_cycle()
{
  escript_instr_cycles.store( escript_instr_cycles.load( std::memory_order_relaxed ) + 1,
                              std::memory_order_relaxed );
}
extern int escript_execinstr_calls;
}  // namespace Pol::Bscript

#endif
