#ifndef CLIB_SPINLOCK_H
#define CLIB_SPINLOCK_H

#include <atomic>
#include <mutex>

namespace Pol {
  namespace Clib {

    /**
     * This is a much fast replacement for a mutex
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
     * Works just like std::mutex::lock
     */
    inline void SpinLock::lock()
    {
      while(_lck.test_and_set(std::memory_order_acquire))
        {}
    }

    /**
     * Works just like std::mutex::unlock
     */
    inline void SpinLock::unlock()
    {
      _lck.clear(std::memory_order_release);
    }

  } // namespace Clib
} // namespace Pol
#endif
