/** @file
 *
 * @par History
 */

#include <algorithm>
#include <cstddef>
#include <string>

#include "clib/logfacility.h"
#include "clib/rawtypes.h"
#include "pol/containr.h"
#include "pol/globals/settings.h"
#include "pol/globals/uvars.h"
#include "pol/item/item.h"
#include "pol/item/location.h"
#include "pol/item/weapon.h"
#include "pol/layers.h"
#include "pol/realms/realm.h"
#include "pol/storage.h"
#include "pol/testing/testenv.h"
#include "pol/uworld.h"

namespace Pol::Testing
{
namespace
{
// The whole point of keeping Location opaque is that this stays affordable: it replaces a
// UContainer* plus two bytes, so anything much past 24 costs real memory on a large world.
static_assert( sizeof( Items::Location ) <= 24, "Location has outgrown its budget" );

constexpr u32 CONTAINER_OBJTYPE = 0xe75;  // a backpack, per the test shard's itemdesc.cfg
constexpr u32 ITEM_OBJTYPE = 0x0eed;      // gold

size_t occurrences( const Realms::Realm* realm, const Core::Pos2d& p, const Items::Item* item )
{
  const auto& items = realm->getzone( p ).items;
  return static_cast<size_t>( std::count( items.begin(), items.end(), item ) );
}

Items::Item* item_in_world( u32 objtype, const Core::Pos4d& p )
{
  auto* item = Items::Item::create( objtype );
  item->setposition( p );
  (void)Items::relocate( *item, Items::InWorld{} );
  return item;
}

Core::UContainer* container_in_world( const Core::Pos4d& p )
{
  return static_cast<Core::UContainer*>( item_in_world( CONTAINER_OBJTYPE, p ) );
}

/// Everything a rejected relocate() has to leave alone.
struct Snapshot
{
  std::string loc;
  Core::UContainer* container;
  u8 layer;
  u8 slot;
  size_t zone_entries;

  explicit Snapshot( const Items::Item* item )
      : loc( item->location().describe() ),
        container( item->container() ),
        layer( item->location().layer() ),
        slot( item->slot_index() ),
        zone_entries( occurrences( item->realm(), item->pos2d(), item ) )
  {
  }

  bool operator==( const Snapshot& other ) const
  {
    return loc == other.loc && container == other.container && layer == other.layer &&
           slot == other.slot && zone_entries == other.zone_entries;
  }
};
}  // namespace

// Location and relocate() have no shard-level coverage that can distinguish "the item ended up in
// the right place" from "the item ended up in the right place and every registry agrees", so the
// state machine is proven here instead.
//
// Equipped and OnCorpse transitions need a live Character to own the worn-items container, which
// this harness does not build; only their validation is covered below.
void location_test()
{
  auto* realm = Core::gamestate.Realms[0];
  const Core::Pos4d spot( realm->area().nw() + Core::Vec2d( 32, 32 ), 0, realm );
  const Core::Pos4d spot2( realm->area().nw() + Core::Vec2d( 33, 33 ), 0, realm );

  // a fresh item belongs to nothing
  {
    auto* item = Items::Item::create( ITEM_OBJTYPE );
    UnitTest( [&]() { return item->location().holds<Items::Detached>(); }, true,
              "a newly created item is detached" );
    item->destroy();
  }

  // the world registry and the location move together
  {
    auto* item = item_in_world( ITEM_OBJTYPE, spot );
    UnitTest( [&]() { return item->location().holds<Items::InWorld>(); }, true,
              "entering the world sets InWorld" );
    UnitTest( [&]() { return occurrences( realm, spot.xy(), item ); }, size_t( 1 ),
              "and registers exactly one zone entry" );

    UnitTest( [&]() { return relocate( *item, Items::Detached{} ); }, true,
              "leaving the world succeeds" );
    UnitTest( [&]() { return item->location().holds<Items::Detached>(); }, true,
              "leaving the world sets Detached" );
    UnitTest( [&]() { return occurrences( realm, spot.xy(), item ); }, size_t( 0 ),
              "and clears the zone entry" );
    item->destroy();
  }

  // Destroying a world item has to unlink it from its zone. Decay steps its index back on exactly
  // that promise (decay.cpp), so an item that stays in the vector is not a leak, it is a loop that
  // logs once per turn -- which is how this went from a stale field to an unusable machine.
  {
    auto* item = item_in_world( ITEM_OBJTYPE, spot );
    item->destroy();
    UnitTest( [&]() { return occurrences( realm, spot.xy(), item ); }, size_t( 0 ),
              "destroying a world item clears its zone entry" );
  }

  // ground -> container -> ground, driven entirely by relocate()
  {
    auto* cont = container_in_world( spot );
    auto* item = item_in_world( ITEM_OBJTYPE, spot2 );

    UnitTest( [&]()
              { return relocate( *item, Items::InContainer{ cont, Core::Pos2d( 1, 1 ), 0 } ); },
              true, "relocate from the world into a container succeeds" );
    UnitTest( [&]() { return item->location().holds<Items::InContainer>(); }, true,
              "the item is now InContainer" );
    UnitTest( [&]() { return item->location().container() == cont; }, true,
              "the location reports the container it is in" );
    UnitTest( [&]() { return item->container() == cont; }, true, "and so does Item::container()" );
    UnitTest( [&]() { return occurrences( realm, spot2.xy(), item ); }, size_t( 0 ),
              "the item left its world zone" );
    UnitTest( [&]() { return cont->count(); }, 1u, "the container holds it" );

    UnitTest( [&]() { return relocate( *item, Items::InWorld{} ); }, true,
              "relocate back to the world succeeds" );
    UnitTest( [&]() { return cont->count(); }, 0u, "the container released it" );
    UnitTest( [&]() { return occurrences( realm, item->pos2d(), item ); }, size_t( 1 ),
              "the item is back in exactly one world zone" );
    UnitTest( [&]() { return item->container() == nullptr; }, true,
              "and it names no container any more" );

    item->destroy();
    cont->destroy();
  }

  // destroying unlinks first, which is what the container-move paths get wrong: the container's
  // entry is a plain pointer, so an item left in it is freed by Reap with the entry still there
  {
    auto* cont = container_in_world( spot );
    auto* item = Items::Item::create( ITEM_OBJTYPE );
    (void)relocate( *item, Items::InContainer{ cont, Core::Pos2d( 1, 1 ), 0 } );

    item->destroy();
    UnitTest( [&]() { return cont->count(); }, 0u,
              "the container no longer references the destroyed item" );
    UnitTest( [&]() { return item->location().holds<Items::Destroyed>(); }, true,
              "the location reports Destroyed" );
    UnitTest( [&]() { return relocate( *item, Items::InWorld{} ); }, false,
              "Destroyed is terminal" );
    UnitTest( [&]() { return relocate( *cont, Items::Destroyed{} ); }, false,
              "and is not somewhere an item can be moved to" );

    item->destroy();  // twice, deliberately: an item already gone has nothing left to leave
    cont->destroy();
  }

  // storage roots carry their key, because the area alone cannot find them again
  {
    Core::StorageArea area( "unittest" );
    auto* item = item_in_world( ITEM_OBJTYPE, spot );
    const std::string name = item->name();

    UnitTest( [&]() { return relocate( *item, Items::InStorage{ &area, name } ); }, true,
              "relocate into a storage area succeeds" );
    UnitTest( [&]() { return item->location().holds<Items::InStorage>(); }, true,
              "the item is now InStorage" );
    UnitTest( [&]() { return area.find_root_item( name ) == item; }, true,
              "the area can find it by its key" );
    UnitTest( [&]() { return relocate( *item, Items::InStorage{ &area, "not its name" } ); }, false,
              "a storage key that is not the item's name is rejected" );

    UnitTest( [&]() { return relocate( *item, Items::InWorld{} ); }, true,
              "relocate out of storage succeeds" );
    UnitTest( [&]() { return area.find_root_item( name ) == nullptr; }, true,
              "leaving storage unlinks it from the area" );

    item->destroy();
  }

  // rejections, each of which must leave the item exactly as it was
  {
    auto* outer = container_in_world( spot );
    auto* inner = container_in_world( spot2 );
    UnitTest( [&]()
              { return relocate( *inner, Items::InContainer{ outer, Core::Pos2d( 1, 1 ), 0 } ); },
              true, "nesting one container in another succeeds" );

    const Snapshot before( outer );

    UnitTest( [&]()
              { return relocate( *outer, Items::InContainer{ outer, Core::Pos2d( 1, 1 ), 0 } ); },
              false, "an item cannot be put inside itself" );
    UnitTest( [&]()
              { return relocate( *outer, Items::InContainer{ inner, Core::Pos2d( 1, 1 ), 0 } ); },
              false, "a container cannot be put inside its own contents" );
    UnitTest( [&]() { return relocate( *outer, Items::InContainer{ nullptr, Core::Pos2d(), 0 } ); },
              false, "a null container is rejected" );
    UnitTest( [&]() { return relocate( *outer, Items::Equipped{ nullptr, 0 } ); }, false,
              "a null character is rejected" );
    UnitTest( [&]() { return relocate( *outer, Items::OnCorpse{ nullptr, Core::Pos2d(), 0, 0 } ); },
              false, "a null corpse is rejected" );
    UnitTest( [&]() { return relocate( *outer, Items::InStorage{ nullptr, "x" } ); }, false,
              "a null storage area is rejected" );
    // location() answers Preparing ahead of the orphan() test, so an item allowed back here would
    // report Preparing even once destroyed
    UnitTest( [&]() { return relocate( *outer, Items::Preparing{} ); }, false,
              "an item cannot go back to being under construction" );

    UnitTest( [&]() { return Snapshot( outer ) == before; }, true,
              "a rejected relocate leaves the item untouched" );
    UnitTest( [&]() { return inner->container() == outer; }, true,
              "a rejected relocate leaves the other item untouched" );

    // and the no-op case is a success, not a rejection -- for every alternative that carries the
    // data its outcome depends on, which is what makes "already there" mean anything
    UnitTest( [&]() { return relocate( *inner, inner->location() ); }, true,
              "relocating to the location it already has is a no-op success" );

    // InWorld is the exception, because it carries nothing: two positions a world apart compare
    // equal, so the question has to go to place_at() instead of being answered here
    UnitTest( [&]() { return relocate( *outer, outer->location() ); }, false,
              "except in the world, where the location does not say where the item is" );

    inner->destroy();
    outer->destroy();
  }

  // the read-only views keep today's meaning for the states that have no container or layer
  {
    auto* item = item_in_world( ITEM_OBJTYPE, spot );
    UnitTest( [&]() { return item->location().container() == nullptr; }, true,
              "InWorld has no container" );
    UnitTest( [&]() { return item->location().layer(); }, u8( 0 ), "InWorld has no layer" );
    UnitTest( [&]() { return item->location().slot(); }, u8( 0 ), "InWorld has no slot" );
    item->destroy();
  }

  // move_into() claims the slot and inserts as one step, and takes the claim back if the insert is
  // refused. UseContainerSlots is off by default, and with it off slot_index() never writes -- so a
  // restored slot index and one that was never touched are indistinguishable, and the property this
  // block exists for cannot be observed. Hence the toggle.
  {
    auto& use_slots = Core::settingsManager.ssopt.use_slot_index;
    const bool slots_were = use_slots;
    use_slots = true;

    auto* cont = container_in_world( spot );
    auto* item = item_in_world( ITEM_OBJTYPE, spot2 );
    const u8 cont_slot_before = cont->slot_index();

    u8 slot = 5;
    UnitTest( [&]() { return Items::move_into( *item, *cont, Core::Pos2d( 1, 1 ), slot ); }, true,
              "move_into inserts and claims a slot" );
    UnitTest( [&]() { return item->slot_index(); }, u8( 5 ), "the item took the slot it claimed" );
    UnitTest( [&]() { return item->container() == cont; }, true, "and it is in the container" );

    // Rejected by validate -- but only after move_into has already written the slot index, which is
    // the whole reason it has to put it back.
    u8 doomed = 7;
    UnitTest( [&]() { return Items::move_into( *cont, *cont, Core::Pos2d( 1, 1 ), doomed ); },
              false, "move_into refuses to put a container inside itself" );
    UnitTest( [&]() { return cont->slot_index(); }, cont_slot_before,
              "a refused move_into puts the slot index back" );

    use_slots = slots_were;
    item->destroy();
    cont->destroy();
  }

  // Intrinsic equipment: shared, never worn, and the one population that stays serial-less until
  // startup finishes. Nothing else in the suite touches it -- deleting the writes that feed
  // chr.weapon.layer once passed every test.
  {
    const Items::Item* wrestling = Core::gamestate.wrestling_weapon;
    UnitTest( [&]() { return wrestling != nullptr; }, true, "the shard has a wrestling weapon" );
    if ( wrestling != nullptr )
    {
      UnitTest( [&]() { return wrestling->location().holds<Items::Intrinsic>(); }, true,
                "the wrestling weapon is Intrinsic, not Detached or Destroyed" );
      UnitTest( [&]() { return wrestling->location().layer(); }, u8( Core::LAYER_HAND1 ),
                "and reports the hand it stands for, which is what chr.weapon.layer reads" );
      UnitTest( [&]() { return wrestling->location().container() == nullptr; }, true,
                "intrinsic equipment is in no container" );
    }
  }
}
}  // namespace Pol::Testing
