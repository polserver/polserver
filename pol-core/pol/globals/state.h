#ifndef GLOBALS_STATE_H
#define GLOBALS_STATE_H

#include "../polsig.h"
#include "../profile.h"
#include "../uobjcnt.h"

#include <boost/noncopyable.hpp>

namespace Pol {
namespace Core {

  const u32 CHARACTERSERIAL_START = 0x00000001Lu;
  const u32 CHARACTERSERIAL_END = 0x3FFFFFFFLu;
  const u32 ITEMSERIAL_START = 0x40000000Lu;
  const u32 ITEMSERIAL_END = 0x7FffFFffLu;


  class StateManager : boost::noncopyable
  {
  public:
	  StateManager();
	  ~StateManager();

	  void deinitialize();

	  const char* last_checkpoint;

	  size_t cycles_per_decay_worldzone;
	  size_t cycles_until_decay_worldzone;

	  bool gflag_enforce_container_limits;
	  bool gflag_in_system_load;
	  bool gflag_in_system_startup;

	  clock_t polclock_paused_at;

	  unsigned int stored_last_item_serial;
	  unsigned int stored_last_char_serial;

	  ProfileVars profilevars;

	  UObjCount uobjcount;

	  u32 itemserialnumber;
	  u32 charserialnumber;

	  PolSig polsig;

  };

  extern StateManager stateManager;
}
}
#endif
