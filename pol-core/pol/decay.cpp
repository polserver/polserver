/** @file
 *
 * @par History
 * - 2005/01/23 Shinigami: decay_items & decay_thread - Tokuno MapDimension doesn't fit blocks of
 * 64x64 (WGRID_SIZE)
 * - 2010/03/28 Shinigami: Transmit Pointer as Pointer and not Int as Pointer within
 * decay_thread_shadow
 */


#include "decay.h"

#include <stddef.h>

#include "../clib/esignal.h"
#include "../clib/logfacility.h"
#include "../plib/systemstate.h"
#include "gameclck.h"
#include "globals/state.h"
#include "globals/uvars.h"
#include "item/item.h"
#include "item/itemdesc.h"
#include "polcfg.h"
#include "polsem.h"
#include "realms/realm.h"
#include "scrdef.h"
#include "scrsched.h"
#include "syshook.h"
#include "ufunc.h"
#include "uoscrobj.h"
#include "uworld.h"


namespace Pol::Core
{
///
/// [1] Item Decay Criteria
///     An Item is allowed to decay if ALL of the following are true:
///        - it is not In Use
///        - it is Movable, OR it is a Corpse
///        - its 'decayat' member is nonzero
///            AND the Game Clock has passed this 'decayat' time
///        - it is not supported by a multi,
///            OR its itemdesc.cfg entry specifies 'DecaysOnMultis 1'
///        - it itemdesc.cfg entry specifies no 'DestroyScript',
///            OR its 'DestroyScript' returns nonzero.
///
/// [2] Decay Action
///     Container contents are moved to the ground at the Container's location
///     before destroying the container.
///

void Decay::decay_worldzone()
{
  auto* realm = gamestate.Realms[realm_index];
  Zone& zone = realm->getzone_grid( *area_itr );
  gameclock_t now = read_gameclock();
  bool statistics = Plib::systemstate.config.thread_decay_statistics;

  for ( ZoneItems::size_type idx = 0; idx < zone.items.size(); ++idx )
  {
    Items::Item* item = zone.items[idx];
    if ( statistics )
    {
      if ( item->can_decay() )
      {
        const Items::ItemDesc& descriptor = item->itemdesc();
        if ( !descriptor.decays_on_multis )
        {
          Multi::UMulti* multi = realm->find_supporting_multi( item->pos3d() );
          if ( multi == nullptr )
            stateManager.decay_statistics.temp_count_active++;
        }
        else
          stateManager.decay_statistics.temp_count_active++;
      }
    }
    if ( item->should_decay( now ) )
    {
      // check the CanDecay syshook first if it returns 1 go over to other checks
      bool skipchecks = false;
      if ( gamestate.system_hooks.can_decay )
      {
        auto res =
            gamestate.system_hooks.can_decay->call_long( new Module::EItemRefObjImp( item ) );
        if ( !res )
          continue;
        if ( res == SKIP_FURTHER_CHECKS )
          skipchecks = true;
      }

      const Items::ItemDesc& descriptor = item->itemdesc();
      Multi::UMulti* multi = realm->find_supporting_multi( item->pos3d() );
      if ( !skipchecks )
      {
        // some things don't decay on multis:
        if ( multi != nullptr && !descriptor.decays_on_multis )
          continue;
      }
      if ( statistics )
        stateManager.decay_statistics.temp_count_decayed++;

      if ( !descriptor.destroy_script.empty() && !item->inuse() )
      {
        bool decayok = call_script( descriptor.destroy_script, item->make_ref() );
        if ( !decayok )
          continue;
      }

      item->spill_contents( multi );
      destroy_item( item );
      --idx;
    }
  }
}

// check if realm_index is still valid and if y is still in valid range
bool Decay::should_switch_realm() const
{
  if ( realm_index >= gamestate.Realms.size() )
    return true;
  if ( gamestate.Realms[realm_index] == nullptr )
    return true;
  if ( area_itr == area.end() )
    return true;
  return false;
}

void Decay::switch_realm()
{
  ++realm_index;
  if ( realm_index >= gamestate.Realms.size() )
  {
    realm_index = 0;
    if ( Plib::systemstate.config.thread_decay_statistics )
    {
      auto& stat = stateManager.decay_statistics;
      stat.decayed.update( stat.temp_count_decayed );
      stat.active_decay.update( stat.temp_count_active );
      stat.temp_count_decayed = 0;
      stat.temp_count_active = 0;
      POLLOG_INFOLN(
          "DECAY STATISTICS: decayed: max {} mean {} variance {} runs {} active max {} mean "
          "{} variance {} runs {}",
          stat.decayed.max(), stat.decayed.mean(), stat.decayed.variance(), stat.decayed.count(),
          stat.active_decay.max(), stat.active_decay.mean(), stat.active_decay.variance(),
          stat.active_decay.count() );
    }
  }
  area = gamestate.Realms[realm_index]->gridarea();
  area_itr = area.begin();
}

void Decay::step()
{
  ++area_itr;
  if ( should_switch_realm() )
    switch_realm();
  decay_worldzone();
}


void Decay::on_delete_realm( Realms::Realm* realm )
{
  if ( realm_index >= gamestate.Realms.size() )  // uninit state
    return;
  if ( realm != gamestate.Realms[realm_index] )
    return;
  if ( realm_index + 1 >= gamestate.Realms.size() )  // already at the end
    return;
  // since the first realms are not deletable its save to subtract one and go to the end
  // due to the ++itr in step the realm will be switched
  --realm_index;
  area = gamestate.Realms[realm_index]->gridarea();
  area_itr = --area.end();
}

void Decay::after_realms_size_changed()
{
  calculate_sleeptime();
}

void Decay::calculate_sleeptime()
{
  // calculate total grid count, based on current realms
  unsigned total_grid_count = 0;
  for ( const auto& realm : gamestate.Realms )
  {
    total_grid_count += ( realm->grid_width() * realm->grid_height() );
  }
  if ( !total_grid_count )
  {
    sleeptime = 0;
    POLLOG_ERRORLN( "No realm grids?!" );
    return;
  }
  // sweep every realm ~10minutes -> 36ms for 6 realms
  // limit to 30ms-500ms
  sleeptime = std::clamp( ( 60u * 10u * 1000u ) / total_grid_count, 30u, 500u );
}
void Decay::decay_thread( void* /*arg*/ )
{
  auto& decay = gamestate.decay;
  decay.calculate_sleeptime();
  decay.threadloop();
}

void Decay::threadloop()
{
  while ( !Clib::exit_signalled )
  {
    if ( !sleeptime )
      return;
    {
      PolLock lck;
      polclock_checkin();
      step();
      restart_all_clients();
    }
    pol_sleep_ms( sleeptime );
  }
}
}  // namespace Pol::Core
