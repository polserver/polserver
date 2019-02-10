#ifndef GLOBALS_STATE_H
#define GLOBALS_STATE_H

#include <atomic>

#include "../../clib/clib.h"
#include "../../clib/rawtypes.h"
#include "../polsig.h"
#include "../profile.h"
#include "../uobjcnt.h"

namespace Pol
{
namespace Core
{
const u32 CHARACTERSERIAL_START = 0x00000001Lu;
const u32 CHARACTERSERIAL_END = 0x3FFFFFFFLu;
const u32 ITEMSERIAL_START = 0x40000000Lu;
const u32 ITEMSERIAL_END = 0x7FffFFffLu;


class StateManager
{
public:
  StateManager();
  ~StateManager() = default;
  StateManager( const StateManager& ) = delete;
  StateManager& operator=( const StateManager& ) = delete;
  size_t estimateSize() const;

  void deinitialize();

  const char* last_checkpoint;

  size_t cycles_per_decay_worldzone;
  size_t cycles_until_decay_worldzone;

  bool gflag_enforce_container_limits;
  bool gflag_in_system_load;
  bool gflag_in_system_startup;

  unsigned int stored_last_item_serial;
  unsigned int stored_last_char_serial;

  ProfileVars profilevars;

  UObjCount uobjcount;

  u32 itemserialnumber;
  u32 charserialnumber;

  PolSig polsig;

  struct
  {
    Clib::OnlineStatistics decayed;
    Clib::OnlineStatistics active_decay;
    u32 temp_count_decayed;
    u32 temp_count_active;
  } decay_statistics;

  std::atomic<int> checkin_clock_times_out_at;
};

extern StateManager stateManager;
}  // namespace Core
}  // namespace Pol
#endif
