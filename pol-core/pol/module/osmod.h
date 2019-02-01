/** @file
 *
 * @par History
 * - 2006/09/17 Shinigami: OSExecutorModule::signal_event() will return error on full evene queue
 */


#ifndef BSCRIPT_OSEMOD_H
#define BSCRIPT_OSEMOD_H

#ifndef BSCRIPT_EXECMODL_H
#include "../../bscript/execmodl.h"
#endif

#include <ctime>
#include <deque>
#include <map>

#include "../globals/script_internals.h"
#include "../polclock.h"
#include "../uoexhelp.h"

namespace Pol
{
namespace Bscript
{
class BObject;
class BObjectImp;
class Executor;
template <class T>
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
polclock_t calc_script_clocksleft( polclock_t now );
}  // namespace Core
namespace Module
{
class OSExecutorModule : public Bscript::TmplExecutorModule<OSExecutorModule>
{
public:
  bool signal_event( Bscript::BObjectImp* eventimp );
  void suspend();
  void revive();

  explicit OSExecutorModule( Bscript::Executor& exec );
  ~OSExecutorModule();

  bool critical;
  unsigned char priority;
  bool warn_on_runaway;

  void SleepFor( int secs );
  void SleepForMs( int msecs );

  unsigned int pid() const;
  bool blocked() const;

  bool in_debugger_holdlist() const;
  void revive_debugged();
  Bscript::BObjectImp* clear_event_queue();  // DAVE

  virtual size_t sizeEstimate() const override;

protected:
  bool getCharacterParam( unsigned param, Mobile::Character*& chrptr );

  friend class Bscript::TmplExecutorModule<OSExecutorModule>;

  Bscript::BObjectImp* create_debug_context();
  Bscript::BObjectImp* getpid();
  Bscript::BObjectImp* getprocess();
  Bscript::BObjectImp* sleep();
  Bscript::BObjectImp* sleepms();
  Bscript::BObjectImp* wait_for_event();
  Bscript::BObjectImp* events_waiting();
  Bscript::BObjectImp* set_critical();
  Bscript::BObjectImp* is_critical();

  Bscript::BObjectImp* start_script();
  Bscript::BObjectImp* start_skill_script();
  Bscript::BObjectImp* run_script_to_completion();
  Bscript::BObjectImp* run_script();
  Bscript::BObjectImp* mf_parameter();
  Bscript::BObjectImp* mf_set_debug();
  Bscript::BObjectImp* mf_Log();
  Bscript::BObjectImp* mf_system_rpm();
  Bscript::BObjectImp* mf_set_priority();
  Bscript::BObjectImp* mf_unload_scripts();
  Bscript::BObjectImp* mf_set_script_option();
  Bscript::BObjectImp* mf_set_event_queue_size();  // DAVE 11/24
  Bscript::BObjectImp* mf_OpenURL();
  Bscript::BObjectImp* mf_OpenConnection();
  Bscript::BObjectImp* mf_debugger();
  Bscript::BObjectImp* mf_HTTPRequest();

  Bscript::BObjectImp* mf_clear_event_queue();  // DAVE

  Bscript::BObjectImp* mf_performance_diff();

  bool blocked_;
  Core::polclock_t sleep_until_clock_;  // 0 if wait forever

  enum
  {
    NO_LIST,
    TIMEOUT_LIST,
    NOTIMEOUT_LIST,
    DEBUGGER_LIST
  } in_hold_list_;
  Core::TimeoutHandle hold_itr_;

  unsigned int pid_;

  enum WAIT_TYPE
  {
    WAIT_SLEEP,
    WAIT_EVENT,
    WAIT_UNKNOWN
  } wait_type;

  enum
  {
    MAX_EVENTQUEUE_SIZE = 20
  };
  unsigned short max_eventqueue_size;  // DAVE 11/24
  std::deque<Bscript::BObjectImp*> events_;


  friend class NPCExecutorModule;
  friend void step_scripts( void );
  // friend void Core::run_ready( void );
  friend class Core::ScriptScheduler;  // TODO: REMOVE THIS AS SOON AS POSSIBLE!!!
  friend void Core::check_blocked( Core::polclock_t* pclocksleft );
  friend void new_check_blocked( void );
  friend void Core::deschedule_executor( Core::UOExecutor* ex );
  friend Core::polclock_t Core::calc_script_clocksleft( Core::polclock_t now );


  void event_occurred( Bscript::BObject event );
};

inline bool OSExecutorModule::getCharacterParam( unsigned param, Mobile::Character*& chrptr )
{
  return Core::getCharacterParam( exec, param, chrptr );
}

inline bool OSExecutorModule::blocked() const
{
  return blocked_;
}
}  // namespace Module
}  // namespace Pol

#endif
