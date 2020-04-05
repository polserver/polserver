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


namespace Pol
{
namespace Core
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

void decay_worldzone( unsigned wx, unsigned wy, Realms::Realm* realm )
{
  Zone& zone = realm->zone[wx][wy];
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
          Multi::UMulti* multi = item->supporting_multi();
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
      if ( gamestate.system_hooks.can_decay )
      {
        if ( !gamestate.system_hooks.can_decay->call( new Module::EItemRefObjImp( item ) ) )
          continue;
      }

      const Items::ItemDesc& descriptor = item->itemdesc();
      Multi::UMulti* multi = item->supporting_multi();

      // some things don't decay on multis:
      if ( multi != nullptr && !descriptor.decays_on_multis )
        continue;

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


///
/// [3] Decay Sweep
///     Each 64x64 tile World Zone is checked for decay approximately
///     once every 10 minutes
///

void decay_single_zone( Realms::Realm* realm, unsigned gridx, unsigned gridy, unsigned& wx,
                        unsigned& wy )
{
  if ( ++wx >= gridx )
  {
    wx = 0;
    if ( ++wy >= gridy )
    {
      wy = 0;
    }
  }
  decay_worldzone( wx, wy, realm );
}

void decay_thread( void* arg )  // Realm*
{
  unsigned wx = ~0u;
  unsigned wy = 0;
  Realms::Realm* realm = static_cast<Realms::Realm*>( arg );

  unsigned gridwidth = realm->grid_width();
  unsigned gridheight = realm->grid_height();

  unsigned sleeptime = ( 60 * 10L * 1000 ) / ( gridwidth * gridheight );
  while ( !Clib::exit_signalled )
  {
    {
      PolLock lck;
      polclock_checkin();
      decay_single_zone( realm, gridwidth, gridheight, wx, wy );
      restart_all_clients();
    }
    // sweep entire world every 10 minutes
    // (60 * 10 * 1000) / (96 * 64) -> (600000 / 6144) -> 97 ms

    pol_sleep_ms( sleeptime );
  }
}

void decay_thread_shadow( void* arg )  // Realm*
{
  unsigned wx = ~0u;
  unsigned wy = 0;
  unsigned id = static_cast<Realms::Realm*>( arg )->shadowid;

  if ( gamestate.shadowrealms_by_id[id] == nullptr )
    return;
  unsigned gridwidth = gamestate.shadowrealms_by_id[id]->grid_width();
  unsigned gridheight = gamestate.shadowrealms_by_id[id]->grid_height();

  unsigned sleeptime = ( 60 * 10L * 1000 ) / ( gridwidth * gridheight );
  while ( !Clib::exit_signalled )
  {
    {
      PolLock lck;
      polclock_checkin();
      if ( gamestate.shadowrealms_by_id[id] == nullptr )  // is realm still there?
        break;
      decay_single_zone( gamestate.shadowrealms_by_id[id], gridwidth, gridheight, wx, wy );
      restart_all_clients();
    }
    // sweep entire world every 10 minutes
    // (60 * 10 * 1000) / (96 * 64) -> (600000 / 6144) -> 97 ms

    pol_sleep_ms( sleeptime );
  }
}

bool should_switch_realm( size_t index, unsigned x, unsigned y, unsigned* gridx, unsigned* gridy )
{
  (void)x;
  if ( index >= gamestate.Realms.size() )
    return true;
  Realms::Realm* realm = gamestate.Realms[index];
  if ( realm == nullptr )
    return true;

  ( *gridx ) = realm->grid_width();
  ( *gridy ) = realm->grid_height();

  // check if ++y would result in reset
  if ( y + 1 >= ( *gridy ) )
    return true;
  return false;
}

void decay_single_thread( void* arg )
{
  (void)arg;
  // calculate total grid count, based on current realms
  unsigned total_grid_count = 0;
  for ( const auto& realm : gamestate.Realms )
  {
    total_grid_count += ( realm->grid_width() * realm->grid_height() );
  }
  if ( !total_grid_count )
  {
    POLLOG_ERROR << "No realm grids?!\n";
    return;
  }
  // sweep every realm ~10minutes -> 36ms for 6 realms
  unsigned sleeptime = ( 60 * 10L * 1000 ) / total_grid_count;
  sleeptime = std::max( sleeptime, 30u );  // limit to 30ms
  bool init = true;
  size_t realm_index = ~0u;
  unsigned wx = 0;
  unsigned wy = 0;
  unsigned gridx = 0;
  unsigned gridy = 0;
  while ( !Clib::exit_signalled )
  {
    {
      PolLock lck;
      polclock_checkin();
      // check if realm_index is still valid and if y is still in valid range
      if ( should_switch_realm( realm_index, wx, wy, &gridx, &gridy ) )
      {
        ++realm_index;
        if ( realm_index >= gamestate.Realms.size() )
        {
          realm_index = 0;
          if ( !init && Plib::systemstate.config.thread_decay_statistics )
          {
            stateManager.decay_statistics.decayed.update(
                stateManager.decay_statistics.temp_count_decayed );
            stateManager.decay_statistics.active_decay.update(
                stateManager.decay_statistics.temp_count_active );
            stateManager.decay_statistics.temp_count_decayed = 0;
            stateManager.decay_statistics.temp_count_active = 0;
            POLLOG_INFO.Format(
                "DECAY STATISTICS: decayed: max {} mean {} variance {} runs {} active max {} mean "
                "{} variance {} runs {}\n" )
                << stateManager.decay_statistics.decayed.max()
                << stateManager.decay_statistics.decayed.mean()
                << stateManager.decay_statistics.decayed.variance()
                << stateManager.decay_statistics.decayed.count()
                << stateManager.decay_statistics.active_decay.max()
                << stateManager.decay_statistics.active_decay.mean()
                << stateManager.decay_statistics.active_decay.variance()
                << stateManager.decay_statistics.active_decay.count();
          }
          init = false;
        }
        wx = 0;
        wy = 0;
      }
      else
      {
        if ( ++wx >= gridx )
        {
          wx = 0;
          if ( ++wy >= gridy )
          {
            POLLOG_ERROR << "SHOULD NEVER HAPPEN\n";
            wy = 0;
          }
        }
      }
      decay_worldzone( wx, wy, gamestate.Realms[realm_index] );
      restart_all_clients();
    }
    pol_sleep_ms( sleeptime );
  }
}
}  // namespace Core
}  // namespace Pol
