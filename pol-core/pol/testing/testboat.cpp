/** @file
 *
 * @par History
 */

#include <fstream>
#include <string>
#include <vector>

#include "bscript/bobject.h"
#include "clib/cfgelem.h"
#include "clib/cfgfile.h"
#include "clib/rawtypes.h"
#include "clib/streamsaver.h"
#include "pol/core.h"
#include "pol/globals/settings.h"
#include "pol/globals/uvars.h"
#include "pol/item/item.h"
#include "pol/item/itemdesc.h"
#include "pol/item/location.h"
#include "pol/multi/boat.h"
#include "pol/multi/multi.h"
#include "pol/realms/realm.h"
#include "pol/testing/testenv.h"
#include "pol/uworld.h"

// A boat moves its components by pairing them, in order, with the component lines of its shape in
// boats.cfg. A save can still list a component twice or out of order - one that was also saved as a
// traveller does both - and loading that as it stands sends every component after it to another
// one's offset on every step. These load such saves and check where each component ends up once
// the boat moves.

namespace Pol::Testing
{
namespace
{
constexpr u32 BOAT_OBJTYPE = 0x11000;  // a small boat, multiid 0, per the test shard's itemdesc.cfg

// Where the test shard's boats.cfg puts each component of multiid 0, relative to the boat.
const Core::Vec3d TILLERMAN_DELTA( 1, 4, 0 );
const Core::Vec3d PORTPLANK_DELTA( -2, 0, 0 );
const Core::Vec3d STARBOARDPLANK_DELTA( 2, 0, 0 );
const Core::Vec3d HOLD_DELTA( 0, -4, 0 );

const std::string testdir = unittest_path( "boat" );

struct Parts
{
  Items::Item* tillerman;
  Items::Item* portplank;
  Items::Item* starboardplank;
  Items::Item* hold;
};

// The components as a save would find them: in the world, all standing on the same spot.
Parts parts_at( const Core::Pos4d& pos )
{
  auto make = [&]( u32 objtype )
  {
    auto* item = Items::Item::create( objtype );
    (void)Items::place_at( *item, pos );
    return item;
  };
  const auto& extobj = Core::settingsManager.extobj;
  return Parts{ make( extobj.tillerman ), make( extobj.port_plank ), make( extobj.starboard_plank ),
                make( extobj.hold ) };
}

std::string serial( const Items::Item* item )
{
  return fmt::format( "{:#x}", item->serial );
}

// Loads a boat at pos from a Multi element holding the given property lines, the way the world
// load does.
Multi::UBoat* load_boat( const Core::Pos4d& pos, const std::vector<std::string>& lines )
{
  std::string body =
      fmt::format( "Multi\n{{\n  ObjType {:#x}\n  Graphic 0x4000\n  MultiID 0\n", BOAT_OBJTYPE );
  body += fmt::format( "  X {}\n  Y {}\n  Z {}\n  Realm {}\n", pos.x(), pos.y(), pos.z(),
                       pos.realm()->name() );
  for ( const auto& line : lines )
    body += "  " + line + "\n";
  body += "}\n";

  const std::string path = testdir + "/multis.txt";
  {
    std::ofstream ofs( path );
    ofs << body;
  }
  Clib::ConfigFile cf( path.c_str(), "Multi" );
  Clib::ConfigElem elem;
  cf.read( elem );

  auto* multi = Multi::UMulti::create( Items::find_itemdesc( BOAT_OBJTYPE ) );
  multi->readProperties( elem );
  Core::add_multi_to_world( multi );
  return multi->as_boat();
}

size_t saved_component_count( const Multi::UBoat* boat )
{
  Clib::StreamWriter sw;
  boat->printProperties( sw );
  const auto view = sw.buffer();
  const std::string text( view.data(), view.size() );
  size_t count = 0;
  for ( size_t at = text.find( "\tComponent\t" ); at != std::string::npos;
        at = text.find( "\tComponent\t", at + 1 ) )
    ++count;
  return count;
}

void destroy( Multi::UBoat* boat )
{
  Bscript::BObject res( Multi::destroy_boat( boat ) );
}

// Steps the boat once, which puts every component it has a slot for at that slot's offset.
void step( Multi::UBoat* boat )
{
  (void)boat->move_to( boat->pos() + Core::Vec2d( 1, 0 ), Core::MOVEITEM_FORCELOCATION );
}

bool at( const Multi::UBoat* boat, const Items::Item* item, const Core::Vec3d& delta )
{
  return item->pos() == boat->pos() + delta;
}
}  // namespace

void boat_load_test()
{
  UnitTestDir dir( testdir );
  auto* realm = Core::gamestate.Realms[0];
  const Core::Pos4d pos( 10, 50, -4, realm );

  // the tillerman listed twice, as a component
  {
    auto p = parts_at( pos );
    auto* boat = load_boat(
        pos, { "Component " + serial( p.tillerman ), "Component " + serial( p.tillerman ),
               "Component " + serial( p.portplank ), "Component " + serial( p.starboardplank ),
               "Component " + serial( p.hold ) } );
    step( boat );

    UnitTest( [&]() { return saved_component_count( boat ); }, size_t( 4 ),
              "a component listed twice is kept once" );
    UnitTest( [&]() { return at( boat, p.tillerman, TILLERMAN_DELTA ); }, true,
              "a component listed twice moves to its own offset" );
    UnitTest( [&]() { return at( boat, p.portplank, PORTPLANK_DELTA ); }, true,
              "the port plank after a component listed twice keeps its offset" );
    UnitTest( [&]() { return at( boat, p.starboardplank, STARBOARDPLANK_DELTA ); }, true,
              "the starboard plank after a component listed twice keeps its offset" );
    UnitTest( [&]() { return at( boat, p.hold, HOLD_DELTA ); }, true,
              "the last component after a component listed twice still moves with the boat" );

    destroy( boat );
  }

  // the port plank listed as a traveller as well, which puts it ahead of the tillerman
  {
    auto p = parts_at( pos );
    auto* boat = load_boat(
        pos, { "Traveller " + serial( p.portplank ), "Component " + serial( p.tillerman ),
               "Component " + serial( p.portplank ), "Component " + serial( p.starboardplank ),
               "Component " + serial( p.hold ) } );
    step( boat );

    UnitTest( [&]() { return saved_component_count( boat ); }, size_t( 4 ),
              "a component also listed as a traveller is kept once" );
    UnitTest( [&]() { return at( boat, p.tillerman, TILLERMAN_DELTA ); }, true,
              "a component read out of order moves to its own offset" );
    UnitTest( [&]() { return at( boat, p.portplank, PORTPLANK_DELTA ); }, true,
              "a component also listed as a traveller moves to its own offset" );
    UnitTest( [&]() { return at( boat, p.hold, HOLD_DELTA ); }, true,
              "the components after one read out of order keep their offsets" );

    destroy( boat );
  }

  // no port plank at all
  {
    auto p = parts_at( pos );
    p.portplank->destroy();
    auto* boat = load_boat(
        pos, { "Component " + serial( p.tillerman ), "Component " + serial( p.starboardplank ),
               "Component " + serial( p.hold ) } );
    step( boat );

    UnitTest( [&]() { return saved_component_count( boat ); }, size_t( 3 ),
              "a missing component is not made up" );
    UnitTest( [&]() { return at( boat, p.starboardplank, STARBOARDPLANK_DELTA ); }, true,
              "the component after a missing one does not take its offset" );
    UnitTest( [&]() { return at( boat, p.hold, HOLD_DELTA ); }, true,
              "the last component after a missing one keeps its offset" );

    destroy( boat );
  }

  // a well-formed save, the case every shard loads
  {
    auto p = parts_at( pos );
    auto* boat = load_boat(
        pos, { "Component " + serial( p.tillerman ), "Component " + serial( p.portplank ),
               "Component " + serial( p.starboardplank ), "Component " + serial( p.hold ) } );
    step( boat );

    UnitTest(
        [&]()
        {
          return at( boat, p.tillerman, TILLERMAN_DELTA ) &&
                 at( boat, p.portplank, PORTPLANK_DELTA ) &&
                 at( boat, p.starboardplank, STARBOARDPLANK_DELTA ) &&
                 at( boat, p.hold, HOLD_DELTA );
        },
        true, "a well-formed save moves every component to its offset" );

    destroy( boat );
  }
}
}  // namespace Pol::Testing
