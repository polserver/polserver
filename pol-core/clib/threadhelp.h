/*
History
=======


Notes
=======

*/

#ifndef THREADHELP_H
#define THREADHELP_H

#include <thread>
#include <future>
#include "message_queue.h"

namespace threadhelp
{
    extern unsigned int child_threads;

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
        typedef std::map<int,std::string> Contents;

        void Register( int pid, const std::string& name );
        void Unregister( int pid );
        void CopyContents( Contents& out ) const;
    private:
        Contents _contents;
    };
    extern ThreadMap threadmap;
#ifdef _WIN32
	void SetThreadName( int dwThreadID, std::string threadName);
#endif



	class LocalThreadPool
	{
		typedef std::function<void()> msg;
		typedef message_queue<msg> msg_queue;
	public:
		LocalThreadPool();
		LocalThreadPool(unsigned int max_count);
		~LocalThreadPool();
		void push(msg msg);
		std::future<bool> checked_push(msg msg);
	private:
		void init(unsigned int max_count);
		bool _done;
		msg_queue _msg_queue;
		std::vector<std::thread> _threads;
	};


} // namespace threadhelp

#endif
