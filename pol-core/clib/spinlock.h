#pragma once

#include <atomic>
#include <mutex>

namespace Pol::Clib
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
  ~SpinLock() = default;

private:
  void lock() noexcept;
  void unlock() noexcept;

  std::atomic_flag _lck;
};

/** This is a std::lock_guard specific for SpinLock */
typedef std::lock_guard<SpinLock> SpinLockGuard;

inline SpinLock::SpinLock()
{
  _lck.clear();
}

/**
 * Puts the caller in an endless busy loop until it acquires the lock
 */
inline void SpinLock::lock() noexcept
{
  while ( _lck.test_and_set( std::memory_order_acquire ) )
    _lck.wait( true, std::memory_order_relaxed );
}

/**
 * Releases the lock
 */
inline void SpinLock::unlock() noexcept
{
  _lck.clear( std::memory_order_release );
  _lck.notify_one();
}

}  // namespace Pol::Clib
