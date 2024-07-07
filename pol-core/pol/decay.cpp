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

void decay_worldzone( const Pos2d& pos, Realms::Realm* realm )
{
  Zone& zone = realm->getzone_grid( pos );
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
      if ( gamestate.system_hooks.can_decay )
      {
        if ( !gamestate.system_hooks.can_decay->call( new Module::EItemRefObjImp( item ) ) )
          continue;
      }

      const Items::ItemDesc& descriptor = item->itemdesc();
      Multi::UMulti* multi = realm->find_supporting_multi( item->pos3d() );

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

void decay_single_zone( Realms::Realm* realm, const Pos2d& gridsize, Pos2d& pos )
{
  pos += Vec2d( 1, 0 );
  if ( pos.x() >= gridsize.x() )
  {
    pos.x( 0 );
    pos += Vec2d( 0, 1 );
    if ( pos.y() >= gridsize.y() )
      pos.y( 0 );
  }
  decay_worldzone( pos, realm );
}

void decay_thread( void* arg )  // Realm*
{
  Pos2d pos{ 0, 0 };
  Realms::Realm* realm = static_cast<Realms::Realm*>( arg );

  const auto& gridsize = realm->gridarea().se();

  unsigned sleeptime = ( 60 * 10L * 1000 ) / ( gridsize.x() * gridsize.y() );
  while ( !Clib::exit_signalled )
  {
    {
      PolLock lck;
      polclock_checkin();
      decay_single_zone( realm, gridsize, pos );
      restart_all_clients();
    }
    // sweep entire world every 10 minutes
    // (60 * 10 * 1000) / (96 * 64) -> (600000 / 6144) -> 97 ms

    pol_sleep_ms( sleeptime );
  }
}

void decay_thread_shadow( void* arg )  // Realm*
{
  Pos2d pos{ 0, 0 };
  unsigned id = static_cast<Realms::Realm*>( arg )->shadowid;

  if ( gamestate.shadowrealms_by_id[id] == nullptr )
    return;
  const auto& gridsize = gamestate.shadowrealms_by_id[id]->gridarea().se();

  unsigned sleeptime = ( 60 * 10L * 1000 ) / ( gridsize.x() * gridsize.y() );
  while ( !Clib::exit_signalled )
  {
    {
      PolLock lck;
      polclock_checkin();
      if ( gamestate.shadowrealms_by_id[id] == nullptr )  // is realm still there?
        break;
      decay_single_zone( gamestate.shadowrealms_by_id[id], gridsize, pos );
      restart_all_clients();
    }
    // sweep entire world every 10 minutes
    // (60 * 10 * 1000) / (96 * 64) -> (600000 / 6144) -> 97 ms

    pol_sleep_ms( sleeptime );
  }
}

bool should_switch_realm( size_t index, const Pos2d& pos, Core::Pos2d* gridsize )
{
  if ( index >= gamestate.Realms.size() )
    return true;
  Realms::Realm* realm = gamestate.Realms[index];
  if ( realm == nullptr )
    return true;

  ( *gridsize ) = realm->gridarea().se();

  // check if ++y would result in reset
  if ( pos.y() + 1 >= gridsize->y() )
    return true;
  return false;
}

void decay_single_thread( void* /*arg*/ )
{
  // calculate total grid count, based on current realms
  unsigned total_grid_count = 0;
  for ( const auto& realm : gamestate.Realms )
  {
    total_grid_count += ( realm->grid_width() * realm->grid_height() );
  }
  if ( !total_grid_count )
  {
    POLLOG_ERRORLN( "No realm grids?!" );
    return;
  }
  // sweep every realm ~10minutes -> 36ms for 6 realms
  unsigned sleeptime = ( 60 * 10L * 1000 ) / total_grid_count;
  sleeptime = std::min( std::max( sleeptime, 30u ), 500u );  // limit to 30ms-500ms
  bool init = true;
  size_t realm_index = ~0u;
  Pos2d pos{ 0, 0 };
  Pos2d gridsize = Pos2d( 0, 0 );
  while ( !Clib::exit_signalled )
  {
    {
      PolLock lck;
      polclock_checkin();
      // check if realm_index is still valid and if y is still in valid range
      if ( should_switch_realm( realm_index, pos, &gridsize ) )
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
            POLLOG_INFOLN(
                "DECAY STATISTICS: decayed: max {} mean {} variance {} runs {} active max {} mean "
                "{} variance {} runs {}",
                stateManager.decay_statistics.decayed.max(),
                stateManager.decay_statistics.decayed.mean(),
                stateManager.decay_statistics.decayed.variance(),
                stateManager.decay_statistics.decayed.count(),
                stateManager.decay_statistics.active_decay.max(),
                stateManager.decay_statistics.active_decay.mean(),
                stateManager.decay_statistics.active_decay.variance(),
                stateManager.decay_statistics.active_decay.count() );
          }
          init = false;
        }
        pos.x( 0 ).y( 0 );
      }
      else
      {
        pos += Vec2d( 1, 0 );
        if ( pos.x() >= gridsize.x() )
        {
          pos.x( 0 );
          pos += Vec2d( 0, 1 );
          if ( pos.y() >= gridsize.y() )
          {
            POLLOG_ERRORLN( "SHOULD NEVER HAPPEN" );
            pos.y( 0 );
          }
        }
      }
      decay_worldzone( pos, gamestate.Realms[realm_index] );
      restart_all_clients();
    }
    pol_sleep_ms( sleeptime );
  }
}
}  // namespace Core
}  // namespace Pol
