/*
 * With permission to reuse:
 * Author:  Thomas Volkert
 * Libary:  Homerbase www.silvo.de
 * Since:   2010-09-20
 */

#ifndef _BASE_MUTEX_
#define _BASE_MUTEX_

#ifdef __linux__
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#define OS_DEP_MUTEX pthread_mutex_t*
#endif
#ifdef _WIN32
#include <windows.h>
#include <stdio.h>
#define OS_DEP_MUTEX HANDLE
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

#endif
