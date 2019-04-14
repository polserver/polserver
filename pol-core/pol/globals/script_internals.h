#ifndef GLOBALS_SCRIPT_INTERNALS_H
#define GLOBALS_SCRIPT_INTERNALS_H

#include <deque>
#include <map>
#include <set>

#include "../../bscript/eprog.h"
#include "../../clib/maputil.h"
#include "../polclock.h"
#include "../reftypes.h"

namespace Pol
{
namespace Core
{
class UOExecutor;

typedef std::deque<UOExecutor*> ExecList;
typedef std::set<UOExecutor*> NoTimeoutHoldList;
typedef std::multimap<Core::polclock_t, Core::UOExecutor*> HoldList;
typedef std::map<std::string, ref_ptr<Bscript::Program>, Clib::ci_cmp_pred> ScriptStorage;
typedef std::map<unsigned int, UOExecutor*> PidList;
typedef HoldList::iterator TimeoutHandle;
typedef NoTimeoutHoldList ExternalHoldList; // for now..


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
  const ExternalHoldList& getExternalHoldlist();


  const PidList& getPidlist();

  // void revive_timeout(UOExecutor* exec);
  void revive_timeout( UOExecutor* exec, TimeoutHandle hold_itr );
  void revive_notimeout( UOExecutor* exec );
  void revive_debugged( UOExecutor* exec );

  // Remove an externally-mananged script.
  void add_externalscript( UOExecutor* exec );
  // Remove an externally-mananged script.
  void free_externalscript( UOExecutor* exec );

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
  /** Holds a list of externally running scripts. */
  ExternalHoldList externalholdlist;

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

const inline ExternalHoldList& ScriptScheduler::getExternalHoldlist()
{
  return externalholdlist;
}

const inline NoTimeoutHoldList& ScriptScheduler::getNoTimeoutHoldlist()
{
  return notimeoutholdlist;
}
const inline PidList& ScriptScheduler::getPidlist()
{
  return pidlist;
}

inline void ScriptScheduler::revive_debugged( UOExecutor* exec )
{
  debuggerholdlist.erase( exec );
  enqueue( exec );
}

inline void ScriptScheduler::revive_timeout( UOExecutor* exec, TimeoutHandle hold_itr )
{
  holdlist.erase( hold_itr );
  enqueue( exec );
}

inline void ScriptScheduler::revive_notimeout( UOExecutor* exec )
{
  notimeoutholdlist.erase( exec );
  enqueue( exec );
}

inline void ScriptScheduler::enqueue( UOExecutor* exec )
{
  runlist.push_back( exec );
}

inline void ScriptScheduler::free_pid( unsigned int pid )
{
  pidlist.erase( pid );
}

extern ScriptScheduler scriptScheduler;
}  // namespace Core
}  // namespace Pol
#endif
