/** @file
 *
 * @par History
 */

#include <algorithm>
#include <cstddef>
#include <vector>

#include "clib/logfacility.h"
#include "clib/rawtypes.h"
#include "pol/globals/uvars.h"
#include "pol/item/item.h"
#include "pol/realms/realm.h"
#include "pol/testing/testenv.h"
#include "pol/uworld.h"

namespace Pol::Testing
{
namespace
{
size_t occurrences( const Realms::Realm* realm, const Core::Pos2d& p, const Items::Item* item )
{
  const auto& items = realm->getzone( p ).items;
  return static_cast<size_t>( std::count( items.begin(), items.end(), item ) );
}

std::vector<u32> serials_in_zone( const Realms::Realm* realm, const Core::Pos2d& p )
{
  std::vector<u32> serials;
  for ( const auto* item : realm->getzone( p ).items )
    serials.push_back( item->serial );
  return serials;
}

// These go through relocate() even though the zone primitives are what is under test: relocate
// calls them, so they are still exercised, and an item left in a zone without a matching location
// is a landmine for anything that walks the zone afterwards -- the decay pass, for one.
Items::Item* item_in_world( const Core::Pos4d& p )
{
  auto* item = Items::Item::create( 0x0eed );
  item->setposition( p );
  (void)Items::relocate( *item, Items::InWorld{} );
  return item;
}

void discard( Items::Item* item )
{
  (void)Items::relocate( *item, Items::Destroyed{} );
}
}  // namespace

// World-zone bookkeeping has no direct coverage today. The order checks matter beyond tidiness:
// savedata.cpp writes items.txt by walking these vectors, so their sequence is part of the save
// file.
void zone_bookkeeping_test()
{
  auto* realm = Core::gamestate.Realms[0];
  auto* other_realm = Core::gamestate.Realms[1];
  const Core::Pos2d base = realm->area().nw();
  const Core::Pos4d zone_a( base + Core::Vec2d( 64, 64 ), 0, realm );
  const Core::Pos4d zone_b( base + Core::Vec2d( 192, 192 ), 0, realm );
  const Core::Pos4d zone_a2( base + Core::Vec2d( 65, 65 ), 0, realm );

  // add / remove keep the zone entry and the realm counter in step
  {
    const auto count_before = realm->toplevel_item_count();
    auto* item = item_in_world( zone_a );

    UnitTest( [&]() { return occurrences( realm, zone_a.xy(), item ); }, size_t( 1 ),
              "add registers exactly one zone entry" );
    UnitTest( [&]() { return realm->toplevel_item_count(); }, count_before + 1,
              "add bumps the toplevel counter" );

    (void)Items::relocate( *item, Items::Detached{} );

    UnitTest( [&]() { return occurrences( realm, zone_a.xy(), item ); }, size_t( 0 ),
              "remove clears the zone entry" );
    UnitTest( [&]() { return realm->toplevel_item_count(); }, count_before,
              "remove restores the toplevel counter" );

    item->destroy();
  }

  // moving between zones of the same realm relocates without touching the counter
  {
    auto* item = item_in_world( zone_a );
    const auto count_before = realm->toplevel_item_count();
    const Core::Pos4d oldpos = item->pos();

    item->setposition( zone_b );
    Core::MoveItemWorldPosition( oldpos, item );

    UnitTest( [&]() { return occurrences( realm, zone_a.xy(), item ); }, size_t( 0 ),
              "move clears the old zone" );
    UnitTest( [&]() { return occurrences( realm, zone_b.xy(), item ); }, size_t( 1 ),
              "move registers in the new zone" );
    UnitTest( [&]() { return realm->toplevel_item_count(); }, count_before,
              "move within a realm leaves the toplevel counter alone" );

    discard( item );
  }

  // moving within one zone must not disturb the collection at all
  {
    auto* item = item_in_world( zone_a );
    const auto before = serials_in_zone( realm, zone_a.xy() );
    const Core::Pos4d oldpos = item->pos();

    item->setposition( zone_a2 );
    Core::MoveItemWorldPosition( oldpos, item );

    UnitTest( [&]() { return serials_in_zone( realm, zone_a.xy() ) == before; }, true,
              "move inside one zone leaves the collection untouched" );

    discard( item );
  }

  // crossing realms hands the item over between the two realm counters
  {
    auto* item = item_in_world( zone_a );
    const auto count_before = realm->toplevel_item_count();
    const auto other_before = other_realm->toplevel_item_count();
    const Core::Pos4d oldpos = item->pos();
    const Core::Pos4d elsewhere( other_realm->area().nw() + Core::Vec2d( 64, 64 ), 0, other_realm );

    item->setposition( elsewhere );
    Core::MoveItemWorldPosition( oldpos, item );

    UnitTest( [&]() { return occurrences( realm, zone_a.xy(), item ); }, size_t( 0 ),
              "realm change clears the old realm's zone" );
    UnitTest( [&]() { return occurrences( other_realm, elsewhere.xy(), item ); }, size_t( 1 ),
              "realm change registers in the new realm's zone" );
    UnitTest( [&]() { return realm->toplevel_item_count(); }, count_before - 1,
              "realm change decrements the old realm counter" );
    UnitTest( [&]() { return other_realm->toplevel_item_count(); }, other_before + 1,
              "realm change increments the new realm counter" );

    discard( item );
  }

  // Sequence within a zone is part of the save file: erase must not reorder the survivors, and
  // an add must append.
  {
    std::vector<Items::Item*> items;
    for ( int i = 0; i < 5; ++i )
      items.push_back( item_in_world( zone_a ) );

    auto expected = serials_in_zone( realm, zone_a.xy() );
    auto* middle = items[2];
    expected.erase( std::find( expected.begin(), expected.end(), middle->serial ) );

    (void)Items::relocate( *middle, Items::Detached{} );
    UnitTest( [&]() { return serials_in_zone( realm, zone_a.xy() ) == expected; }, true,
              "erasing from the middle preserves the order of the survivors" );

    auto* appended = item_in_world( zone_a );
    expected.push_back( appended->serial );
    UnitTest( [&]() { return serials_in_zone( realm, zone_a.xy() ) == expected; }, true,
              "add appends to the end of the zone collection" );

    middle->destroy();
    discard( appended );
    for ( auto* item : items )
    {
      if ( item != middle )
        discard( item );
    }
  }
}
}  // namespace Pol::Testing
