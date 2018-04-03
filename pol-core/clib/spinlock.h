#ifndef CLIB_SPINLOCK_H
#define CLIB_SPINLOCK_H

#include <atomic>
#include <mutex>

namespace Pol
{
namespace Clib
{
/**
 * This is a much faster replacement for mutex when locking very small
 * and fast running protected parts. It is based on a busy loop.
 *
 * @warning use it only through SpinLockGuard
 */
class SpinLock
{
  friend class std::lock_guard<SpinLock>;

public:
  SpinLock();
  ~SpinLock();

private:
  void lock();
  void unlock();

  std::atomic_flag _lck;
};

/** This is a std::lock_guard specific for SpinLock */
typedef std::lock_guard<SpinLock> SpinLockGuard;

inline SpinLock::SpinLock()
{
  _lck.clear();
}
inline SpinLock::~SpinLock()
{
}

/**
 * Puts the caller in an endless busy loop until it acquires the lock
 */
inline void SpinLock::lock()
{
  while ( _lck.test_and_set( std::memory_order_acquire ) )
  {
  }
}

/**
 * Releases the lock
 */
inline void SpinLock::unlock()
{
  _lck.clear( std::memory_order_release );
}

}  // namespace Clib
}  // namespace Pol
#endif
