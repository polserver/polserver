/** @file
 *
 * @par History
 * - 2006/09/17 Shinigami: OSExecutorModule::signal_event() will return error on full evene queue
 */


#ifndef BSCRIPT_OSEMOD_H
#define BSCRIPT_OSEMOD_H

#ifndef POL_POLMODL_H
#include "../polmodl.h"
#endif

#include <ctime>
#include <deque>
#include <map>

#include "../globals/script_internals.h"
#include "../polclock.h"
#include "clib/rawtypes.h"

namespace Pol
{
namespace Bscript
{
class BObject;
class BObjectImp;
class Executor;
template <class T, class T2>
class TmplExecutorModule;
}  // namespace Bscript
namespace Mobile
{
class Character;
}  // namespace Mobile
}  // namespace Pol

namespace Pol
{
namespace Core
{
class UOExecutor;

void run_ready( void );
void check_blocked( polclock_t* pclocksleft );
void deschedule_executor( UOExecutor* ex );
}  // namespace Core
namespace Module
{
class OSExecutorModule : public Bscript::TmplExecutorModule<OSExecutorModule, Core::PolModule>
{
public:
  bool signal_event( Bscript::BObjectImp* eventimp );
  void suspend();
  void revive();

  explicit OSExecutorModule( Bscript::Executor& exec );
  ~OSExecutorModule() override;


  void SleepFor( u32 secs );
  void SleepForMs( u32 msecs );

  unsigned int pid() const;
  bool blocked() const;

  bool in_debugger_holdlist() const;
  void revive_debugged();
  Bscript::BObjectImp* clear_event_queue();  // DAVE

  size_t sizeEstimate() const override;

  bool critical() const;
  void critical( bool critical );

  bool warn_on_runaway() const;
  void warn_on_runaway( bool warn_on_runaway );

  unsigned char priority() const;
  void priority( unsigned char priority );

  Core::polclock_t sleep_until_clock() const;
  void sleep_until_clock( Core::polclock_t sleep_until_clock );

  Core::TimeoutHandle hold_itr() const;
  void hold_itr( Core::TimeoutHandle hold_itr );

  Core::HoldListType in_hold_list() const;
  void in_hold_list( Core::HoldListType in_hold_list );

  [[nodiscard]] Bscript::BObjectImp* mf_Create_Debug_Context();
  [[nodiscard]] Bscript::BObjectImp* mf_GetPid();
  [[nodiscard]] Bscript::BObjectImp* mf_GetProcess();
  [[nodiscard]] Bscript::BObjectImp* mf_Sleep();
  [[nodiscard]] Bscript::BObjectImp* mf_Sleepms();
  [[nodiscard]] Bscript::BObjectImp* mf_Wait_For_Event();
  [[nodiscard]] Bscript::BObjectImp* mf_Events_Waiting();
  [[nodiscard]] Bscript::BObjectImp* mf_Set_Critical();
  [[nodiscard]] Bscript::BObjectImp* mf_Is_Critical();

  [[nodiscard]] Bscript::BObjectImp* mf_Start_Script();
  [[nodiscard]] Bscript::BObjectImp* mf_Start_Skill_Script();
  [[nodiscard]] Bscript::BObjectImp* mf_Run_Script_To_Completion();
  [[nodiscard]] Bscript::BObjectImp* mf_Run_Script();
  [[nodiscard]] Bscript::BObjectImp* mf_Set_Debug();
  [[nodiscard]] Bscript::BObjectImp* mf_SysLog();
  [[nodiscard]] Bscript::BObjectImp* mf_Set_Priority();
  [[nodiscard]] Bscript::BObjectImp* mf_Unload_Scripts();
  [[nodiscard]] Bscript::BObjectImp* mf_Set_Script_Option();
  [[nodiscard]] Bscript::BObjectImp* mf_Set_Event_Queue_Size();  // DAVE 11/24
  [[nodiscard]] Bscript::BObjectImp* mf_OpenURL();
  [[nodiscard]] Bscript::BObjectImp* mf_OpenConnection();
  [[nodiscard]] Bscript::BObjectImp* mf_Debugger();
  [[nodiscard]] Bscript::BObjectImp* mf_HTTPRequest();

  [[nodiscard]] Bscript::BObjectImp* mf_Clear_Event_Queue();  // DAVE

  [[nodiscard]] Bscript::BObjectImp* mf_PerformanceMeasure();
  [[nodiscard]] Bscript::BObjectImp* mf_LoadExportedScript();
  [[nodiscard]] Bscript::BObjectImp* mf_GetEnvironmentVariable();
  [[nodiscard]] Bscript::BObjectImp* mf_SendEmail();

protected:
  bool critical_;
  unsigned char priority_;
  bool warn_on_runaway_;
  bool blocked_;
  Core::polclock_t sleep_until_clock_;  // 0 if wait forever

  Core::TimeoutHandle hold_itr_;
  Core::HoldListType in_hold_list_;
  Core::WAIT_TYPE wait_type;


  unsigned int pid_;


  unsigned short max_eventqueue_size;  // DAVE 11/24
  std::deque<Bscript::BObjectImp*> events_;


  friend class NPCExecutorModule;
  friend void step_scripts( void );
  // friend void Core::run_ready( void );
  friend void Core::check_blocked( Core::polclock_t* pclocksleft );
  friend void new_check_blocked( void );
  friend void Core::deschedule_executor( Core::UOExecutor* ex );

  void event_occurred( Bscript::BObject event );
};


inline bool OSExecutorModule::blocked() const
{
  return blocked_;
}
}  // namespace Module
}  // namespace Pol

#endif
