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
unsigned int child_threads = 0;
static int threads = 0;

#ifdef _WIN32
#include <windows.h>
#include <process.h>
void init_threadhelp()
{
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

static pthread_attr_t create_detached_attr;


void init_threadhelp()
{
    int res;
    res = pthread_attr_init( &create_detached_attr );
    passert_always( res == 0 );
    res = pthread_attr_setdetachstate( &create_detached_attr, PTHREAD_CREATE_DETACHED );
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
        _ThreadhelpMutex.lock(); //protect child_threads
    
    ++child_threads;
    
    if (need_lock)
        _ThreadhelpMutex.unlock();
}
void dec_child_thread_count( bool need_lock )
{
    if (need_lock)
        _ThreadhelpMutex.lock(); //protect child_threads
    
    --child_threads;
    
    if (need_lock)
        _ThreadhelpMutex.unlock();
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
    unsigned threadid = 0;
    HANDLE h = (HANDLE) _beginthreadex( NULL, 0, thread_stub2, td, 0, &threadid );
	if (h == 0) // added for better debugging
	{
		_ThreadhelpMutex.lock(); //protect threads
		Log( "error in create_thread: %d %d \"%s\" \"%s\" %d %d %s %d %d %d\n",
			 errno, _doserrno, strerror( errno ), strerror( _doserrno ),
			 threads++, thread_stub2, td->name.c_str(), td->entry, td->entry_noparam, td->arg );
		_ThreadhelpMutex.unlock();
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
		_ThreadhelpMutex.lock(); //protect threads
		Log( "error in create_thread: %d %d \"%s\" %d %d %s %d %d %d\n",
			 result, errno, strerror( errno ), 
			 threads++, thread_stub2, td->name.c_str(), td->entry, td->entry_noparam, td->arg );
		_ThreadhelpMutex.unlock();
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
    _ThreadMapMutex.lock(); //protect _contents
    _contents.insert( make_pair(pid,name) );
    _ThreadMapMutex.unlock();
}
void ThreadMap::Unregister( int pid )
{
    _ThreadMapMutex.lock(); //protect _contents
    _contents.erase( pid );
    _ThreadMapMutex.unlock();
}
void ThreadMap::CopyContents( Contents& out )
{
    _ThreadMapMutex.lock(); //protect _contents
    out = _contents;
    _ThreadMapMutex.unlock();
}

}
