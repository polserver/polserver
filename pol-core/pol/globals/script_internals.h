#ifndef GLOBALS_SCRIPT_INTERNALS_H
#define GLOBALS_SCRIPT_INTERNALS_H

#include <deque>
#include <map>
#include <set>

#include "../../bscript/eprog.h"
#include "../../clib/maputil.h"
#include "../polclock.h"
#include "../reftypes.h"


namespace Pol::Core
{
class UOExecutor;

using ExecList = std::deque<UOExecutor*>;
using NoTimeoutHoldList = std::set<UOExecutor*>;
using HoldList = std::multimap<Core::polclock_t, Core::UOExecutor*>;
using ScriptStorage = std::map<std::string, ref_ptr<Bscript::EScriptProgram>, Clib::ci_cmp_pred>;
using PidList = std::map<unsigned int, UOExecutor*>;
using TimeoutHandle = HoldList::iterator;


enum HoldListType
{
  NO_LIST,
  TIMEOUT_LIST,
  NOTIMEOUT_LIST,
  DEBUGGER_LIST
};

enum WAIT_TYPE
{
  WAIT_SLEEP,
  WAIT_EVENT,
  WAIT_UNKNOWN
};

enum
{
  MAX_EVENTQUEUE_SIZE = 20
};

class ScriptScheduler
{
public:
  static const unsigned int PID_MIN;

  int priority_divide;

  // Consider moving the ScriptStorage to a different class.
  // It's not even used here besides to calculate how much memory is used.
  ScriptStorage scrstore;

  ScriptScheduler();
  ~ScriptScheduler();
  ScriptScheduler( const ScriptScheduler& ) = delete;
  ScriptScheduler& operator=( const ScriptScheduler& ) = delete;
  void deinitialize();

  struct Memory
  {
    size_t script_count;
    size_t script_size;
    size_t scriptstorage_count;
    size_t scriptstorage_size;
  };
  Memory estimateSize( bool verbose ) const;
  bool logScriptVariables( const std::string& name ) const;

  void run_ready();

  const ExecList& getRanlist();
  const ExecList& getRunlist();
  const HoldList& getHoldlist();
  const NoTimeoutHoldList& getNoTimeoutHoldlist();

  const PidList& getPidlist();

  // void revive_timeout(UOExecutor* exec);
  void revive_timeout( UOExecutor* exec, TimeoutHandle hold_itr );
  void revive_notimeout( UOExecutor* exec );
  void revive_debugged( UOExecutor* exec );

  // Adds a new executor to the queue directly
  void enqueue( UOExecutor* exec );

  // Sets up the executor before adding to the queue
  void schedule( UOExecutor* exec );


  // The following methods should go to a different class,
  // together with the pidlist and new_pid.

  // Gets new PID and store the executor in the pidlist
  unsigned int get_new_pid( UOExecutor* exec );

  void free_pid( unsigned int pid );

  // Finds an UOExecutor from a pid. Returns false if not found.
  bool find_exec( unsigned int pid, UOExecutor** exec );


private:
  ExecList runlist;
  ExecList ranlist;
  HoldList holdlist;
  NoTimeoutHoldList notimeoutholdlist;
  NoTimeoutHoldList debuggerholdlist;

  PidList pidlist;
  unsigned int next_pid;
};

const inline ExecList& ScriptScheduler::getRanlist()
{
  return ranlist;
}
const inline ExecList& ScriptScheduler::getRunlist()
{
  return runlist;
}
const inline HoldList& ScriptScheduler::getHoldlist()
{
  return holdlist;
}
const inline NoTimeoutHoldList& ScriptScheduler::getNoTimeoutHoldlist()
{
  return notimeoutholdlist;
}
const inline PidList& ScriptScheduler::getPidlist()
{
  return pidlist;
}

extern ScriptScheduler scriptScheduler;
}  // namespace Pol::Core

#endif
