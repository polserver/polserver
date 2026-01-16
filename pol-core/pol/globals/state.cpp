#include "state.h"


namespace Pol::Core
{
StateManager stateManager;

StateManager::StateManager()
    : last_checkpoint(),
      gflag_enforce_container_limits( true ),
      gflag_in_system_load( false ),
      gflag_in_system_startup( false ),
      stored_last_item_serial( 0 ),
      stored_last_char_serial( 0 ),
      profilevars(),
      uobjcount(),
      itemserialnumber( ITEMSERIAL_START ),
      charserialnumber( CHARACTERSERIAL_START ),
      polsig(),
      decay_statistics(),
      checkin_clock_times_out_at( 0 )
{
}

void StateManager::deinitialize() {}

size_t StateManager::estimateSize() const
{
  return sizeof( StateManager );
}
}  // namespace Pol::Core
