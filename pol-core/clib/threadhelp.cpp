/** @file
 *
 * @par History
 * - 2005/12/13 Shinigami: added error code printing in create_thread for debugging
 * - 2006/02/06 Shinigami: smaller bugfix in logging
 *                         error code printing in create_thread extended
 * - 2007/02/28 Shinigami: error code printing in create_thread added for linux
 * - 2007/03/08 Shinigami: added pthread_exit and _endhreadex to close threads
 * - 2008/03/02 Nando:     Added bool dec_child to create_thread, used to dec_child_thread_count()
 *                         if there is an error on create_thread. Will fix some of the zombies.
 */


#include "threadhelp.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstring>
#include <exception>
#include <thread>

#include "esignal.h"
#include "logfacility.h"
#include "passert.h"

#ifndef _WIN32
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#endif

// TODO: fix trunc cast warnings
#ifdef _MSC_VER
#pragma warning( disable : 4311 )  // trunc cast
#pragma warning( disable : 4302 )  // trunc cast
#endif


namespace Pol::threadhelp
{
using namespace std::chrono_literals;

ThreadMap& threadmap_instance()
{
  static ThreadMap threadmap;
  return threadmap;
}

std::atomic<unsigned int> child_threads( 0 );
static int threads = 0;

#ifdef _WIN32
void init_threadhelp() {}

void thread_sleep_ms( unsigned millis )
{
  Sleep( millis );
}
size_t thread_pid()
{
  return GetCurrentThreadId();
}

const DWORD MS_VC_EXCEPTION = 0x406D1388;

#pragma pack( push, 8 )
typedef struct tagTHREADNAME_INFO
{
  DWORD dwType;      // Must be 0x1000.
  LPCSTR szName;     // Pointer to name (in user addr space).
  DWORD dwThreadID;  // Thread ID (-1=caller thread).
  DWORD dwFlags;     // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack( pop )

void _SetThreadName( DWORD dwThreadID, const char* name )
{
  THREADNAME_INFO info;
  info.dwType = 0x1000;
  info.szName = name;
  info.dwThreadID = dwThreadID;
  info.dwFlags = 0;

  __try
  {  // oh my god i hate ms ...
    RaiseException( MS_VC_EXCEPTION, 0, sizeof( info ) / sizeof( ULONG_PTR ), (ULONG_PTR*)&info );
  }
  __except ( EXCEPTION_EXECUTE_HANDLER )
  {
  }
}
void SetThreadName( int threadid, std::string threadName )
{
  // This redirection is needed because std::string has a destructor
  // which isn't compatible with __try
  _SetThreadName( threadid, threadName.c_str() );
}
#else
static pthread_attr_t create_detached_attr;
static Clib::SpinLock pthread_attr_lock;

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
size_t thread_pid()
{
#ifdef __APPLE__
  return reinterpret_cast<size_t>( pthread_self() );
#else
  return pthread_self();
#endif
}
#endif

void run_thread( void ( *threadf )() )
{
  // thread creator calls inc_child_thread_count before starting thread
  try
  {
    ( *threadf )();
  }
  catch ( std::exception& ex )
  {
    ERROR_PRINTLN( "Thread exception: {}", ex.what() );
  }

  --child_threads;

  threadmap_instance().Unregister( thread_pid() );
}
void run_thread( void ( *threadf )( void* ), void* arg )
{
  // thread creator calls inc_child_thread_count before starting thread
  try
  {
    ( *threadf )( arg );
  }
  catch ( std::exception& ex )
  {
    ERROR_PRINTLN( "Thread exception: {}", ex.what() );
  }

  --child_threads;

  threadmap_instance().Unregister( thread_pid() );
}

class ThreadData
{
public:
  std::string name;
  void ( *entry )( void* );
  void ( *entry_noparam )();
  void* arg;
};

#ifdef _WIN32
unsigned __stdcall thread_stub2( void* v_td )
#else
void* thread_stub2( void* v_td )
#endif
{
  ThreadData* td = reinterpret_cast<ThreadData*>( v_td );

  void ( *entry )( void* ) = td->entry;
  void ( *entry_noparam )() = td->entry_noparam;
  void* arg = td->arg;

  threadmap_instance().Register( thread_pid(), td->name );

  delete td;
  td = nullptr;

  if ( entry != nullptr )
    run_thread( entry, arg );
  else
    run_thread( entry_noparam );

#ifdef _WIN32
  _endthreadex( 0 );
  return 0;
#else
  pthread_exit( nullptr );
  return nullptr;
#endif
}

#ifdef _WIN32
void create_thread( ThreadData* td, bool dec_child = false )
{
  // If the thread starts successfully, td will be deleted by thread_stub2.
  // So we must save the threadName for later.
  std::string threadName = td->name;

  unsigned threadid = 0;
  HANDLE h = (HANDLE)_beginthreadex( nullptr, 0, thread_stub2, td, 0, &threadid );
  if ( h == 0 )  // added for better debugging
  {
    POLLOG( "error in create_thread: {} {} \"{}\" \"{}\" {} {} {} {} {} {}\n", errno, _doserrno,
            strerror( errno ), strerror( _doserrno ), threads++, (unsigned)thread_stub2,
            td->name.c_str(), (unsigned)td->entry, (unsigned)td->entry_noparam, td->arg );

    // dec_child says that we should dec_child_threads when there's an error... :)
    if ( dec_child )
      --child_threads;
  }
  else
  {
    SetThreadName( threadid, threadName );
    CloseHandle( h );
  }
}
#else
void create_thread( ThreadData* td, bool dec_child = false )
{
  Clib::SpinLockGuard guard( pthread_attr_lock );
  pthread_t thread;
  int result = pthread_create( &thread, &create_detached_attr, thread_stub2, td );
  if ( result != 0 )  // added for better debugging
  {
    POLLOG( "error in create_thread: {} {} \"{}\" {} {} {} {} {} {}\n", result, errno,
            strerror( errno ), threads++, reinterpret_cast<const void*>( thread_stub2 ),
            td->name.c_str(), reinterpret_cast<const void*>( td->entry ),
            reinterpret_cast<const void*>( td->entry_noparam ), td->arg );

    // dec_child says that we should dec_child_threads when there's an error... :)
    if ( dec_child )
      --child_threads;
  }
}
#endif

void start_thread( void ( *entry )( void* ), const char* thread_name, void* arg )
{
  auto td = new ThreadData;
  td->name = thread_name;
  td->entry = entry;
  td->entry_noparam = nullptr;
  td->arg = arg;

  ++child_threads;

  create_thread( td, true );
}

void start_thread( void ( *entry )(), const char* thread_name )
{
  auto td = new ThreadData;
  td->name = thread_name;
  td->entry = nullptr;
  td->entry_noparam = entry;
  td->arg = nullptr;

  ++child_threads;

  create_thread( td, true );
}

ThreadMap::ThreadMap()
    : _spinlock(),
      _contents()
#ifdef _WIN32
      ,
      _handles()
#endif
{
}

#ifdef _WIN32
HANDLE ThreadMap::getThreadHandle( size_t pid ) const
{
  Clib::SpinLockGuard guard( _spinlock );
  auto itr = _handles.find( pid );
  if ( itr == _handles.end() )
  {
    return 0;
  }
  return itr->second;
}
#endif
void ThreadMap::Register( size_t pid, const std::string& name )
{
  Clib::SpinLockGuard guard( _spinlock );
  _contents.insert( std::make_pair( pid, name ) );
#ifdef _WIN32
  HANDLE hThread = 0;
  if ( !DuplicateHandle( GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &hThread, 0,
                         FALSE, DUPLICATE_SAME_ACCESS ) )
  {
    ERROR_PRINTLN( "failed to duplicate thread handle" );
    return;
  }
  _handles.insert( std::make_pair( pid, hThread ) );
#endif
}
void ThreadMap::Unregister( size_t pid )
{
  Clib::SpinLockGuard guard( _spinlock );
  _contents.erase( pid );
#ifdef _WIN32
  auto itr = _handles.find( pid );
  if ( itr != _handles.end() )
    CloseHandle( itr->second );
  _handles.erase( pid );
#endif
}
void ThreadMap::CopyContents( Contents& out ) const
{
  Clib::SpinLockGuard guard( _spinlock );
  out = _contents;
}

ThreadRegister::ThreadRegister( const std::string& name )
{
  threadmap_instance().Register( thread_pid(), name );
}
ThreadRegister::~ThreadRegister()
{
  threadmap_instance().Unregister( thread_pid() );
}


/// Creates a threadpool of workers.
/// blocks on deconstruction
/// eg:
/// TaskThreadPool workers;
/// for (....)
///   workers.push([&](){dosomework();});
TaskThreadPool::TaskThreadPool() : _done( false ), _msg_queue() {}

TaskThreadPool::TaskThreadPool( const std::string& name ) : _done( false ), _msg_queue()
{
  // get the count of processors
  unsigned int max_count = std::thread::hardware_concurrency();
  if ( !max_count )  // can fail so at least one
    max_count = 1;
  init( max_count, name );
}

TaskThreadPool::TaskThreadPool( unsigned int max_count, const std::string& name )
    : _done( false ), _msg_queue()
{
  init( max_count, name );
}

void TaskThreadPool::init( unsigned int max_count, const std::string& name )
{
  for ( unsigned int i = 0; i < max_count; ++i )
  {
    _threads.emplace_back(
        [this, name]()
        {
          ThreadRegister register_thread( "TaskPool " + name );
          auto f = msg();
          try
          {
            while ( !_done )
            {
              try
              {
                _msg_queue.pop_wait( &f );
                f();
              }
              catch ( std::exception& ex )
              {
                ERROR_PRINTLN( "Thread exception: {}", ex.what() );
                Clib::force_backtrace( true );
              }
            }
          }
          catch ( msg_queue::Canceled& )
          {
          }
          // purge the queue empty
          std::list<msg> remaining;
          _msg_queue.pop_remaining( &remaining );
          for ( auto& _f : remaining )
            _f();
        } );
  }
}

void TaskThreadPool::init_pool( unsigned int max_count, const std::string& name )
{
  if ( !_threads.empty() )
    return;
  init( max_count, name );
}

void TaskThreadPool::deinit_pool()
{
  if ( _threads.empty() )
    return;
  // send both done and cancel to wake up all workers
  _msg_queue.push_move(
      [&]()
      {
        _done = true;
        _msg_queue.cancel();
      } );
  for ( auto& thread : _threads )
    thread.join();
  _threads.clear();
}
TaskThreadPool::~TaskThreadPool()
{
  deinit_pool();
}

/// simply fire and forget only the deconstructor ensures the msg to be finished
void TaskThreadPool::push( msg&& msg )
{
  _msg_queue.push_move( std::move( msg ) );
}

/// returns a future which will be set once the msg is processed
std::future<bool> TaskThreadPool::checked_push( msg&& msg )
{
  auto promise = std::promise<bool>();
  auto ret = promise.get_future();
  _msg_queue.push_move(
      [promise = std::move( promise ), msg = std::move( msg )]() mutable
      {
        try
        {
          msg();
          promise.set_value( true );
        }
        catch ( ... )
        {
          promise.set_exception( std::current_exception() );
        }
      } );
  return ret;
}

size_t TaskThreadPool::size() const
{
  return _threads.size();
}


class DynTaskThreadPool::PoolWorker
{
public:
  PoolWorker( DynTaskThreadPool* parent, const std::string& name );
  PoolWorker( const PoolWorker& ) = delete;
  PoolWorker& operator=( const PoolWorker& ) = delete;
  void run();
  bool isretired() const;

  constexpr static size_t MIN_WORKER = 4;
  constexpr static auto TIMEOUT = 1min;

private:
  std::string _name;
  std::jthread _thread;
  DynTaskThreadPool* _parent;
  std::atomic<bool> _retired;
  struct BusyGuard
  {
    std::atomic<size_t>* _busy;
    BusyGuard( std::atomic<size_t>* busy ) : _busy( busy )
    {
      _busy->fetch_add( 1, std::memory_order_relaxed );
    }
    ~BusyGuard() { _busy->fetch_sub( 1, std::memory_order_relaxed ); }
  };
};
DynTaskThreadPool::PoolWorker::PoolWorker( DynTaskThreadPool* parent, const std::string& name )
    : _name( name ), _thread(), _parent( parent ), _retired( false )
{
  run();
}

bool DynTaskThreadPool::PoolWorker::isretired() const
{
  return _retired.load( std::memory_order_relaxed );
}

void DynTaskThreadPool::PoolWorker::run()
{
  _thread = std::jthread(
      [&]()
      {
        ThreadRegister register_thread( _name );
        ERROR_PRINTLN( "created pool worker {}", _name );
        auto f = msg();
        try
        {
          while ( !_parent->_done && !Clib::exit_signalled )
          {
            if ( !_parent->_msg_queue.pop_wait_for( &f, TIMEOUT ) )
            {
              std::lock_guard<std::mutex> guard( _parent->_pool_mutex );
              // we timed out: can we retire?
              if ( _parent->_live_threads > MIN_WORKER )
              {
                --_parent->_live_threads;
                _retired.store( true, std::memory_order_relaxed );
                ERROR_PRINTLN( "removed pool worker {}", _name );
                return;
              }
              continue;  // keep alive
            }

            {
              BusyGuard busy( &_parent->_busy_count );
              try
              {
                f();
              }
              catch ( std::exception& ex )
              {
                ERROR_PRINTLN( "Thread exception: {}", ex.what() );
                Clib::force_backtrace( true );
              }
            }
          }
        }
        catch ( msg_queue::Canceled& )
        {
        }
      } );
}

/// Creates a dynamic threadpool of workers.
/// if no idle worker is found creates a new worker thread
/// blocks on deconstruction
/// idle worker threads get destroyed after timeout until minimum count is reached
/// eg:
/// DynTaskThreadPool workers;
/// for (....)
///   workers.push([&](){dosomework();});
DynTaskThreadPool::DynTaskThreadPool( const std::string& name )
    : _done( false ), _msg_queue(), _pool_mutex(), _name( "DynTaskPool" + name )
{
}

void DynTaskThreadPool::prefill_workers()
{
  std::lock_guard<std::mutex> guard( _pool_mutex );
  for ( size_t i = 0; i < PoolWorker::MIN_WORKER; ++i )
  {
    _threads.emplace_back( new PoolWorker( this, fmt::format( "{} {}", _name, _live_threads ) ) );
    ++_live_threads;
  }
}

size_t DynTaskThreadPool::threadpoolsize() const
{
  std::lock_guard<std::mutex> guard( _pool_mutex );
  return _live_threads;
}

void DynTaskThreadPool::create_thread()
{
  std::lock_guard<std::mutex> guard( _pool_mutex );
  // remove timeout threads
  std::erase_if( _threads, []( const auto& w ) { return w->isretired(); } );
  // still atleast one idle worker left?
  size_t idle = _live_threads - _busy_count.load( std::memory_order_relaxed );
  if ( idle > 0 )
    return;

  _threads.emplace_back( new PoolWorker( this, fmt::format( "{} {}", _name, _live_threads ) ) );
  ++_live_threads;
}

DynTaskThreadPool::~DynTaskThreadPool()
{
  // send both done and cancel to wake up all workers
  _msg_queue.push_move(
      [&]()
      {
        _done = true;
        _msg_queue.cancel();
      } );
  _threads.clear();  // dont rely on order
}

/// simply fire and forget only the deconstructor ensures the msg to be finished
void DynTaskThreadPool::push( msg&& msg )
{
  create_thread();
  _msg_queue.push_move( std::move( msg ) );
}

/// returns a future which will be set once the msg is processed
std::future<bool> DynTaskThreadPool::checked_push( msg&& msg )
{
  auto promise = std::promise<bool>();
  auto ret = promise.get_future();
  create_thread();
  _msg_queue.push_move(
      [promise = std::move( promise ), msg = std::move( msg )]() mutable
      {
        try
        {
          msg();
          promise.set_value( true );
        }
        catch ( ... )
        {
          promise.set_exception( std::current_exception() );
        }
      } );
  return ret;
}
}  // namespace Pol::threadhelp
