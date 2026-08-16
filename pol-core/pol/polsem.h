/** @file
 *
 * @par History
 */


#ifndef POLSEM_H
#define POLSEM_H

// define to debug PolLocks, log each lock entry
// #define POLLOCK_TRACE

// TODO: encapsulate the "locker" variable to remove those includes from here. Would a size_t work?
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <pthread.h>
#include <unistd.h>
#endif
#ifdef POLLOCK_TRACE
#include "clib/logfacility.h"
#endif
#include <atomic>


namespace Pol::Core
{
void init_ipc_vars();
void deinit_ipc_vars();

void send_pulse();
void wait_for_pulse( unsigned int millis );

void wake_tasks_thread();
void tasks_thread_sleep( unsigned int millis );

/// tid of the thread holding the world lock, 0 if free. Read unlocked by the stuck-thread
/// watchdog and the crash reports, hence atomic; relaxed, it orders nothing.
extern std::atomic<size_t> locker;
#ifdef _WIN32
extern CRITICAL_SECTION cs;
#else
extern pthread_mutex_t polsem;
#endif  // not _WIN32

void polsem_lock();
void polsem_unlock();
/// Takes the world lock if it is free. For diagnostics that must not block on a stuck shard.
bool polsem_trylock();

#ifdef POLLOCK_TRACE
class PolLockD
{
public:
  PolLockD() { polsem_lock(); }
  ~PolLockD() { polsem_unlock(); }
};
inline void noop() {};
#define PolLock                                     \
  noop();                                           \
  INFO_PRINTLN( "lock {} {}", __FILE__, __LINE__ ); \
  Core::PolLockD
#else
class PolLock
{
public:
  PolLock() { polsem_lock(); }
  ~PolLock() { polsem_unlock(); }
};
#endif

/// Takes the world lock only if it is free, so a diagnostic can read world state without
/// hanging on the very deadlock it is reporting. Check locked() before trusting what you read.
///
/// A plain attempt fails most of the time on a *healthy* shard, because the scripts thread
/// holds the lock for a whole pass and releases it only between passes -- so pass a budget
/// in milliseconds to retry within, which distinguishes "busy" from "wedged" instead of
/// treating them alike.
class PolLockTry
{
public:
  explicit PolLockTry( unsigned max_wait_ms = 0 );
  ~PolLockTry()
  {
    if ( locked_ )
      polsem_unlock();
  }
  PolLockTry( const PolLockTry& ) = delete;
  PolLockTry& operator=( const PolLockTry& ) = delete;

  bool locked() const { return locked_; }

private:
  bool locked_;
};

class PolLock2
{
public:
  PolLock2() : locked_( true ) { polsem_lock(); }
  ~PolLock2()
  {
    if ( locked_ )
      polsem_unlock();
    locked_ = false;
  }

  void unlock()
  {
    polsem_unlock();
    locked_ = false;
  }
  void lock()
  {
    polsem_lock();
    locked_ = true;
  }

private:
  bool locked_;
};
}  // namespace Pol::Core

#endif  // POLSEM_H
