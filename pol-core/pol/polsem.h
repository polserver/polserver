/*
History
=======


Notes
=======

*/

#ifndef POLSEM_H
#define POLSEM_H

// TODO: encapsulate the "locker" variable to remove those includes from here. Would a size_t work?
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <pthread.h>
#include <unistd.h>
#include "clib/passert.h"
#endif
#include <atomic>

namespace Pol {
  namespace Core {
	void init_ipc_vars();
	void deinit_ipc_vars();

	void send_pulse();
	void wait_for_pulse( unsigned int millis );

	void wake_tasks_thread();
	void tasks_thread_sleep( unsigned int millis );

	void send_ClientTransmit_pulse();
	void wait_for_ClientTransmit_pulse( unsigned int millis );
    
#ifdef _WIN32
	extern CRITICAL_SECTION cs;
	extern DWORD locker;
#else 
	extern pid_t locker;
    extern pthread_mutex_t polsem;
#endif // not _WIN32

	void polsem_lock();
	void polsem_unlock();

	class PolLock
	{
	public:
	  PolLock() { polsem_lock(); }
	  ~PolLock() { polsem_unlock(); }
	};

	class PolLock2
	{
	public:
	  PolLock2() : locked_( true ) { polsem_lock(); }
	  ~PolLock2() { if ( locked_ ) polsem_unlock(); locked_ = false; }

	  void unlock() { polsem_unlock(); locked_ = false; }
	  void lock() { polsem_lock(); locked_ = true; }
	private:
	  bool locked_;
	};

    
    
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
  }
}
#endif // POLSEM_H
