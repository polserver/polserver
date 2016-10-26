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

class ScriptEngineInternalManager : boost::noncopyable
{
public:
  static const unsigned int PID_MIN;
  int priority_divide;
  ScriptStorage scrstore;
  PidList pidlist;
  unsigned int next_pid;

  ScriptEngineInternalManager();
  ~ScriptEngineInternalManager();
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
  void revive_timeout(UOExecutor* exec, HoldList::iterator hold_itr);
  void revive_notimeout(UOExecutor* exec);
  void revive_debugged(UOExecutor* exec);
  
  ExecList runlist;
private:
	ExecList ranlist;
	HoldList holdlist;
	NoTimeoutHoldList notimeoutholdlist;
	NoTimeoutHoldList debuggerholdlist;
};

const inline ExecList& ScriptEngineInternalManager::getRanlist() {
	return ranlist;
}
const inline ExecList& ScriptEngineInternalManager::getRunlist() {
	return runlist;
}
const inline HoldList& ScriptEngineInternalManager::getHoldlist() {
	return holdlist;
}
const inline NoTimeoutHoldList& ScriptEngineInternalManager::getNoTimeoutHoldlist() {
	return notimeoutholdlist;
}

inline void ScriptEngineInternalManager::revive_debugged(UOExecutor* exec) {
	debuggerholdlist.erase(exec);
	runlist.push_back(exec);
}

inline void ScriptEngineInternalManager::revive_timeout(UOExecutor* exec, HoldList::iterator hold_itr)
{
	holdlist.erase(hold_itr);
	runlist.push_back(exec);
}

inline void ScriptEngineInternalManager::revive_notimeout(UOExecutor* exec)
{
	notimeoutholdlist.erase(exec);
	runlist.push_back(exec);
}


extern ScriptEngineInternalManager scriptEngineInternalManager;
}
}
#endif
