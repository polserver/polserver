/** @file
 *
 * @par History
 * - 2005/09/16 Shinigami: scripts_thread_script* moved to passert
 */

#ifndef __POLSIG_H
#define __POLSIG_H

namespace Pol::Core
{
void install_signal_handlers();
void signal_catch_thread();

struct PolSig
{
  PolSig();
  volatile bool reload_configuration_signalled;
  volatile bool report_status_signalled;

  // 1-100: scripts_thread
  // 100-109: step_scripts
  // 110-130: run_ready
  // 131-140: check_blocked
  unsigned scripts_thread_checkpoint;
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
  unsigned tasks_thread_checkpoint;

  // 100-199: transmit_encrypted
  // 200-299: Client::xmit
  // 300-399: Client::queue_data
  unsigned active_client_thread_checkpoint;  // only set this in guarded code

  unsigned check_attack_after_move_function_checkpoint;
};
}  // namespace Pol::Core
#define THREAD_CHECKPOINT( thread, check ) \
  Core::stateManager.polsig.thread##_thread_checkpoint = check
#define FUNCTION_CHECKPOINT( func, check ) \
  Core::stateManager.polsig.func##_function_checkpoint = check

#endif
