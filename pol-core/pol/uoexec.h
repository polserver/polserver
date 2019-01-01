/** @file
 *
 * @par History
 * - 2006/09/23 Shinigami: Script_Cycles, Sleep_Cycles and Script_passes uses 64bit now
 */


#ifndef __UOEXEC_H
#define __UOEXEC_H

#ifndef __BSCRIPT_EXECUTOR_H
#include "../bscript/executor.h"
#endif

#include <string>
#include <time.h>

#include "../clib/rawtypes.h"
#include "../clib/weakptr.h"

namespace Pol
{
namespace Module
{
class OSExecutorModule;
}
namespace Core
{
// const int SCRIPT_RUNAWAY_INTERVAL = 5000;

class UOExecutor final : public Bscript::Executor
{
  typedef Bscript::Executor base;

public:  // soon to be private ;)
  Module::OSExecutorModule* os_module;

public:
  UOExecutor();
  virtual ~UOExecutor();
  virtual size_t sizeEstimate() const override;

  bool suspend();
  bool revive();

  std::string state();

  u64 instr_cycles;
  u64 sleep_cycles;
  time_t start_time;

  u64 warn_runaway_on_cycle;
  u64 runaway_cycles;

  bool listens_to( unsigned int eventflag ) const;
  bool signal_event( Bscript::BObjectImp* eventimp );

  unsigned int eventmask;
  unsigned short area_size;
  unsigned short speech_size;

  bool can_access_offline_mobiles;
  bool auxsvc_assume_string;
  weak_ptr_owner<UOExecutor> weakptr;

  UOExecutor *pParent, *pChild;
};

inline bool UOExecutor::listens_to( unsigned int eventflag ) const
{
  return ( eventmask & eventflag ) ? true : false;
}

}  // namespace Core
}  // namespace Pol
#endif
