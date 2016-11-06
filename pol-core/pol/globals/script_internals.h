#ifndef GLOBALS_SCRIPT_INTERNALS_H
#define GLOBALS_SCRIPT_INTERNALS_H

#include "../../clib/maputil.h"
#include "../../bscript/eprog.h"

#include "../reftypes.h"

#include "../polclock.h"
#include <boost/noncopyable.hpp>
#include <deque>
#include <set>
#include <map>

namespace Pol
{
namespace Core
{
class UOExecutor;

typedef std::deque<UOExecutor*> ExecList;
typedef std::set<UOExecutor*> NoTimeoutHoldList;
typedef std::multimap<Core::polclock_t, Core::UOExecutor*> HoldList;
typedef std::map<std::string, ref_ptr<Bscript::EScriptProgram>, Clib::ci_cmp_pred> ScriptStorage;
typedef std::map<unsigned int, UOExecutor*> PidList;
typedef HoldList::iterator TimeoutHandle;

class ScriptScheduler : boost::noncopyable
{
public:
  static const unsigned int PID_MIN;
  int priority_divide;
  ScriptStorage scrstore;
  PidList pidlist;
  unsigned int next_pid;

  ScriptScheduler();
  ~ScriptScheduler();
  void deinitialize();

  struct Memory
  {
    size_t script_count;
    size_t script_size;
    size_t scriptstorage_count;
    size_t scriptstorage_size;
  };
  Memory estimateSize() const;

  void run_ready();

  const ExecList& getRanlist();
  const ExecList& getRunlist();
  const HoldList& getHoldlist();
  const NoTimeoutHoldList& getNoTimeoutHoldlist();

  //void revive_timeout(UOExecutor* exec);
  void revive_timeout(UOExecutor* exec, TimeoutHandle hold_itr);
  void revive_notimeout(UOExecutor* exec);
  void revive_debugged(UOExecutor* exec);

  // Adds a new executor to the queue directly
  void enqueue(UOExecutor* exec);

  // Sets up the executor before adding to the queue
  void schedule(UOExecutor* exec);
  
private:
	ExecList runlist;
	ExecList ranlist;
	HoldList holdlist;
	NoTimeoutHoldList notimeoutholdlist;
	NoTimeoutHoldList debuggerholdlist;
};

const inline ExecList& ScriptScheduler::getRanlist() {
	return ranlist;
}
const inline ExecList& ScriptScheduler::getRunlist() {
	return runlist;
}
const inline HoldList& ScriptScheduler::getHoldlist() {
	return holdlist;
}
const inline NoTimeoutHoldList& ScriptScheduler::getNoTimeoutHoldlist() {
	return notimeoutholdlist;
}

inline void ScriptScheduler::revive_debugged(UOExecutor* exec) {
	debuggerholdlist.erase(exec);
	enqueue(exec);
}

inline void ScriptScheduler::revive_timeout(UOExecutor* exec, TimeoutHandle hold_itr)
{
	holdlist.erase(hold_itr);
	enqueue(exec);
}

inline void ScriptScheduler::revive_notimeout(UOExecutor* exec)
{
	notimeoutholdlist.erase(exec);
	enqueue(exec);
}

inline void ScriptScheduler::enqueue(UOExecutor* exec)
{
	runlist.push_back(exec);
}


extern ScriptScheduler scriptScheduler;
}
}
#endif
