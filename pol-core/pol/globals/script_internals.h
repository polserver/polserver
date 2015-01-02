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

namespace Pol {
namespace Core {
  class UOExecutor;

  typedef std::deque<UOExecutor*> ExecList;
  typedef std::set<UOExecutor*> NoTimeoutHoldList;
  typedef std::multimap<Core::polclock_t, Core::UOExecutor*> HoldList;
  typedef std::map< std::string, ref_ptr<Bscript::EScriptProgram>, Clib::ci_cmp_pred > ScriptStorage;
  typedef std::map<unsigned int, UOExecutor*> PidList;

  class ScriptEngineInternalManager : boost::noncopyable
  {
  public:
	  ScriptEngineInternalManager();
	  ~ScriptEngineInternalManager();

	  void deinitialize();

	  ExecList runlist;
	  ExecList ranlist;
	  HoldList holdlist;
	  NoTimeoutHoldList notimeoutholdlist;
	  NoTimeoutHoldList debuggerholdlist;
	  int priority_divide;
	  ScriptStorage scrstore;
	  PidList pidlist;
	  unsigned int next_pid;
  };

  extern ScriptEngineInternalManager scriptEngineInternalManager;
}
}
#endif
