/*
History
=======


Notes
=======

*/

#ifndef POLSEM_H
#define POLSEM_H

#ifdef _WIN32
#include <windows.h>
#include <iostream>
#else
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <iostream>
#include "clib/passert.h"
#endif

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

	//
	// moved to threadhelp:
	//

	//void start_thread( void (*threadf)(void), const char* thread_name );
	//void start_thread( void (*threadf)(void*), const char* thread_name, void* arg );

	//void run_thread( void (*threadf)(void) );
	//void run_thread( void (*threadf)(void *), void* arg );

	//void inc_child_thread_count( bool need_lock = true );
	//void dec_child_thread_count( bool need_lock = true );

	//extern unsigned int child_threads;

#ifdef _WIN32
	extern CRITICAL_SECTION cs;
	extern DWORD locker;
	void polsem_lock();
	void polsem_unlock();
#else 
	extern pid_t locker;
	extern pthread_mutex_t polsem;
	void polsem_lock();
	void polsem_unlock();
#endif // not _WIN32

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
  }
}
#endif // POLSEM_H
