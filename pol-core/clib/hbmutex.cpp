/*
 * With permission to reuse:
 * Author:  Thomas Volkert
 * Libary:  Homerbase http://www.homer-conferencing.com/
 * Since:   2010-09-20
 */

#include "hbmutex.h"
#include "logfile.h"

///////////////////////////////////////////////////////////////////////////////

Mutex::Mutex()
{
	bool tResult = false;
#ifdef _WIN32
	mPThreadMutex = CreateMutex(NULL, false, NULL);
	tResult = (mPThreadMutex != NULL);
#else
	mPThreadMutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	tResult = (pthread_mutex_init(mPThreadMutex, NULL) == 0);
#endif
	if (!tResult)
        Log("Initiation of mutex failed");
}

Mutex::~Mutex()
{
    bool tResult = false;
#ifdef _WIN32
	tResult = (CloseHandle(mPThreadMutex) != 0);
#else
	tResult = (pthread_mutex_destroy(mPThreadMutex) == 0);
	free(mPThreadMutex);
#endif
    if (!tResult)
		Log("Destruction of mutex failed");
}

///////////////////////////////////////////////////////////////////////////////

bool Mutex::lock()
{
#ifdef _WIN32
	return (WaitForSingleObject(mPThreadMutex, INFINITE) != WAIT_FAILED);
#else
	return !pthread_mutex_lock(mPThreadMutex);
#endif
}

bool Mutex::unlock()
{
#ifdef _WIN32
	return (ReleaseMutex(mPThreadMutex) != 0);
#else
	return !pthread_mutex_unlock(mPThreadMutex);
#endif
}

bool Mutex::tryLock()
{
	bool tResult = false;
#ifdef _WIN32
	switch(WaitForSingleObject(mPThreadMutex, (DWORD)0))
	{
	case WAIT_ABANDONED:
	case WAIT_TIMEOUT:
	case WAIT_FAILED:
		break;
	case WAIT_OBJECT_0:
		tResult = true;
		break;
	}
#else
	switch(pthread_mutex_trylock(mPThreadMutex))
	{
		case EDEADLK:
			Log("Lock already held by calling thread");
			break;
		case EBUSY: // lock can't be obtained because it is busy
			break;
		case EINVAL:
			Log("Lock was found in uninitialized state");
			break;
		case EFAULT:
			Log("Invalid lock pointer was given");
			break;
		case 0: // lock was free and is obtained now
			tResult = true;
			break;
		default:
			Log("Error occurred while trying to get lock: code %d", tResult);
			break;
	}
#endif
	return tResult;
}


bool Mutex::tryLock(int pMSecs)
{
	bool tResult = false;
#ifdef _WIN32
	switch(WaitForSingleObject(mPThreadMutex, (DWORD)pMSecs))
	{
	case WAIT_ABANDONED:
	case WAIT_TIMEOUT:
	case WAIT_FAILED:
		break;
	case WAIT_OBJECT_0:
		tResult = true;
		break;
	}
#else
	struct timespec tTimeout;

	if (clock_gettime(CLOCK_REALTIME, &tTimeout) == -1)
		Log("Failed to get time from clock");

	// add msecs to current time stamp
	tTimeout.tv_nsec += pMSecs * 1000 * 1000;

	switch(pthread_mutex_timedlock(mPThreadMutex, &tTimeout))
	{
		case EDEADLK:
			Log("Lock already held by calling thread");
			break;
		case EBUSY: // lock can't be obtained because it is busy
			break;
		case EINVAL:
			Log("Lock was found in uninitialized state");
			break;
		case EFAULT:
			Log("Invalid lock pointer was given");
			break;
		case ETIMEDOUT:
			Log("Lock couldn't be obtained in given time");
			break;
		case 0: // lock was free and is obtained now
			tResult = true;
			break;
		default:
			Log("Error occurred while trying to get lock: code %d", tResult);
			break;
	}
#endif
	return tResult;
}

///////////////////////////////////////////////////////////////////////////////

