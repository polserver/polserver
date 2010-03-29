/*
History
=======
2005/12/13 Shinigami: added error code printing in create_thread for debugging
2006/02/06 Shinigami: smaller bugfix in logging
                      error code printing in create_thread extended
2007/02/28 Shinigami: error code printing in create_thread added for linux
2007/03/08 Shinigami: added pthread_exit and _endhreadex to close threads
2008/03/02 Nando: Added bool dec_child to create_thread, used to dec_child_thread_count()
                  if there is an error on create_thread. Will fix some of the zombies.

Notes
=======

*/

#include "stl_inc.h"

#include "passert.h"
#include "threadhelp.h"
#include "logfile.h"

#include <cstring>

namespace threadhelp
{

ThreadMap threadmap;
unsigned int child_threads;
static int threads = 0;

#ifdef _WIN32
#include <windows.h>
#include <process.h>
static CRITICAL_SECTION threadhelp_cs;
static CRITICAL_SECTION threadmap_cs;
void init_threadhelp()
{
    InitializeCriticalSection( &threadhelp_cs );
    InitializeCriticalSection( &threadmap_cs );
}
void threadsem_lock()
{
    EnterCriticalSection( &threadhelp_cs );
}
void threadsem_unlock()
{
    LeaveCriticalSection( &threadhelp_cs );
}

void threadmap_lock()
{
    EnterCriticalSection( &threadmap_cs );
}
void threadmap_unlock()
{
    LeaveCriticalSection( &threadmap_cs );
}

void thread_sleep_ms( unsigned millis )
{
    Sleep( millis );
}
unsigned thread_pid()
{
    return GetCurrentThreadId();
}
#else
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

static pthread_mutexattr_t threadsem_attr;
static pthread_mutex_t threadsem;
static pid_t threadhelp_locker;

static pthread_mutexattr_t threadmap_sem_attr;
static pthread_mutex_t threadmap_sem;

static pthread_attr_t create_detached_attr;


void init_threadhelp()
{
    int res;

    res = pthread_mutexattr_init( &threadsem_attr );
    passert_always( res == 0 );

    res = pthread_mutex_init( &threadsem, &threadsem_attr );
    passert_always( res == 0 );

    res = pthread_mutexattr_init( &threadmap_sem_attr );
    passert_always( res == 0 );

    res = pthread_mutex_init( &threadmap_sem, &threadmap_sem_attr );
    passert_always( res == 0 );

    res = pthread_attr_init( &create_detached_attr );
    passert_always( res == 0 );
    res = pthread_attr_setdetachstate( &create_detached_attr, PTHREAD_CREATE_DETACHED );
    passert_always( res == 0 );
}
void threadsem_lock()
{
    pid_t pid = getpid();
    int res = pthread_mutex_lock( &threadsem );
    if (res != 0)
    {
        Log( "pthread_mutex_lock: res=%d, pid=%d\n", res, pid );
    }
    passert_always( res == 0 );
    passert_always( threadhelp_locker == 0 );
    threadhelp_locker = pid;
}
void threadsem_unlock()
{
    pid_t pid = getpid();
    passert_always( threadhelp_locker == pid );
    threadhelp_locker = 0;
    int res = pthread_mutex_unlock( &threadsem );
    if (res != 0)
    {
        Log( "pthread_mutex_unlock: res=%d,pid=%d", res, pid );
    }
    passert_always( res == 0 );
}

void threadmap_lock()
{
    int res = pthread_mutex_lock( &threadmap_sem );
    if (res != 0)
    {
        Log( "pthread_mutex_lock(threadmap_sem): res=%d, pid=%d\n", res, getpid() );
    }
    passert_always( res == 0 );
}
void threadmap_unlock()
{
    int res = pthread_mutex_unlock( &threadmap_sem );
    if (res != 0)
    {
        Log( "pthread_mutex_unlock(threadmap_sem): res=%d,pid=%d", res, getpid() );
    }
    passert_always( res == 0 );
}

void thread_sleep_ms( unsigned millis )
{
    usleep( millis * 1000L );
}
unsigned thread_pid()
{
    return getpid();
}
#endif

void inc_child_thread_count( bool need_lock )
{
    if (need_lock)
        threadsem_lock();
    
    ++child_threads;
    
    if (need_lock)
        threadsem_unlock();
}
void dec_child_thread_count( bool need_lock )
{
    if (need_lock)
        threadsem_lock();
    
    --child_threads;
    
    if (need_lock)
        threadsem_unlock();
}

void run_thread( void (*threadf)(void) )
{
    // thread creator calls inc_child_thread_count before starting thread
    try {
        (*threadf)();
    }
    catch( std::exception& ex )
    {
		cerr << "Thread exception: " << ex.what() <<endl;
    }

	dec_child_thread_count();

	threadmap.Unregister(thread_pid());
}
void run_thread( void (*threadf)(void *), void* arg )
{
    // thread creator calls inc_child_thread_count before starting thread
    try {
        (*threadf)(arg);
    }
    catch( std::exception& ex)
    {
		cerr << "Thread exception: " << ex.what() <<endl;
    }

	dec_child_thread_count();

	threadmap.Unregister( thread_pid() );
}

class ThreadData
{
public:
    string name;
    void (*entry)(void*);
    void (*entry_noparam)(void);
    void* arg;
};

#ifdef _WIN32
unsigned __stdcall thread_stub2( void *v_td)
#else
void* thread_stub2( void *v_td)
#endif
{
    ThreadData* td = reinterpret_cast<ThreadData*>(v_td);

    void (*entry)(void*) = td->entry;
    void (*entry_noparam)(void) = td->entry_noparam;
    void* arg = td->arg;

    threadmap.Register( thread_pid(), td->name );

    delete td;
    td = NULL;

    if (entry != NULL)
        run_thread( entry, arg );
    else
        run_thread( entry_noparam );

    #ifdef _WIN32
    _endthreadex(0);
    #else
    pthread_exit(NULL);
    #endif
	return 0;
}

#ifdef _WIN32
void create_thread( ThreadData* td, bool dec_child = false )
{
    unsigned threadid;
    HANDLE h = (HANDLE) _beginthreadex( NULL, 0, thread_stub2, td, 0, &threadid );
	if (h == 0) // added for better debugging
	{
		Log( "error in create_thread: %d %d \"%s\" \"%s\" %d %d %s %d %d %d\n",
			 errno, _doserrno, strerror( errno ), strerror( _doserrno ),
			 threads++, thread_stub2, td->name.c_str(), td->entry, td->entry_noparam, td->arg );

		// dec_child says that we should dec_child_threads when there's an error... :)
		if (dec_child)
			dec_child_thread_count();
	}
	else
		CloseHandle( h );
}
#else
void create_thread( ThreadData* td, bool dec_child = false )
{
    pthread_t thread;
	int result = pthread_create( &thread, &create_detached_attr, thread_stub2, td );
	if ( result != 0) // added for better debugging
	{
		Log( "error in create_thread: %d %d \"%s\" %d %d %s %d %d %d\n",
			 result, errno, strerror( errno ), 
			 threads++, thread_stub2, td->name.c_str(), td->entry, td->entry_noparam, td->arg );
		
		// dec_child says that we should dec_child_threads when there's an error... :)
		if (dec_child)
			dec_child_thread_count();
	}
}
#endif

void start_thread( void (*entry)(void*), const char* thread_name, void* arg )
{
    ThreadData* td = new ThreadData;
    td->name = thread_name;
    td->entry = entry;
    td->entry_noparam = NULL;
    td->arg = arg;

    inc_child_thread_count();

    create_thread( td, true );
}

void start_thread( void (*entry)(void), const char* thread_name )
{
    ThreadData* td = new ThreadData;
    td->name = thread_name;
    td->entry = NULL;
    td->entry_noparam = entry;
    td->arg = NULL;

    inc_child_thread_count();

    create_thread( td, true );
}

void ThreadMap::Register( int pid, const string& name )
{
    threadmap_lock();
    _contents.insert( make_pair(pid,name) );
    threadmap_unlock();
}
void ThreadMap::Unregister( int pid )
{
    threadmap_lock();
    _contents.erase( pid );
    threadmap_unlock();
}
void ThreadMap::CopyContents( Contents& out ) const
{
    threadmap_lock();
    out = _contents;
    threadmap_unlock();
}

}
