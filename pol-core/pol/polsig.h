/** @file
 *
 * @par History
 * - 2005/09/16 Shinigami: scripts_thread_script* moved to passert
 */

#ifndef __POLSIG_H
#define __POLSIG_H

#include <atomic>

namespace Pol::Core
{
void install_signal_handlers();
void signal_catch_thread();

/**
 * Progress markers written by the long-running threads and read by the stuck-thread watchdog
 * (pol.cpp) to say where each one got to. Every member is written by one thread and read by
 * another, so they are atomic -- but they are advisory values that order nothing, so the
 * macros below store relaxed. The checkpoints sit on hot paths (the scripts one is written
 * per eScript instruction), and a relaxed store is a plain move on every platform we build,
 * where the default seq_cst store would be a locked instruction.
 *
 * The two signalled flags are written from real signal handlers, which is why they are
 * atomic rather than merely volatile.
 */
struct PolSig
{
  PolSig();
  std::atomic<bool> reload_configuration_signalled;
  std::atomic<bool> report_status_signalled;

  // 1-100: scripts_thread
  // 100-109: step_scripts
  // 110-130: run_ready
  // 131-140: check_blocked
  std::atomic<unsigned> scripts_thread_checkpoint;
  // 0-99: tasks_thread
  // 100-199: check_scheduled_tasks
  // 200-299: update_sysload
  // 300-399: update_rpm
  // 400-499: regen_stats
  // 500-599: reload_account_data
  // 600-699: reload_pol_cfg_task
  // 700-799: regen_resources
  // 800-899: swing_task_func
  // 900-999: SpellTask::on_run
  // 1000-1099: RepSystem::repsys_task
  std::atomic<unsigned> tasks_thread_checkpoint;

  // 100-199: transmit_encrypted
  // 200-299: Client::xmit
  // 300-399: Client::queue_data
  std::atomic<unsigned> active_client_thread_checkpoint;  // only set this in guarded code

  std::atomic<unsigned> check_attack_after_move_function_checkpoint;
};
}  // namespace Pol::Core
#define THREAD_CHECKPOINT( thread, check ) \
  Core::stateManager.polsig.thread##_thread_checkpoint.store( check, std::memory_order_relaxed )
#define FUNCTION_CHECKPOINT( func, check ) \
  Core::stateManager.polsig.func##_function_checkpoint.store( check, std::memory_order_relaxed )

#endif
