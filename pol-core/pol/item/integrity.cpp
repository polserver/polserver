#include "pol/item/integrity.h"

#include <algorithm>
#include <array>
#include <map>
#include <string>

#include "clib/logfacility.h"
#include "pol/containr.h"
#include "pol/getitem.h"
#include "pol/globals/object_storage.h"
#include "pol/globals/uvars.h"
#include "pol/item/item.h"
#include "pol/item/location.h"
#include "pol/layers.h"
#include "pol/mobile/charactr.h"
#include "pol/mobile/corpse.h"
#include "pol/mobile/wornitems.h"
#include "pol/multi/house.h"
#include "pol/multi/multi.h"
#include "pol/objecthash.h"
#include "pol/polclass.h"
#include "pol/realms/realm.h"
#include "pol/storage.h"
#include "pol/uworld.h"

namespace Pol::Items
{
namespace
{
/// How many times a container lists this exact item among its direct contents. More than one is
/// itself a finding, which is why this counts instead of returning a bool.
unsigned times_listed_in( const Core::UContainer& cont, const Item& item )
{
  unsigned seen = 0;
  for ( const auto* held : cont )
  {
    if ( held == &item )
      ++seen;
  }
  return seen;
}

class Sweep
{
public:
  IntegrityReport report;

  void note( const Item& item, const std::string& what )
  {
    ++report.violations;
    POLLOG_ERRORLN( "item integrity: item {:#x} ({}): {}", item.serial, item.location().describe(),
                    what );
  }

  /**
   * Record that a registry lists this item, and report it if another already did.
   *
   * This is the cross-check the two directions cannot make on their own: an item can be listed by
   * two registries while agreeing with one of them, so both directions pass and the world still
   * has an item in two places. Keyed on the pointer, not the serial -- a destroyed item's serial
   * is zero, so serials are not unique here.
   */
  void claim( const Item& item, const std::string& by )
  {
    auto [entry, fresh] = claims_.try_emplace( &item, by );
    if ( !fresh )
    {
      ++report.violations;
      POLLOG_ERRORLN( "item integrity: item {:#x} is listed by two registries: {} and {}",
                      item.serial, entry->second, by );
    }
  }

private:
  std::map<const Item*, std::string> claims_;
};

/**
 * Does the registry the item names still hold it?
 *
 * Says nothing about registries the item does *not* name -- that is the backward pass's job, and
 * it is where the interesting failures are: an item that has been looted off a corpse honestly
 * reports the backpack it is honestly in, so only walking the corpse finds the stale entry.
 */
void check_forward( Sweep& sweep, Item& item )
{
  const Location loc = item.location();

  // A realm is part of a position, so only something actually placed has one of its own. Whatever
  // is held by something else used to carry a borrowed copy of its holder's, kept true by a cascade
  // that nothing ever read; what replaced that cascade is the stronger claim that the copy is not
  // there at all. Detached is deliberately not on this list -- it is the state an item passes
  // through mid-move, and its position is kept precisely so a refused move can be rolled back.
  const bool held_by_something = loc.holds<InContainer>() || loc.holds<OnCorpse>() ||
                                 loc.holds<Equipped>() || loc.holds<OnCursor>() ||
                                 loc.holds<InStorage>();
  if ( held_by_something && item.stored_realm() != nullptr )
  {
    sweep.note( item, fmt::format( "is held by something else but still claims to be in realm {}",
                                   item.stored_realm()->name() ) );
  }

  if ( const auto* in_world = loc.get_if<InWorld>(); in_world != nullptr )
  {
    if ( item.stored_realm() == nullptr )
    {
      sweep.note( item, "in the world with no realm" );
      return;
    }
    const Core::ZoneItems& zone = item.stored_realm()->getzone( item.pos().xy() ).items;
    if ( std::find( zone.begin(), zone.end(), &item ) == zone.end() )
      sweep.note( item, fmt::format( "not in the zone for its position {}", item.pos() ) );
  }
  else if ( const auto* in_cont = loc.get_if<InContainer>(); in_cont != nullptr )
  {
    if ( in_cont->cont == nullptr )
      sweep.note( item, "names a null container" );
    else if ( unsigned seen = times_listed_in( *in_cont->cont, item ); seen != 1 )
      sweep.note( item, fmt::format( "container {:#x} lists it {} times, expected once",
                                     in_cont->cont->serial, seen ) );
  }
  else if ( const auto* equipped = loc.get_if<Equipped>(); equipped != nullptr )
  {
    if ( equipped->chr == nullptr )
      sweep.note( item, "names a null character" );
    else if ( equipped->chr->worn_items()->GetItemOnLayer( equipped->layer ) != &item )
      sweep.note( item, fmt::format( "character {:#x} does not wear it on that layer",
                                     equipped->chr->serial ) );
  }
  else if ( const auto* on_corpse = loc.get_if<OnCorpse>(); on_corpse != nullptr )
  {
    if ( on_corpse->corpse == nullptr )
      sweep.note( item, "names a null corpse" );
    else
    {
      // No "does the corpse render it on that layer" check any more: the corpse's layer view is
      // derived from this very answer, so it could only agree. What can still go wrong is two
      // items claiming one layer, and that is asked from the corpse's side below.
      if ( unsigned seen = times_listed_in( *on_corpse->corpse, item ); seen != 1 )
        sweep.note( item, fmt::format( "corpse {:#x} lists it {} times among its contents, "
                                       "expected once",
                                       on_corpse->corpse->serial, seen ) );
    }
  }
  else if ( const auto* on_cursor = loc.get_if<OnCursor>(); on_cursor != nullptr )
  {
    // Derived from the gotten_by link, so the item's half is true by construction; the holder's
    // half is a separate field and is what can go missing.
    if ( on_cursor->holder == nullptr )
      sweep.note( item, "names a null cursor holder" );
    else if ( !on_cursor->holder->has_gotten_item() ||
              on_cursor->holder->gotten_item().item() != &item )
      sweep.note( item, fmt::format( "character {:#x} does not hold it on their cursor",
                                     on_cursor->holder->serial ) );
  }
  else if ( const auto* in_storage = loc.get_if<InStorage>(); in_storage != nullptr )
  {
    if ( in_storage->area == nullptr )
      sweep.note( item, "names a null storage area" );
    else if ( in_storage->area->find_root_item( in_storage->key.get() ) != &item )
      sweep.note( item, fmt::format( "storage area does not file it under \"{}\"",
                                     in_storage->key.get() ) );
  }
  // Preparing, Detached, Intrinsic and Destroyed name no registry, so there is nothing to confirm
  // here. Belonging to none of them is a legal answer, and the backward pass is what catches one
  // of them holding on anyway.
}

void check_zones( Sweep& sweep )
{
  for ( auto& realm : Core::gamestate.Realms )
  {
    for ( const auto& grid : realm->gridarea() )
    {
      for ( auto* item : realm->getzone_grid( grid ).items )
      {
        ++sweep.report.checks;
        sweep.claim( *item, fmt::format( "the world zone at {}", grid ) );

        if ( !item->location().holds<InWorld>() )
          sweep.note( *item, "is in a world zone but does not say it is in the world" );
        else if ( Core::zone_convert( item->pos() ) != grid )
          sweep.note( *item, fmt::format( "is in the zone at {} but its position {} is in {}", grid,
                                          item->pos(), Core::zone_convert( item->pos() ) ) );
      }
    }
  }
}

/// Everything a container holds must say so. A corpse may answer either way: what it renders on a
/// layer is OnCorpse, and what is merely lying in it is InContainer.
void check_container( Sweep& sweep, Core::UContainer& cont )
{
  const bool is_corpse = cont.script_isa( Core::POLCLASS_CORPSE );

  for ( auto* held : cont )
  {
    ++sweep.report.checks;
    sweep.claim( *held, fmt::format( "container {:#x}", cont.serial ) );

    const Location loc = held->location();
    if ( const auto* in_cont = loc.get_if<InContainer>(); in_cont != nullptr )
    {
      if ( in_cont->cont != &cont )
        sweep.note( *held,
                    fmt::format( "is held by container {:#x} but names another", cont.serial ) );
    }
    else if ( const auto* on_corpse = loc.get_if<OnCorpse>(); on_corpse != nullptr )
    {
      if ( !is_corpse )
        sweep.note( *held, fmt::format( "is held by container {:#x}, which is not a corpse",
                                        cont.serial ) );
      else if ( on_corpse->corpse != &cont )
        sweep.note( *held,
                    fmt::format( "is held by corpse {:#x} but names another", cont.serial ) );
    }
    else
    {
      sweep.note( *held, fmt::format( "is held by container {:#x} but says it is elsewhere",
                                      cont.serial ) );
    }
  }

  // A corpse renders at most one item per layer, and the view enforces that by indexing on the
  // layer -- so a second claimant does not conflict, it silently displaces the first and vanishes
  // from the packet. relocate refuses this on insert; asking again here is what covers the loader
  // and anything that has ever written a location without going through it.
  if ( is_corpse )
  {
    std::array<const Item*, Core::HIGHEST_LAYER + 1> claimed{};
    for ( const auto& item : cont )
    {
      if ( item == nullptr )
        continue;
      const Location loc = item->location();
      const auto* on_corpse = loc.get_if<OnCorpse>();
      if ( on_corpse == nullptr || !Items::valid_equip_layer( on_corpse->layer ) )
        continue;

      ++sweep.report.checks;
      if ( const Item* first = claimed[on_corpse->layer]; first != nullptr )
        sweep.note( *item, fmt::format( "claims layer {} of corpse {:#x}, which {:#x} already "
                                        "claims",
                                        on_corpse->layer, cont.serial, first->serial ) );
      else
        claimed[on_corpse->layer] = item;
    }
  }
}

/// A character's worn items and cursor are reached through the character: a WornItemsContainer
/// has no serial of its own, so it is not in the objecthash to be found any other way.
void check_character( Sweep& sweep, Mobile::Character& chr )
{
  for ( unsigned layer = Core::LOWEST_LAYER; layer <= Core::HIGHEST_LAYER; ++layer )
  {
    Item* worn = chr.worn_items()->GetItemOnLayer( layer );
    if ( worn == nullptr )
      continue;

    ++sweep.report.checks;
    sweep.claim( *worn, fmt::format( "character {:#x}, layer {}", chr.serial, layer ) );

    const Location loc = worn->location();
    const auto* equipped = loc.get_if<Equipped>();
    if ( equipped == nullptr || equipped->chr != &chr || equipped->layer != layer )
      sweep.note( *worn, fmt::format( "is worn on layer {} of character {:#x} but does not say so",
                                      layer, chr.serial ) );
  }

  if ( chr.has_gotten_item() )
  {
    if ( Item* held = chr.gotten_item().item(); held != nullptr )
    {
      ++sweep.report.checks;
      sweep.claim( *held, fmt::format( "the cursor of {:#x}", chr.serial ) );

      const Location loc = held->location();
      const auto* on_cursor = loc.get_if<OnCursor>();
      if ( on_cursor == nullptr || on_cursor->holder != &chr )
        sweep.note( *held,
                    fmt::format( "is on the cursor of {:#x} but does not say so", chr.serial ) );
    }
  }
}

void check_storage( Sweep& sweep )
{
  Core::gamestate.storage.for_each_area(
      [&sweep]( Core::StorageArea& area )
      {
        area.for_each_root_item(
            [&sweep, &area]( const std::string& key, Item* item )
            {
              if ( item == nullptr )
                return;
              ++sweep.report.checks;
              sweep.claim( *item, fmt::format( "storage key \"{}\"", key ) );

              const Location loc = item->location();
              const auto* in_storage = loc.get_if<InStorage>();
              if ( in_storage == nullptr || in_storage->area != &area )
                sweep.note( *item, fmt::format( "is a storage root under \"{}\" but does not say "
                                                "so",
                                                key ) );
              else if ( in_storage->key.get() != key )
                sweep.note( *item, fmt::format( "is filed under \"{}\" but carries the key \"{}\"",
                                                key, in_storage->key.get() ) );
            } );
      } );
}

/**
 * A house component is a world item the house also lists, so it has two owners to agree with.
 *
 * Components are deliberately not removed from the list when the core destroys them, so an orphan
 * here is expected and not a finding.
 */
void check_houses( Sweep& sweep )
{
  for ( auto& realm : Core::gamestate.Realms )
  {
    for ( const auto& grid : realm->gridarea() )
    {
      for ( auto* multi : realm->getzone_grid( grid ).multis )
      {
        Multi::UHouse* house = multi->as_house();
        if ( house == nullptr )
          continue;

        house->for_each_component(
            [&sweep, house]( Item& component )
            {
              ++sweep.report.checks;
              if ( component.house() != house )
                sweep.note(
                    component,
                    fmt::format( "is a component of house {:#x} but points at {:#x}", house->serial,
                                 component.house() != nullptr ? component.house()->serial : 0 ) );
              if ( !component.location().holds<InWorld>() )
                sweep.note( component, fmt::format( "is a component of house {:#x} but is not in "
                                                    "the world",
                                                    house->serial ) );
            } );
      }
    }
  }
}
}  // namespace

IntegrityReport check_item_integrity()
{
  Sweep sweep;

  // Forward: every item in the objecthash agrees with the registry it names. Intrinsic equipment
  // is in here too and belongs to no registry by design, and so is anything destroyed but not yet
  // reaped -- both are legal "in nothing" answers, which is why this asks the location first
  // rather than assuming every item has a home.
  for ( const auto& entry : Core::objStorageManager.objecthash )
  {
    Core::UObject* obj = entry.second.get();
    // A multi is an Item by inheritance but lives in the zone's multi list, which Location does
    // not model; it has no location of its own to check.
    if ( obj->ismobile() || obj->ismulti() )
      continue;

    Item& item = static_cast<Item&>( *obj );
    ++sweep.report.checks;
    check_forward( sweep, item );
  }

  // Backward: every registry entry points back. This is the half that finds an owner which forgot
  // to unlink, because the item itself has nothing wrong with it to report.
  check_zones( sweep );
  check_storage( sweep );
  check_houses( sweep );

  for ( const auto& entry : Core::objStorageManager.objecthash )
  {
    Core::UObject* obj = entry.second.get();
    // A destroyed owner is not a registry any more, it is something waiting for Reap() with its
    // last contents still hanging off it. Only a *live* owner holding a destroyed item is a
    // finding, and that is still caught below.
    if ( obj->orphan() )
      continue;

    if ( obj->ismobile() )
      check_character( sweep, static_cast<Mobile::Character&>( *obj ) );
    else if ( obj->isa( Core::UOBJ_CLASS::CLASS_CONTAINER ) )
      check_container( sweep, static_cast<Core::UContainer&>( *obj ) );
  }

  return sweep.report;
}
}  // namespace Pol::Items
