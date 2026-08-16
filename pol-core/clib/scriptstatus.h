/** @file
 *
 * @par History
 * - 2026/08/16 Nando: extracted from the scripts_thread_script* globals added 2005/09/16
 */

#ifndef CLIB_SCRIPTSTATUS_H
#define CLIB_SCRIPTSTATUS_H

#include <atomic>
#include <mutex>
#include <string>

namespace Pol::Clib
{
/**
 * The script and program counter last seen by a script execution path, published for the
 * stuck-thread watchdog and for the crash reports.
 *
 * Purely advisory. It is written by whichever thread is running a script and read by another
 * thread (the watchdog dump) or from a crash path, so every access here is defined but none
 * is ordered against anything else -- a snapshot may mix a name and a PC that were never
 * current at the same instant.
 *
 * The readers must never block: a shard that is stuck is exactly when the watchdog runs, and
 * a crash report must not deadlock against the thread that crashed. So the name is taken with
 * try_lock and the reader settles for no name at all rather than waiting. The write side is a
 * fixed-buffer copy taken once per script switch, which is cheaper than the std::string
 * assignment it replaces.
 *
 * There is only one instance, so a script running off the scripts thread -- a client i/o
 * thread running a logoff test or a syshook -- overwrites the scripts thread's value and a
 * report can name the wrong script. That is pre-existing: this class exists to make the
 * access race-free, not to make it accurate.
 */
class ScriptStatus
{
public:
  void set_script( const std::string& name );
  void set_pc( unsigned pc ) { pc_.store( pc, std::memory_order_relaxed ); }

  /**
   * Copies the current name and PC. Never blocks.
   * @param name receives the script name, or is left empty if another thread is mid-write.
   * @param pc always receives the current PC.
   * @returns false if the name was unavailable.
   */
  bool snapshot( std::string& name, unsigned& pc ) const;

  /// The name alone, or "(unavailable)" -- for log lines that just want to interpolate it.
  std::string script_name() const;

private:
  mutable std::mutex name_mutex_;
  char name_[128] = {};
  std::atomic<unsigned> pc_{ 0 };
};

extern ScriptStatus script_status;
}  // namespace Pol::Clib

#endif
