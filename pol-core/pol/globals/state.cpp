#include "state.h"


namespace Pol {
namespace Core {
  StateManager stateManager;

  StateManager::StateManager() :
	last_checkpoint(),
	cycles_per_decay_worldzone(0),
	cycles_until_decay_worldzone(0),
	gflag_enforce_container_limits(true),
	gflag_in_system_load(false),
	gflag_in_system_startup(false),
	polclock_paused_at(0),
	stored_last_item_serial(0),
	stored_last_char_serial(0),
	profilevars(),
	uobjcount(),
	itemserialnumber(ITEMSERIAL_START),
	charserialnumber(CHARACTERSERIAL_START),
	polsig(),
    decay_statistics()
  {
  
  }

  StateManager::~StateManager()
  {
  }


  void StateManager::deinitialize()
  {

  }

  size_t StateManager::estimateSize() const
  {
    return sizeof(StateManager);
  }
}
}
