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
#include <map>

#include "message_queue.h"

namespace Pol {
  namespace threadhelp {
	extern unsigned int child_threads;

	void init_threadhelp();
	void inc_child_thread_count( bool need_lock = true );
	void dec_child_thread_count( bool need_lock = true );
	void run_thread( void( *threadf )( void ) );
	void run_thread( void( *threadf )( void * ), void* arg );

	void start_thread( void( *entry )( void* ), const char* thread_name, void* arg );
	void start_thread( void( *entry )( void ), const char* thread_name );

	void thread_sleep_ms( unsigned milliseconds );
    size_t thread_pid();

	class ThreadMap
	{
	public:
      typedef std::map<size_t, std::string> Contents;
#ifdef _WIN32
      typedef std::map<size_t, HANDLE> HANDLES;
      HANDLE getThreadHandle( size_t pid ) const;
#endif
      void Register( size_t pid, const std::string& name );
      void Unregister( size_t pid );
	  void CopyContents( Contents& out ) const;
	private:
	  Contents _contents;
#ifdef _WIN32
      HANDLES _handles;
#endif
	};
	extern ThreadMap threadmap;
#ifdef _WIN32
	void SetThreadName( int dwThreadID, std::string threadName );
#endif

    class ThreadRegister
    {
    public:
      ThreadRegister(const std::string &name);
      ~ThreadRegister();
    };



    class TaskThreadPool : boost::noncopyable
	{
	  typedef std::function<void()> msg;
	  typedef Clib::message_queue<msg> msg_queue;
	public:
      TaskThreadPool( const std::string& name );
	  TaskThreadPool( unsigned int max_count, const std::string& name );
	  ~TaskThreadPool();
	  void push( msg msg );
	  std::future<bool> checked_push( msg msg );
	private:
      void init( unsigned int max_count, const std::string& name );
	  bool _done;
	  msg_queue _msg_queue;
	  std::vector<std::thread> _threads;
	};

    class DynTaskThreadPool : boost::noncopyable
    {
      class PoolWorker;
      friend class PoolWorker;
      typedef std::function<void()> msg;
      typedef Clib::message_queue<msg> msg_queue;
    public:
      DynTaskThreadPool( const std::string& name );
      ~DynTaskThreadPool();
      void push( msg msg );
      std::future<bool> checked_push( msg msg );
      size_t threadpoolsize() const;
    protected:
      bool _done;
    private:
      void create_thread();
      msg_queue _msg_queue;
      std::vector<std::unique_ptr<PoolWorker>> _threads;
      mutable std::mutex _pool_mutex;
      std::string _name;
    };


  } // namespace threadhelp
}
#endif
