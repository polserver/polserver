/** @file
 *
 * @par History
 */

#include <algorithm>
#include <array>
#include <cstring>
#include <string>

#include "../../clib/logfacility.h"
#include "../../clib/rawtypes.h"
#include "../../plib/systemstate.h"
#include "../globals/uvars.h"
#include "../item/item.h"
#include "../polclock.h"
#include "../realms/realm.h"
#include "../realms/realms.h"
#include "../reftypes.h"
#include "../ufunc.h"
#include "../uworld.h"
#include "testenv.h"

namespace Pol::Testing
{
using namespace std::chrono_literals;
void decay_test()
{
  Plib::systemstate.config.decaytask = false;
  auto createitem = []( Core::Pos4d p, u32 decay )
  {
    auto item = Items::Item::create( 0x0eed );
    item->setposition( p );
    Core::add_item_to_world( item );
    item->set_decay_after( decay );
  };
  auto decay_full_realm_loop = []( Core::Decay& d )
  {
    for ( const auto& p : d.area )
    {
      (void)p;
      d.step();
    }
  };
  INFO_PRINTLN( "    create items" );
  auto* firstrealm = Core::gamestate.Realms[0];
  auto* secondrealm = Core::gamestate.Realms[1];

  // create 3 items, two should decay
  createitem( { 0, 0, 0, firstrealm }, 1 );
  createitem( { 0, 0, 0, firstrealm }, 60 );
  createitem( { firstrealm->area().se() - Core::Vec2d( 1, 1 ), 0, firstrealm }, 1 );
  if ( firstrealm->toplevel_item_count() != 3 )
  {
    INFO_PRINTLN( "first realm toplevelcount 3!={}", firstrealm->toplevel_item_count() );
    UnitTest::inc_failures();
    return;
  }

  // on the second realm one item should also decay
  createitem( { 0, 0, 0, secondrealm }, 1 );
  if ( secondrealm->toplevel_item_count() != 1 )
  {
    INFO_PRINTLN( "second realm toplevelcount 1!={}", secondrealm->toplevel_item_count() );
    UnitTest::inc_failures();
    return;
  }

  // time machine
  Core::shift_clock_for_unittest( 2s );

  // decay thread doesnt run in test environment
  // to be able to test realm add/delete the gamestate instance needs to be used
  auto& d = Core::gamestate.decay;
  d.calculate_sleeptime();

  // first step should directly destroy on item
  INFO_PRINTLN( "    first zone sweep" );
  d.step();
  if ( firstrealm->toplevel_item_count() != 2 )
  {
    INFO_PRINTLN( "first decay step did not destroy item toplevelcount 2!={}",
                  firstrealm->toplevel_item_count() );
    UnitTest::inc_failures();
    return;
  }
  // since we did already one step, this loop will also switch realm
  INFO_PRINTLN( "    full sweep" );
  decay_full_realm_loop( d );
  if ( firstrealm->toplevel_item_count() != 1 )
  {
    INFO_PRINTLN( "toplevelcount after complete loop 1!={}", firstrealm->toplevel_item_count() );
    UnitTest::inc_failures();
    return;
  }
  if ( d.realm_index != 1 )
  {
    INFO_PRINTLN( "active realm didnt switch 1!={}", d.realm_index );
    UnitTest::inc_failures();
    return;
  }
  if ( secondrealm->toplevel_item_count() != 0 )
  {
    INFO_PRINTLN( "second realm toplevelcount 0!={}", secondrealm->toplevel_item_count() );
    UnitTest::inc_failures();
    return;
  }
  // loop until active realm should be again 0
  INFO_PRINTLN( "    rollover" );
  decay_full_realm_loop( d );
  if ( d.realm_index != 0 )
  {
    INFO_PRINTLN( "active realm didnt roll over 0!={}", d.realm_index );
    UnitTest::inc_failures();
    return;
  }

  // test realm add/delete
  INFO_PRINTLN( "    prepare shadow realms" );

  Core::add_realm( "firstshadow", firstrealm );
  Core::add_realm( "secondshadow", firstrealm );
  auto* firstshadow = Core::gamestate.Realms[2];
  auto* secondshadow = Core::gamestate.Realms[3];
  // last shadow realm one item should decay
  createitem( { 0, 0, 0, secondshadow }, 1 );
  if ( secondshadow->toplevel_item_count() != 1 )
  {
    INFO_PRINTLN( "second shadow toplevelcount 1!={}", secondshadow->toplevel_item_count() );
    UnitTest::inc_failures();
    return;
  }
  // time machine
  Core::shift_clock_for_unittest( 2s );
  // current index is 0 so loop 2 times to get to the first shadow realm
  decay_full_realm_loop( d );
  decay_full_realm_loop( d );
  if ( d.realm_index != 2 )
  {
    INFO_PRINTLN( "active realm isnt first shadow 2!={}", d.realm_index );
    UnitTest::inc_failures();
    return;
  }

  INFO_PRINTLN( "    remove active realm" );
  Core::remove_realm( firstshadow->name() );
  d.step();
  if ( d.realm_index != 2 )
  {
    INFO_PRINTLN( "active realm isnt second shadow 2!={}", d.realm_index );
    UnitTest::inc_failures();
    return;
  }
  if ( secondshadow->toplevel_item_count() != 0 )
  {
    INFO_PRINTLN( "second shadow toplevelcount 0!={}", secondshadow->toplevel_item_count() );
    UnitTest::inc_failures();
    return;
  }
  UnitTest::inc_successes();
}

void decaytask_test()
{
  {
    // wipe realms
    for ( auto& realm : Core::gamestate.Realms )
    {
      Core::WorldIterator<Core::ItemFilter>::InBox(
          realm->area(), realm, [&]( Items::Item* item ) { destroy_item( item ); } );
    }
  }
  auto& decay = Core::gamestate.world_decay;
  Plib::systemstate.config.decaytask = true;
  auto now = Core::read_gameclock();
  auto createitem = [&]( Core::Pos4d p, u32 decaytime ) -> Items::Item*
  {
    auto item = Items::Item::create( 0x0eed );
    item->setposition( p );
    Core::add_item_to_world( item );
    if ( !item->has_decay_task() )
    {
      INFO_PRINTLN( "decay task not active for item" );
      UnitTest::inc_failures();
      return nullptr;
    }
    if ( decay.getDecayTime( item ) <= now )
    {
      INFO_PRINTLN( "decay time {}<{}", decay.getDecayTime( item ), now );
      UnitTest::inc_failures();
      return nullptr;
    }
    decay.addObject( item, decaytime );
    return item;
  };
  INFO_PRINTLN( "    create items" );
  auto* firstrealm = Core::gamestate.Realms[0];

  auto i1 = Core::ItemRef( createitem( { 0, 0, 0, firstrealm }, 10 ) );
  auto i2 = Core::ItemRef( createitem( { 0, 0, 0, firstrealm }, 60 ) );
  if ( !i1 || !i2 )
    return;
  if ( firstrealm->toplevel_item_count() != 2 )
  {
    INFO_PRINTLN( "first realm toplevelcount 2!={}", firstrealm->toplevel_item_count() );
    UnitTest::inc_failures();
    return;
  }
  INFO_PRINTLN( "Gameclock {}", Core::read_gameclock() );
  INFO_PRINTLN( "i1 {} {}", i1->has_decay_task(), decay.getDecayTime( i1.get() ) );
  INFO_PRINTLN( "i2 {} {}", i2->has_decay_task(), decay.getDecayTime( i2.get() ) );
  decay.decayTask();  // should not destroy items
  if ( firstrealm->toplevel_item_count() != 2 )
  {
    INFO_PRINTLN( "first realm toplevelcount 2!={}", firstrealm->toplevel_item_count() );
    UnitTest::inc_failures();
    return;
  }
  // time machine to first item
  Core::shift_clock_for_unittest( 10s );

  decay.decayTask();
  if ( !i1->orphan() || i2->orphan() )
  {
    INFO_PRINTLN( "first destroyed: {}, second not: {}", i1->orphan(), !i2->orphan() );
    UnitTest::inc_failures();
    return;
  }
  if ( decay.activeObjects() != 1 )
  {
    INFO_PRINTLN( "decay activeObjects 1!={}", decay.activeObjects() );
    UnitTest::inc_failures();
    return;
  }


  UnitTest::inc_successes();
}
}  // namespace Pol::Testing
