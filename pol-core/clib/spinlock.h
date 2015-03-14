#ifndef CLIB_SPINLOCK_H
#define CLIB_SPINLOCK_H

#include <atomic>

namespace Pol {
  namespace Clib {
    // spinLock implementation
    // use only with std::lock_guard !
    class SpinLock
    {
    public:
      SpinLock();
      ~SpinLock();
      void lock();
      void unlock();
    private:
      std::atomic_flag _lck;
    };
    
    inline SpinLock::SpinLock()
    {
      _lck.clear();
    }
    inline SpinLock::~SpinLock()
    {
    }
    inline void SpinLock::lock()
    {
      while(_lck.test_and_set(std::memory_order_acquire))
        {}
    }
    inline void SpinLock::unlock()
    {
      _lck.clear(std::memory_order_release);
    }

  } // namespace Clib
} // namespace Pol
#endif
