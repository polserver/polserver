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
#include "../globals/uvars.h"
#include "../item/item.h"
#include "../polclock.h"
#include "../realms/realm.h"
#include "../realms/realms.h"
#include "../uworld.h"
#include "testenv.h"

namespace Pol::Testing
{
void decay_test()
{
  using namespace std::chrono_literals;
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

  // decay thread doesnt run, recreate gamestate class
  // to be able to test realm add/delete the gamestate instance needs to be used
  Core::gamestate.decay = Core::Decay();
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
}  // namespace Pol::Testing
