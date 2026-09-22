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

// A boat moves and turns its components by pairing them, in order, with the component lines of its
// shape in boats.cfg. These load saves whose component list does not line up with that shape - a
// component listed twice, one listed as a traveller as well, one missing - and check where each
// component ends up once the boat moves or turns.

namespace Pol::Testing
{
namespace
{
constexpr u32 BOAT_OBJTYPE = 0x11000;  // a small boat, multiid 0, per the test shard's itemdesc.cfg

// Where the test shard's boats.cfg puts each component, relative to the boat: multiid 0 faces
// north, and multiid 1 is the same boat turned right.
const Core::Vec3d TILLERMAN_DELTA( 1, 4, 0 );
const Core::Vec3d PORTPLANK_DELTA( -2, 0, 0 );
const Core::Vec3d STARBOARDPLANK_DELTA( 2, 0, 0 );
const Core::Vec3d HOLD_DELTA( 0, -4, 0 );
const Core::Vec3d TURNED_TILLERMAN_DELTA( -4, 0, 0 );
const Core::Vec3d TURNED_PORTPLANK_DELTA( 0, -2, 0 );
const Core::Vec3d TURNED_STARBOARDPLANK_DELTA( 0, 2, 0 );
const Core::Vec3d TURNED_HOLD_DELTA( 4, 0, 0 );
// Turning gives each component the graphic of its slot; the itemdesc graphic is 0x1 for all four,
// so a plank counts as retracted.
constexpr u16 TURNED_TILLERMAN_GRAPHIC = 0x3e55;
constexpr u16 TURNED_PORTPLANK_GRAPHIC = 0x3e8a;
constexpr u16 TURNED_STARBOARDPLANK_GRAPHIC = 0x3e85;
constexpr u16 TURNED_HOLD_GRAPHIC = 0x3e65;

// Multiid 0 with a second hold, for the one case that needs two components sharing an objtype: the
// test shard's boats.cfg has one component of each.
const std::string two_hold_shape =
    "Boat\n"
    "{\n"
    "  TillerMan                   0x3e4e  1  4  0\n"
    "  PortGangplankExtended       0x3ed5 -2  0  0\n"
    "  PortGangplankRetracted      0x3eb1 -2  0  0\n"
    "  StarboardGangplankExtended  0x3ed4  2  0  0\n"
    "  StarboardGangplankRetracted 0x3eb2  2  0  0\n"
    "  Hold                        0x3eae  0 -4  0\n"
    "  Hold                        0x3eae  0 -3  0\n"
    "}\n";
const Core::Vec3d SECOND_HOLD_DELTA( 0, -3, 0 );

const std::string testdir = unittest_path( "boat" );

struct Parts
{
  Items::Item* tillerman;
  Items::Item* portplank;
  Items::Item* starboardplank;
  Items::Item* hold;
};

// A component as a save would find it: in the world, standing wherever the boat is.
Items::Item* part_at( u32 objtype, const Core::Pos4d& pos )
{
  auto* item = Items::Item::create( objtype );
  (void)Items::place_at( *item, pos );
  return item;
}

Parts parts_at( const Core::Pos4d& pos )
{
  const auto& extobj = Core::settingsManager.extobj;
  return Parts{ part_at( extobj.tillerman, pos ), part_at( extobj.port_plank, pos ),
                part_at( extobj.starboard_plank, pos ), part_at( extobj.hold, pos ) };
}

std::string component( const Items::Item* item )
{
  return fmt::format( "Component {:#x}", item->serial );
}

std::string traveller( const Items::Item* item )
{
  return fmt::format( "Traveller {:#x}", item->serial );
}

// Reported only when it fails. Each of these is a step a case cannot go on without.
bool require( bool ok, const std::string& what )
{
  if ( !ok )
  {
    UnitTest::inc_failures();
    UnitTest::report_failure( what, "false", "true" );
  }
  return ok;
}

// The element outlives the file it is read from, so what it reports goes here instead.
Clib::StubConfigSource elem_source;

// Reads the one element of a config file written from body.
bool read_elem( const std::string& name, const std::string& body, const std::string& type,
                Clib::ConfigElem& elem )
{
  const std::string path = testdir + "/" + name;
  {
    std::ofstream ofs( path );
    ofs << body;
  }
  Clib::ConfigFile cf( path.c_str(), type.c_str() );
  const bool read = cf.read( elem );
  elem.set_source( &elem_source );
  return read;
}

// Loads a boat at pos from a Multi element holding the given property lines, as the world load
// reads one from multis.txt.
Multi::UBoat* load_boat( const Core::Pos4d& pos, const std::vector<std::string>& lines )
{
  std::string body =
      fmt::format( "Multi\n{{\n  ObjType {:#x}\n  Graphic 0x4000\n  MultiID 0\n", BOAT_OBJTYPE );
  body += fmt::format( "  X {}\n  Y {}\n  Z {}\n  Realm {}\n", pos.x(), pos.y(), pos.z(),
                       pos.realm()->name() );
  for ( const auto& line : lines )
    body += "  " + line + "\n";
  body += "}\n";

  Clib::ConfigElem elem;
  if ( !require( read_elem( "multis.txt", body, "Multi", elem ), "the saved boat reads" ) )
    return nullptr;

  auto* multi = Multi::UMulti::create( Items::find_itemdesc( BOAT_OBJTYPE ) );
  if ( !require( multi != nullptr, "the saved boat is created" ) )
    return nullptr;
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

// Moves the boat one tile, which puts every component it has a slot for at that slot's offset.
bool step( Multi::UBoat* boat )
{
  return require( boat->move_to( boat->pos() + Core::Vec2d( 1, 0 ), Core::MOVEITEM_FORCELOCATION ),
                  "the boat moves" );
}

bool destroy( Multi::UBoat* boat )
{
  Bscript::BObject res( Multi::destroy_boat( boat ) );
  return require( res.isTrue(), "the boat is destroyed" );
}

// Takes the boat back out of the world however the case ends, a failed check included. A boat left
// standing would take the next case's components aboard as travellers.
class BoatAfloat
{
public:
  explicit BoatAfloat( Multi::UBoat* boat ) : _boat( boat ) {}
  ~BoatAfloat() { destroy( _boat ); }
  BoatAfloat( const BoatAfloat& ) = delete;
  BoatAfloat& operator=( const BoatAfloat& ) = delete;

private:
  Multi::UBoat* _boat;
};

bool at( const Multi::UBoat* boat, const Items::Item* item, const Core::Vec3d& delta )
{
  return item->pos() == boat->pos() + delta;
}

// Puts a shape in place of multiid 0's for as long as it lives. The table deletes what it holds at
// shutdown, so the test shard's own shape has to be back by then.
class ShapeSwap
{
public:
  explicit ShapeSwap( Multi::BoatShape* shape ) : _original( Core::gamestate.boatshapes[0] )
  {
    Core::gamestate.boatshapes[0] = shape;
  }
  ~ShapeSwap() { Core::gamestate.boatshapes[0] = _original; }
  ShapeSwap( const ShapeSwap& ) = delete;
  ShapeSwap& operator=( const ShapeSwap& ) = delete;

private:
  Multi::BoatShape* _original;
};
}  // namespace

void boat_load_test()
{
  UnitTestDir dir( testdir );
  auto* realm = Core::gamestate.Realms[0];
  const Core::Pos4d pos( 10, 50, -4, realm );

  // the tillerman listed twice
  {
    auto p = parts_at( pos );
    auto* boat = load_boat(
        pos, { component( p.tillerman ), component( p.tillerman ), component( p.portplank ),
               component( p.starboardplank ), component( p.hold ) } );
    if ( !boat )
      return;
    BoatAfloat afloat( boat );
    if ( !step( boat ) )
      return;

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
  }

  // the same save, turned instead of moved
  {
    auto p = parts_at( pos );
    auto* boat = load_boat(
        pos, { component( p.tillerman ), component( p.tillerman ), component( p.portplank ),
               component( p.starboardplank ), component( p.hold ) } );
    if ( !boat )
      return;
    BoatAfloat afloat( boat );
    if ( !require( boat->turn( Multi::UBoat::RIGHT ), "the boat turns" ) )
      return;

    UnitTest(
        [&]()
        {
          return at( boat, p.tillerman, TURNED_TILLERMAN_DELTA ) &&
                 at( boat, p.portplank, TURNED_PORTPLANK_DELTA ) &&
                 at( boat, p.starboardplank, TURNED_STARBOARDPLANK_DELTA ) &&
                 at( boat, p.hold, TURNED_HOLD_DELTA );
        },
        true, "a boat with a component listed twice turns every component to its offset" );
    UnitTest(
        [&]()
        {
          return p.tillerman->graphic == TURNED_TILLERMAN_GRAPHIC &&
                 p.portplank->graphic == TURNED_PORTPLANK_GRAPHIC &&
                 p.starboardplank->graphic == TURNED_STARBOARDPLANK_GRAPHIC &&
                 p.hold->graphic == TURNED_HOLD_GRAPHIC;
        },
        true, "a boat with a component listed twice gives every component its turned graphic" );
  }

  // the port plank named on a Traveller line as well as on its own Component line
  {
    auto p = parts_at( pos );
    auto* boat = load_boat(
        pos, { traveller( p.portplank ), component( p.tillerman ), component( p.portplank ),
               component( p.starboardplank ), component( p.hold ) } );
    if ( !boat )
      return;
    BoatAfloat afloat( boat );
    if ( !step( boat ) )
      return;

    UnitTest( [&]() { return saved_component_count( boat ); }, size_t( 4 ),
              "a component also listed as a traveller is kept once" );
    UnitTest( [&]() { return at( boat, p.tillerman, TILLERMAN_DELTA ); }, true,
              "the component before one named on both lines keeps its offset" );
    UnitTest( [&]() { return at( boat, p.portplank, PORTPLANK_DELTA ); }, true,
              "a component also listed as a traveller moves to its own offset" );
    UnitTest(
        [&]() {
          return at( boat, p.starboardplank, STARBOARDPLANK_DELTA ) &&
                 at( boat, p.hold, HOLD_DELTA );
        },
        true, "the components after one named on both lines keep their offsets" );
  }

  // a plank of the boat's own objtype lying loose on the deck, named on a Traveller line only: it
  // is not one of the boat's components and must not take the slot of the one that is
  {
    auto p = parts_at( pos );
    auto* loose_plank = part_at( Core::settingsManager.extobj.port_plank, pos );
    auto* boat = load_boat(
        pos, { traveller( loose_plank ), component( p.tillerman ), component( p.portplank ),
               component( p.starboardplank ), component( p.hold ) } );
    if ( !boat )
      return;
    BoatAfloat afloat( boat );
    if ( !step( boat ) )
      return;

    UnitTest(
        [&]()
        {
          return at( boat, p.tillerman, TILLERMAN_DELTA ) &&
                 at( boat, p.portplank, PORTPLANK_DELTA ) &&
                 at( boat, p.starboardplank, STARBOARDPLANK_DELTA ) &&
                 at( boat, p.hold, HOLD_DELTA );
        },
        true, "a loose plank named as a traveller takes no component's slot" );
  }

  // a component that shares its objtype with another, listed twice with the extra copy first: it
  // has to stay in its own slot rather than take the first one of its objtype
  {
    Clib::ConfigElem shapeelem;
    if ( !require( read_elem( "boats.cfg", two_hold_shape, "Boat", shapeelem ),
                   "the two-hold shape reads" ) )
      return;
    Multi::BoatShape shape( shapeelem );
    ShapeSwap swap( &shape );

    auto p = parts_at( pos );
    auto* second_hold = part_at( Core::settingsManager.extobj.hold, pos );
    auto* boat = load_boat(
        pos, { component( second_hold ), component( p.tillerman ), component( p.portplank ),
               component( p.starboardplank ), component( p.hold ), component( second_hold ) } );
    if ( !boat )
      return;
    BoatAfloat afloat( boat );
    if ( !step( boat ) )
      return;

    UnitTest( [&]() { return saved_component_count( boat ); }, size_t( 5 ),
              "a component that shares its objtype, listed twice, is kept once" );
    UnitTest( [&]() { return at( boat, p.hold, HOLD_DELTA ); }, true,
              "the first of two components sharing an objtype keeps its slot" );
    UnitTest( [&]() { return at( boat, second_hold, SECOND_HOLD_DELTA ); }, true,
              "the second of two components sharing an objtype keeps its slot" );
  }

  // no port plank at all, as when one was destroyed before the save
  {
    auto p = parts_at( pos );
    p.portplank->destroy();
    auto* boat = load_boat(
        pos, { component( p.tillerman ), component( p.starboardplank ), component( p.hold ) } );
    if ( !boat )
      return;
    BoatAfloat afloat( boat );
    if ( !step( boat ) )
      return;

    UnitTest( [&]() { return saved_component_count( boat ); }, size_t( 3 ),
              "a missing component is not made up" );
    UnitTest( [&]() { return at( boat, p.starboardplank, STARBOARDPLANK_DELTA ); }, true,
              "the component after a missing one does not take its offset" );
    UnitTest( [&]() { return at( boat, p.hold, HOLD_DELTA ); }, true,
              "the last component after a missing one keeps its offset" );
  }

  // a well-formed save, the case every shard loads
  {
    auto p = parts_at( pos );
    auto* boat = load_boat( pos, { component( p.tillerman ), component( p.portplank ),
                                   component( p.starboardplank ), component( p.hold ) } );
    if ( !boat )
      return;
    BoatAfloat afloat( boat );
    if ( !step( boat ) )
      return;

    UnitTest(
        [&]()
        {
          return at( boat, p.tillerman, TILLERMAN_DELTA ) &&
                 at( boat, p.portplank, PORTPLANK_DELTA ) &&
                 at( boat, p.starboardplank, STARBOARDPLANK_DELTA ) &&
                 at( boat, p.hold, HOLD_DELTA );
        },
        true, "a well-formed save moves every component to its offset" );
  }
}
}  // namespace Pol::Testing
