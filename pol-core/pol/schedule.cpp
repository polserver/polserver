/** @file
 *
 * @par History
 */


#include "schedule.h"

#include "../clib/logfacility.h"
#include "../clib/passert.h"
#include "../clib/tracebuf.h"
#include "../plib/systemstate.h"
#include "globals/state.h"
#include "globals/uvars.h"
#include "polclock.h"
#include "polsig.h"
#include "profile.h"

namespace Pol
{
namespace Core
{
bool SchComparer::operator()( const ScheduledTask* x, const ScheduledTask* y ) const
{
  if ( x->next_run_clock_ == y->next_run_clock_ )
    return x > y;
  else
    return x->next_run_clock_ > y->next_run_clock_;
}

bool TaskScheduler::dirty_ = false;

static void add_task( ScheduledTask* task )
{
  TaskScheduler::mark_dirty();
  gamestate.task_queue.push( task );
}

ScheduledTask::ScheduledTask( polclock_t next_run_clock )
    : cancelled( false ), next_run_clock_( next_run_clock ), last_run_clock_( 0 )
{
}

void ScheduledTask::cancel()
{
  cancelled = true;
}

inline bool ScheduledTask::ready( polclock_t now_clock )
{
  return ( now_clock >= next_run_clock_ );
}
inline bool ScheduledTask::late( polclock_t now_clock )
{
  return ( now_clock > next_run_clock_ );
}
inline polclock_t ScheduledTask::ticks_late( polclock_t now_clock )
{
  return ( now_clock - next_run_clock_ );
}

polclock_t ScheduledTask::clocksleft( polclock_t now_clock )
{
  return next_run_clock_ - now_clock;
}

PeriodicTask::PeriodicTask( void ( *i_f )( void ), int initial_wait_seconds, const char* name )
    : ScheduledTask( 0 ),
      n_initial_clocks( initial_wait_seconds * POLCLOCKS_PER_SEC ),
      n_clocks( initial_wait_seconds * POLCLOCKS_PER_SEC ),
      f( i_f ),
      name_( name )
{
}

PeriodicTask::PeriodicTask( void ( *i_f )( void ), int initial_wait_seconds, int periodic_seconds,
                            const char* name )
    : ScheduledTask( 0 ),
      n_initial_clocks( initial_wait_seconds * POLCLOCKS_PER_SEC ),
      n_clocks( periodic_seconds * POLCLOCKS_PER_SEC ),
      f( i_f ),
      name_( name )
{
}


void PeriodicTask::set_secs( int n_secs )
{
  n_clocks = n_secs * POLCLOCKS_PER_SEC;
  n_initial_clocks = n_secs * POLCLOCKS_PER_SEC;
}
void PeriodicTask::start()
{
  next_run_clock_ = polclock() + n_initial_clocks;
  last_run_clock_ = 0;
  add_task( this );
}

void PeriodicTask::execute( polclock_t now )
{
  last_run_clock_ = now;
  next_run_clock_ += n_clocks;
  ( *f )();
}

OneShotTask::OneShotTask( OneShotTask** handle, polclock_t run_when_clock )
    : ScheduledTask( run_when_clock ), handle( handle )
{
  if ( handle != nullptr )
  {
    passert( *handle == nullptr );
    *handle = this;
  }

  add_task( this );
}

OneShotTask::~OneShotTask()
{
  if ( handle != nullptr )
    *handle = nullptr;
}

void OneShotTask::cancel()
{
  ScheduledTask::cancel();
  if ( handle != nullptr )
    *handle = nullptr;
  handle = nullptr;
}
void OneShotTask::execute( polclock_t /*now_clock*/ )
{
  if ( !cancelled )
  {
    cancel();
    on_run();
  }
}

void check_scheduled_tasks( polclock_t* clocksleft, bool* pactivity )
{
  THREAD_CHECKPOINT( tasks, 101 );
  TaskScheduler::cleanse();

  polclock_t now_clock = polclock();
  TRACEBUF_ADDELEM( "check_scheduled_tasks now_clock", now_clock );
  bool activity = false;
  passert( !gamestate.task_queue.empty() );
  THREAD_CHECKPOINT( tasks, 102 );
  for ( ;; )
  {
    THREAD_CHECKPOINT( tasks, 103 );
    ScheduledTask* task = gamestate.task_queue.top();
    passert_paranoid( task != nullptr );
    TRACEBUF_ADDELEM( "check_scheduled_tasks toptask->nextrun", task->next_run_clock() );
    THREAD_CHECKPOINT( tasks, 104 );
    if ( !task->ready( now_clock ) )
    {
      *clocksleft = task->clocksleft( now_clock );
      *pactivity = activity;
      TRACEBUF_ADDELEM( "check_scheduled_tasks clocksleft", *clocksleft );
      return;
    }

    THREAD_CHECKPOINT( tasks, 105 );
    if ( !task->late( now_clock ) )
    {
      INC_PROFILEVAR( tasks_ontime );
    }
    else
    {
      INC_PROFILEVAR( tasks_late );
      INC_PROFILEVAR_BY( tasks_late_ticks, static_cast<u32>( task->ticks_late( now_clock ) ) );
    }

    THREAD_CHECKPOINT( tasks, 106 );
    gamestate.task_queue.pop();

    THREAD_CHECKPOINT( tasks, 107 );
    task->execute( now_clock );
    THREAD_CHECKPOINT( tasks, 108 );

    activity = true;

    if ( task->cancelled )
    {
      THREAD_CHECKPOINT( tasks, 109 );
      delete task;
    }
    else
    {
      THREAD_CHECKPOINT( tasks, 110 );
      gamestate.task_queue.push( task );
    }
    THREAD_CHECKPOINT( tasks, 111 );
  }
}

}  // namespace Core
}  // namespace Pol
