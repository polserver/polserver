/** @file
 *
 * @par History
 * - 2006/10/28 Shinigami: GCC 4.1.1 fix - missing declaration of friend functions
 */


#ifndef __SCHEDULE_H
#define __SCHEDULE_H

#include <functional>
#include <string>

#include "polclock.h"

namespace Pol
{
namespace Core
{
class ScheduledTask;
class SchComparer : public std::less<ScheduledTask*>
{
public:
  bool operator()( const ScheduledTask* x, const ScheduledTask* y ) const;
};

class TaskScheduler
{
public:
  static bool is_dirty() { return dirty_; }
  static void cleanse() { dirty_ = false; }
  static void mark_dirty() { dirty_ = true; }

private:
  static bool dirty_;
};

class ScheduledTask
{
public:
  explicit ScheduledTask( polclock_t next_run_clock );
  virtual ~ScheduledTask() = default;

  bool ready( polclock_t now );
  bool late( polclock_t now );
  polclock_t ticks_late( polclock_t now );

  polclock_t clocksleft( polclock_t now );
  polclock_t next_run_clock() const;
  virtual void execute( polclock_t now ) = 0;

  virtual void cancel();

protected:
  bool cancelled;
  polclock_t next_run_clock_;
  polclock_t last_run_clock_;
  friend class SchComparer;
  friend void check_scheduled_tasks( polclock_t* clocksleft, bool* pactivity );
};

inline polclock_t ScheduledTask::next_run_clock() const
{
  return next_run_clock_;
}

void check_scheduled_tasks( polclock_t* clocksleft, bool* pactivity );

class PeriodicTask final : public ScheduledTask
{
public:
  PeriodicTask( void ( *f )(), int n_secs, const char* name );
  PeriodicTask( void ( *f )(), int initial_wait_seconds, int periodic_seconds, const char* name );
  ~PeriodicTask() override = default;

  void set_secs( int n_secs );

  void execute( polclock_t now ) override;
  void start();

  std::string name() const { return name_; }

private:
  polclock_t n_initial_clocks;
  polclock_t n_clocks;
  void ( *f )();
  const char* name_;
};

class OneShotTask : public ScheduledTask
{
public:
  OneShotTask( OneShotTask** handle, polclock_t run_when );
  void cancel() override;

protected:
  // oneshots can't be deleted, only cancelled.
  ~OneShotTask() override;
  void execute( polclock_t now ) override;

  virtual void on_run() = 0;

private:
  OneShotTask** handle;
};


template <class T>
class OneShotTaskInst final : public OneShotTask
{
public:
  OneShotTaskInst( OneShotTask** handle, polclock_t run_when, void ( *f )( T data ), T data )
      : OneShotTask( handle, run_when ), data_( data ), f_( f )
  {
  }
  ~OneShotTaskInst() override = default;

  void on_run() override;

private:
  T data_;
  void ( *f_ )( T data );
};

template <class T>
void OneShotTaskInst<T>::on_run()
{
  ( *f_ )( data_ );
}
}  // namespace Core
}  // namespace Pol
#endif
