/*
 * With permission to reuse:
 * Author:  Thomas Volkert
 * Libary:  Homerbase www.silvo.de
 * Since:   2010-09-20
 */

#ifndef _BASE_MUTEX_
#define _BASE_MUTEX_

#ifdef _WIN32
	#include <windows.h>
	#include <stdio.h>
	#define OS_DEP_MUTEX HANDLE
#else
	#include <pthread.h>
	#include <stdlib.h>
	#include <errno.h>
	#define OS_DEP_MUTEX pthread_mutex_t*
#endif

///////////////////////////////////////////////////////////////////////////////

class Mutex
{
public:
    Mutex( );

    virtual ~Mutex( );

    /* every function returns TRUE if successful */
    bool lock();
    bool unlock();
    bool tryLock();
    bool tryLock(int pMSecs);

private:
    OS_DEP_MUTEX mPThreadMutex;
};

///////////////////////////////////////////////////////////////////////////////

// Small mod: Guard Class for exception safe lock
class LocalMutex
{
public:
	inline LocalMutex(Mutex* mutex);
	inline ~LocalMutex();
	inline void unlock();
private:
	Mutex* _mutex;
	LocalMutex( LocalMutex& ); 
	LocalMutex& operator=( LocalMutex& );
	LocalMutex(const Mutex*);
};
inline LocalMutex::LocalMutex(Mutex* mutex):_mutex(mutex)
{
	_mutex->lock();
}
inline LocalMutex::~LocalMutex()
{
	_mutex->unlock();
}
inline void LocalMutex::unlock()
{
	if (_mutex != 0)
	{
		_mutex->unlock();
		_mutex = 0;
	}
}

#endif
