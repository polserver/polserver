/*
History
=======


Notes
=======

*/

#ifndef THREADHELP_H
#define THREADHELP_H

#include "hbmutex.h"

namespace threadhelp
{
    extern unsigned int child_threads;
	static Mutex _ThreadhelpMutex;

    void init_threadhelp();
    void inc_child_thread_count( bool need_lock = true );
    void dec_child_thread_count( bool need_lock = true );
    void run_thread( void (*threadf)(void) );
    void run_thread( void (*threadf)(void *), void* arg );

    void start_thread( void (*entry)(void*), const char* thread_name, void* arg );
    void start_thread( void (*entry)(void), const char* thread_name );

    void thread_sleep_ms( unsigned milliseconds );
    unsigned thread_pid();

    class ThreadMap
    {
    public:
        typedef std::map<int,string> Contents;

        void Register( int pid, const string& name );
        void Unregister( int pid );
        void CopyContents( Contents& out );
    private:
        Contents _contents;
		Mutex _ThreadMapMutex;
    };
    extern ThreadMap threadmap;


} // namespace threadhelp

#endif
