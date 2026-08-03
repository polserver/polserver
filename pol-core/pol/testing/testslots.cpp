/** @file
 *
 * @par History
 */

#include <cstdio>
#include <fstream>
#include <iterator>
#include <string>

#include "clib/rawtypes.h"
#include "clib/streamsaver.h"
#include "pol/containr.h"
#include "pol/globals/settings.h"
#include "pol/globals/uvars.h"
#include "pol/item/item.h"
#include "pol/item/location.h"
#include "pol/loaddata.h"
#include "pol/realms/realm.h"
#include "pol/testing/testenv.h"

namespace Pol::Testing
{
namespace
{
constexpr u32 CONTAINER_OBJTYPE = 0xe75;  // a backpack, per the test shard's itemdesc.cfg
constexpr u32 ITEM_OBJTYPE = 0x0eed;      // gold

/// Turns container slots on for the length of a scope. Nothing in the test shard sets
/// UseContainerSlots, so without this every function below short-circuits to "yes" and proves
/// nothing at all.
struct WithSlots
{
  bool previous;
  WithSlots() : previous( Core::settingsManager.ssopt.use_slot_index )
  {
    Core::settingsManager.ssopt.use_slot_index = true;
  }
  ~WithSlots() { Core::settingsManager.ssopt.use_slot_index = previous; }
};

Items::Item* item_in_world( u32 objtype, const Core::Pos4d& p )
{
  auto* item = Items::Item::create( objtype );
  item->setposition( p );
  (void)Items::relocate( *item, Items::InWorld{} );
  return item;
}

/// Put an item into cont at an explicitly chosen slot, bypassing allocation, so a test can build a
/// deliberate arrangement -- including one no allocator would produce.
Items::Item* at_slot( Core::UContainer* cont, u8 slot, const Core::Pos4d& spot )
{
  auto* item = item_in_world( ITEM_OBJTYPE, spot );
  (void)item->slot_index( slot );
  (void)Items::relocate( *item, Items::InContainer{ cont, Core::Pos2d( 1, 1 ), slot } );
  return item;
}

constexpr auto npos = std::string::npos;

/// What the world file would hold for this item. StreamWriter only writes to a file, so this goes
/// through one -- which is the point: it is the real save path, not a re-implementation of it.
std::string saved_properties_of( const Items::Item* item )
{
  const std::string path = "testslots.tmp";
  {
    Clib::StreamWriter sw( path );
    item->printOn( sw );
    sw.flush_close();
  }
  std::ifstream in( path );
  std::string text( ( std::istreambuf_iterator<char>( in ) ), std::istreambuf_iterator<char>() );
  in.close();
  std::remove( path.c_str() );
  return text;
}
}  // namespace

// Container slots have never had a test. The subsystem is inert at the default
// (UseContainerSlots=0, which is also what the test shard runs), so every one of these functions
// has shipped for years without executing a single meaningful branch -- which is how an allocator
// that hands out occupied slots survived.
void container_slot_test()
{
  WithSlots slots_on;

  auto* realm = Core::gamestate.Realms[0];
  const Core::Pos4d spot( realm->area().nw() + Core::Vec2d( 40, 40 ), 0, realm );

  // The allocator has to look at every item, not just the first one.
  //
  // This is the arrangement the old scan got wrong: it left the inner loop at the first item that
  // did not hold the candidate slot, so with the occupant of slot 1 stored anywhere but first, it
  // reported slot 1 free and handed it out on top of the item already there.
  {
    auto* cont = static_cast<Core::UContainer*>( item_in_world( CONTAINER_OBJTYPE, spot ) );
    auto* third = at_slot( cont, 3, spot );
    auto* first = at_slot( cont, 1, spot );
    auto* second = at_slot( cont, 2, spot );

    UnitTest( [&]() { return cont->is_slot_empty( 1 ); }, false, "slot 1 is occupied" );
    UnitTest( [&]() { return cont->is_slot_empty( 4 ); }, true, "slot 4 is free" );

    u8 found = 1;
    UnitTest( [&]() { return cont->find_empty_slot( found ); }, true, "a free slot exists" );
    UnitTest( [&]() { return found; }, u8( 4 ),
              "the allocator skips every occupied slot, not just the first item's" );

    // The same question through the door callers actually use.
    u8 wanted = 1;
    UnitTest( [&]() { return cont->can_add_to_slot( wanted ); }, true, "can_add_to_slot agrees" );
    UnitTest( [&]() { return wanted; }, u8( 4 ), "and reports the slot it reserved" );

    (void)Items::relocate( *first, Items::Destroyed{} );
    (void)Items::relocate( *second, Items::Destroyed{} );
    (void)Items::relocate( *third, Items::Destroyed{} );
    (void)Items::relocate( *cont, Items::Destroyed{} );
  }

  // A free slot is one nothing sits in, whichever order the container stores its contents.
  {
    auto* cont = static_cast<Core::UContainer*>( item_in_world( CONTAINER_OBJTYPE, spot ) );
    auto* a = at_slot( cont, 2, spot );
    auto* b = at_slot( cont, 1, spot );

    u8 found = 1;
    (void)cont->find_empty_slot( found );
    UnitTest( [&]() { return cont->is_slot_empty( found ); }, true,
              "the slot the allocator hands out is actually empty" );

    (void)Items::relocate( *a, Items::Destroyed{} );
    (void)Items::relocate( *b, Items::Destroyed{} );
    (void)Items::relocate( *cont, Items::Destroyed{} );
  }

  // Asking past the container's ceiling is refused rather than clamped.
  {
    auto* cont = static_cast<Core::UContainer*>( item_in_world( CONTAINER_OBJTYPE, spot ) );
    u8 too_high = static_cast<u8>( cont->max_slots() );
    UnitTest( [&]() { return cont->can_add_to_slot( too_high ); }, true,
              "the last slot is usable -- slots count from one" );

    (void)Items::relocate( *cont, Items::Destroyed{} );
  }

  // A container with no room left says so. can_add_to_slot used to fall through to "yes" when the
  // allocator came up empty, which made every "no free slot" guard in the tree unreachable.
  {
    auto* cont = static_cast<Core::UContainer*>( item_in_world( CONTAINER_OBJTYPE, spot ) );
    cont->max_slots_mod( static_cast<s8>( 2 - cont->max_slots() ) );  // room for exactly two
    UnitTest( [&]() { return cont->max_slots(); }, u8( 2 ), "the container now holds two slots" );

    auto* a = at_slot( cont, 1, spot );
    auto* b = at_slot( cont, 2, spot );

    u8 wanted = 1;
    UnitTest( [&]() { return cont->can_add_to_slot( wanted ); }, false,
              "a container whose slots are all taken has no room for another" );

    (void)Items::relocate( *a, Items::Destroyed{} );
    (void)Items::relocate( *b, Items::Destroyed{} );
    (void)Items::relocate( *cont, Items::Destroyed{} );
  }

  // The slot survives a save. It was never written before, so every arrangement a player made
  // collapsed to slot 0 on the next restart -- while still being sent to the client in 0x25 and
  // 0x3C, which is most of why this subsystem cannot have seen real use.
  {
    auto* cont = static_cast<Core::UContainer*>( item_in_world( CONTAINER_OBJTYPE, spot ) );
    auto* item = at_slot( cont, 6, spot );

    UnitTest( [&]() { return saved_properties_of( item ).find( "SlotIndex\t6" ) != npos; }, true,
              "an item in a slot saves it" );

    // The read side, through the door the loader uses.
    auto* loaded = item_in_world( ITEM_OBJTYPE, spot );
    Core::add_loaded_item( cont, loaded, 0, 9 );
    UnitTest( [&]() { return loaded->slot_index(); }, u8( 9 ),
              "and the loader puts it back in the slot the save recorded" );

    (void)Items::relocate( *item, Items::Destroyed{} );
    (void)Items::relocate( *loaded, Items::Destroyed{} );
    (void)Items::relocate( *cont, Items::Destroyed{} );
  }

  // Nothing new lands in the save file for a shard that does not use slots, which is what keeps
  // the world files byte-identical for everybody else.
  {
    Core::settingsManager.ssopt.use_slot_index = false;
    auto* cont = static_cast<Core::UContainer*>( item_in_world( CONTAINER_OBJTYPE, spot ) );
    auto* item = at_slot( cont, 4, spot );

    UnitTest( [&]() { return saved_properties_of( item ).find( "SlotIndex" ) == npos; }, true,
              "with slots off no SlotIndex is written" );

    (void)Items::relocate( *item, Items::Destroyed{} );
    (void)Items::relocate( *cont, Items::Destroyed{} );
    Core::settingsManager.ssopt.use_slot_index = true;
  }

  // Item::slot_index and UContainer::can_add_to_slot have to agree about the last slot, or
  // move_into fails on a slot the container just said it had reserved.
  {
    auto* item = item_in_world( ITEM_OBJTYPE, spot );
    const u8 last = Core::settingsManager.ssopt.default_max_slots;
    UnitTest( [&]() { return item->slot_index( last ); }, true,
              "an item accepts the last slot the container can hand out" );
    UnitTest( [&]() { return item->slot_index( static_cast<u8>( last + 1 ) ); }, false,
              "and refuses one past it" );

    (void)Items::relocate( *item, Items::Destroyed{} );
  }
}
}  // namespace Pol::Testing
