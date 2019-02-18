/** @file
 *
 * @par History
 */


#ifndef CLIB_THREADHELP_H
#define CLIB_THREADHELP_H

#include <atomic>
#include <functional>
#include <future>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "Header_Windows.h"
#include "message_queue.h"
#include "spinlock.h"

namespace Pol
{
namespace threadhelp
{
extern std::atomic<unsigned int> child_threads;

void init_threadhelp();
void run_thread( void ( *threadf )( void ) );
void run_thread( void ( *threadf )( void* ), void* arg );

void start_thread( void ( *entry )( void* ), const char* thread_name, void* arg );
void start_thread( void ( *entry )( void ), const char* thread_name );

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

  ThreadMap();

private:
  mutable Clib::SpinLock _spinlock;
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
  ThreadRegister( const std::string& name );
  ~ThreadRegister();
};


class TaskThreadPool
{
  typedef std::function<void()> msg;
  typedef Clib::message_queue<msg> msg_queue;

public:
  TaskThreadPool();
  TaskThreadPool( const std::string& name );
  TaskThreadPool( unsigned int max_count, const std::string& name );
  TaskThreadPool( const TaskThreadPool& ) = delete;
  TaskThreadPool& operator=( const TaskThreadPool& ) = delete;
  ~TaskThreadPool();
  void push( const msg& msg );
  std::future<bool> checked_push( const msg& msg );
  size_t size() const;

  void init_pool( unsigned int max_count, const std::string& name );
  void deinit_pool();

private:
  void init( unsigned int max_count, const std::string& name );
  std::atomic<bool> _done;
  msg_queue _msg_queue;
  std::vector<std::thread> _threads;
};

class DynTaskThreadPool
{
  class PoolWorker;

  friend class PoolWorker;
  typedef std::function<void()> msg;
  typedef Clib::message_queue<msg> msg_queue;

public:
  DynTaskThreadPool( const std::string& name );
  ~DynTaskThreadPool();
  DynTaskThreadPool( const DynTaskThreadPool& ) = delete;
  DynTaskThreadPool& operator=( const DynTaskThreadPool& ) = delete;
  void push( const msg& msg );
  std::future<bool> checked_push( const msg& msg );
  size_t threadpoolsize() const;

protected:
  std::atomic<bool> _done;

private:
  void create_thread();
  msg_queue _msg_queue;
  std::vector<std::unique_ptr<PoolWorker>> _threads;
  mutable std::mutex _pool_mutex;
  std::string _name;
};


}  // namespace threadhelp
}  // namespace Pol
#endif  // CLIB_THREADHELP_H
