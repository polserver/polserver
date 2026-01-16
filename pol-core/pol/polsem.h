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

extern size_t locker;
#ifdef _WIN32
extern CRITICAL_SECTION cs;
#else
extern pthread_mutex_t polsem;
#endif  // not _WIN32

void polsem_lock();
void polsem_unlock();

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
