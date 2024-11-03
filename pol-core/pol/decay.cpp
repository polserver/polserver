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
#include "../clib/timer.h"
#include "../plib/objtype.h"
#include "../plib/systemstate.h"
#include "gameclck.h"
#include "globals/state.h"
#include "globals/uvars.h"
#include "item/item.h"
#include "item/itemdesc.h"
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
WorldDecay::SerialFromDecayItem::result_type WorldDecay::SerialFromDecayItem::operator()(
    const WorldDecay::DecayItem& i ) const
{
  return i.obj->serial_ext;
}

WorldDecay::DecayItem::DecayItem( gameclock_t decaytime, ItemRef itemref )
    : time( decaytime ), obj( std::move( itemref ) )
{
}

WorldDecay::WorldDecay() : decay_cont() {}

void WorldDecay::addObject( Items::Item* item, gameclock_t decaytime )
{
  auto& indexByObj = decay_cont.get<IndexByObject>();
  auto res = indexByObj.emplace( decaytime, ItemRef( item ) );
  if ( !res.second )  // emplace failed, .first is itr of "blocking" entry
    indexByObj.modify( res.first, [&decaytime]( DecayItem& i ) { i.time = decaytime; } );
  else
    item->set_decay_task( true );
}

void WorldDecay::removeObject( Items::Item* item )
{
  auto& indexByObj = decay_cont.get<IndexByObject>();
  indexByObj.erase( item->serial_ext );  // ignore error?
  item->set_decay_task( false );
}

gameclock_t WorldDecay::getDecayTime( const Items::Item* obj ) const
{
  if ( !obj->has_decay_task() )
    return 0;
  auto& indexByObj = decay_cont.get<IndexByObject>();
  const auto& entry = indexByObj.find( obj->serial_ext );
  if ( entry == indexByObj.cend() )
    return 0;  // TODO error
  return entry->time;
}

void WorldDecay::decayTask()
{
  auto& indexByTime = decay_cont.get<IndexByTime>();
  auto now = read_gameclock();
  std::vector<DecayItem> decayitems;
  bool statistics = Plib::systemstate.config.thread_decay_statistics;
  if ( statistics )
    stateManager.decay_statistics.active_decay.update( indexByTime.size() );
  auto decayStats = []()
  {
    POLLOG_INFOLN(
        "DECAY STATISTICS: decayed: max {} mean {} variance {} runs {} active max {} "
        "mean "
        "{} variance {} runs {}",
        stateManager.decay_statistics.decayed.max(), stateManager.decay_statistics.decayed.mean(),
        stateManager.decay_statistics.decayed.variance(),
        stateManager.decay_statistics.decayed.count(),
        stateManager.decay_statistics.active_decay.max(),
        stateManager.decay_statistics.active_decay.mean(),
        stateManager.decay_statistics.active_decay.variance(),
        stateManager.decay_statistics.active_decay.count() );
  };
  // need to collect possible items
  // since script calls could add/remove in container
  for ( auto& v : indexByTime )
  {
    if ( v.time > now )
      break;
    decayitems.push_back( v );
  }
  if ( decayitems.empty() )  // early out
  {
    if ( statistics )
    {
      stateManager.decay_statistics.decayed.update( 0 );
      decayStats();
    }
    return;
  }

  std::vector<Items::Item*> destroyeditems;
  std::vector<Items::Item*> delayeditems;
  for ( auto& v : decayitems )
  {
    auto item = v.obj.get();
    if ( item->orphan() )
    {
      destroyeditems.push_back( item );
      continue;
    }
    // item::should_decay checks
    if ( item->inuse() )
    {
      delayeditems.push_back( item );
      continue;
    }
    // testing code TODO remove
    if ( item->owner() != nullptr )
    {
      POLLOG_INFOLN( "DECAY IS NOT TOPLEVEL: 0x{:#x} {}", item->serial, item->name() );
      continue;
    }
    if ( !item->movable() && item->objtype_ != UOBJ_CORPSE )
    {
      POLLOG_INFOLN( "DECAY IS NOT MOVABLE: 0x{:#x} {}", item->serial, item->name() );
      continue;
    }
    // check the CanDecay syshook first if it returns 1 go over to other checks
    bool skipchecks = false;
    if ( gamestate.system_hooks.can_decay )
    {
      auto res = gamestate.system_hooks.can_decay->call_long( new Module::EItemRefObjImp( item ) );
      if ( !res )
      {
        delayeditems.push_back( item );
        continue;
      }
      if ( res == Decay::SKIP_FURTHER_CHECKS )
        skipchecks = true;
    }
    auto multi = item->realm()->find_supporting_multi( item->pos3d() );
    // TODO DECAY what to do with this dynamic skipcheck
    // always add items in multis and remove them here once they would decay?
    if ( !skipchecks )
    {
      if ( !item->itemdesc().decays_on_multis )
      {
        if ( multi != nullptr )
        {
          POLLOG_INFOLN( "DECAY IS ON MULTI: 0x{:#x} {}", item->serial, item->name() );
          continue;
        }
      }
    }

    const auto& descriptor = item->itemdesc();
    if ( !descriptor.destroy_script.empty() )
    {
      if ( !call_script( descriptor.destroy_script, item->make_ref() ) )
      {
        delayeditems.push_back( item );
        continue;
      }
    }
    item->spill_contents( multi );
    destroy_item( item );
    destroyeditems.push_back( item );
  }

  auto& indexByObj = decay_cont.get<IndexByObject>();
  if ( statistics )
    stateManager.decay_statistics.decayed.update( destroyeditems.size() );
  for ( const auto& item : destroyeditems )
  {
    indexByObj.erase( item->serial_ext );
    item->set_decay_task( false );
  }
  for ( const auto& item : delayeditems )
  {
    if ( getDecayTime( item ) <= now )   // check if script has removed it or changed time
      addObject( item, now + 10 * 60 );  // delay by 10minutes like old decay system would behave
  }
  if ( statistics )
    decayStats();
}

void WorldDecay::initialize()
{
  POLLOG_INFO( "Initializing decay " );
  auto now = read_gameclock();
  Tools::Timer<> timer;
  for ( auto& realm : gamestate.Realms )
  {
    WorldIterator<ItemFilter>::InBox(
        realm->area(), realm,
        [&]( Items::Item* item )
        {
          if ( item->can_add_to_decay_task() )
          {
            if ( item->has_reldecay_time_loaded() )  // use stored reltime
            {
              Core::gamestate.world_decay.addObject( item, item->reldecay_time_loaded() + now );
              item->reldecay_time_loaded( 0 );
            }
            else
              Core::gamestate.world_decay.addObject( item, item->itemdesc().decay_time * 60 );
          }
          else
          {
            // no item should have this flag directly after loading, see uimport for container
            // items
            item->reldecay_time_loaded( 0 );
          }
        } );
    POLLOG_INFO( "." );
  }
  timer.stop();
  auto& indexByTime = decay_cont.get<IndexByTime>();
  POLLOG_INFOLN( " {} elements in {}ms.", indexByTime.size(), timer.ellapsed() );
}

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
