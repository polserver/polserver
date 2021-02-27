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
#include "./globals/script_internals.h"
#include "skillid.h"

namespace Pol
{
namespace Module
{
class OSExecutorModule;
}

namespace Bscript
{
class BError;
class Executor;
}  // namespace Bscript
namespace Mobile
{
class Attribute;
class Character;
}  // namespace Mobile
namespace Items
{
class Item;
class ItemDesc;
}  // namespace Items
namespace Multi
{
class UBoat;
class UMulti;
}  // namespace Multi
namespace Network
{
class Client;
}

namespace Core
{
class UObject;
class Vital;
class Guild;
class Party;
// const int SCRIPT_RUNAWAY_INTERVAL = 5000;

class UOExecutor final : public Bscript::Executor
{
  typedef Bscript::Executor base;

private:
  Module::OSExecutorModule* os_module;
  bool keep_alive_;  // special flag scheduler will not delete the executor on progend

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

  virtual Bscript::ExecutorType type() override { return Bscript::ExecutorType::POL; }

  bool listens_to( unsigned int eventflag ) const;
  bool signal_event( Bscript::BObjectImp* eventimp );

  unsigned int eventmask;
  unsigned char area_mask;
  unsigned short area_size;
  unsigned short speech_size;
  
 

  bool can_access_offline_mobiles_;
  bool auxsvc_assume_string;
  bool survive_attached_disconnect;
  weak_ptr_owner<UOExecutor> weakptr;

  UOExecutor *pParent, *pChild;

public:
  bool critical() const;
  void critical( bool critical );

  unsigned char priority() const;
  void priority( unsigned char priority );

  bool warn_on_runaway() const;
  void warn_on_runaway( bool warn_on_runaway );

  bool keep_alive() const;
  void keep_alive( bool status );

  void SleepFor( u32 secs );
  void SleepForMs( u32 msecs );
  unsigned int pid() const;
  bool blocked() const;
  bool in_debugger_holdlist() const;
  void revive_debugged();

  Core::polclock_t sleep_until_clock() const;
  void sleep_until_clock( Core::polclock_t sleep_until_clock );

  Core::TimeoutHandle hold_itr() const;
  void hold_itr( Core::TimeoutHandle hold_itr );

  Core::HoldListType in_hold_list() const;
  void in_hold_list( Core::HoldListType in_hold_list );


  Bscript::BObjectImp* clear_event_queue();


public:
  bool getCharacterOrClientParam( unsigned param, Mobile::Character*& chrptr,
                                  Network::Client*& clientptr );
  bool getCharacterParam( unsigned param, Mobile::Character*& chrptr );
  bool getItemParam( unsigned param, Items::Item*& itemptr );
  bool getUBoatParam( unsigned param, Multi::UBoat*& boatptr );
  bool getMultiParam( unsigned param, Multi::UMulti*& multiptr );
  bool getUObjectParam( unsigned param, UObject*& objptr );
  bool getObjtypeParam( unsigned param, unsigned int& objtype );
  bool getObjtypeParam( unsigned param, const Items::ItemDesc*& itemdesc_out );
  bool getSkillIdParam( unsigned param, USKILLID& skillid );
  bool getAttributeParam( unsigned param, const Mobile::Attribute*& attr );
  bool getVitalParam( unsigned param, const Vital*& vital );
  bool getGuildParam( unsigned param, Core::Guild*& guild, Bscript::BError*& err );
  bool getPartyParam( unsigned param, Core::Party*& party, Bscript::BError*& err );
};

inline bool UOExecutor::listens_to( unsigned int eventflag ) const
{
  return ( eventmask & eventflag ) ? true : false;
}

}  // namespace Core
}  // namespace Pol
#endif
