/** @file
 *
 * @par History
 * - 2005/01/23 Shinigami: check_item_integrity & check_character_integrity - fix for multi realm
 * support (had used WGRID_X & WGRID_Y)
 *                         ClrCharacterWorldPosition - Tokuno MapDimension doesn't fit blocks of
 * 64x64 (WGRID_SIZE)
 * - 2009/09/03 MuadDib:   Relocation of multi related cpp/h
 * - 2012/02/06 MuadDib:   Added Old Serial for debug on orphaned items that make it to
 * remove_item_from_world.
 */


#include "uworld.h"

#include <stddef.h>
#include <string>

#include "../clib/clib_endian.h"
#include "../clib/logfacility.h"
#include "../clib/passert.h"
#include "globals/uvars.h"
#include "item/item.h"
#include "mobile/charactr.h"
#include "multi/multi.h"

namespace Pol
{
namespace Core
{
void add_item_to_world( Items::Item* item )
{
  Zone& zone = getzone( item->x(), item->y(), item->realm() );

  passert( std::find( zone.items.begin(), zone.items.end(), item ) == zone.items.end() );

  item->realm()->add_toplevel_item( *item );
  zone.items.push_back( item );
}

void remove_item_from_world( Items::Item* item )
{
  // Unregister the item if it is on a multi
  if ( item->container == nullptr && !item->has_gotten_by() )
  {
    Multi::UMulti* multi = item->realm()->find_supporting_multi( item->x(), item->y(), item->z() );

    if ( multi != nullptr )
      multi->unregister_object( item );
  }

  Zone& zone = getzone( item->x(), item->y(), item->realm() );

  ZoneItems::iterator itr = std::find( zone.items.begin(), zone.items.end(), item );
  if ( itr == zone.items.end() )
  {
    POLLOG_ERROR.Format(
        "remove_item_from_world: item 0x{:X} at {},{} does not exist in world zone ( Old Serial: "
        "0x{:X} )\n" )
        << item->serial << item->x() << item->y() << ( cfBEu32( item->serial_ext ) );

    passert( itr != zone.items.end() );
  }

  item->realm()->remove_toplevel_item( *item );
  zone.items.erase( itr );
}

void add_multi_to_world( Multi::UMulti* multi )
{
  Zone& zone = getzone( multi->x(), multi->y(), multi->realm() );
  zone.multis.push_back( multi );
  multi->realm()->add_multi( *multi );
}

void remove_multi_from_world( Multi::UMulti* multi )
{
  Zone& zone = getzone( multi->x(), multi->y(), multi->realm() );
  ZoneMultis::iterator itr = std::find( zone.multis.begin(), zone.multis.end(), multi );

  passert( itr != zone.multis.end() );

  multi->realm()->remove_multi( *multi );
  zone.multis.erase( itr );
}

void move_multi_in_world( unsigned short oldx, unsigned short oldy, unsigned short newx,
                          unsigned short newy, Multi::UMulti* multi, Realms::Realm* oldrealm )
{
  Zone& oldzone = getzone( oldx, oldy, oldrealm );
  Zone& newzone = getzone( newx, newy, multi->realm() );

  if ( &oldzone != &newzone )
  {
    ZoneMultis::iterator itr = std::find( oldzone.multis.begin(), oldzone.multis.end(), multi );
    passert( itr != oldzone.multis.end() );

    oldzone.multis.erase( itr );
    newzone.multis.push_back( multi );
  }

  if ( multi->realm() != oldrealm )
  {
    oldrealm->remove_multi( *multi );
    multi->realm()->add_multi( *multi );
  }
}

int get_toplevel_item_count()
{
  int count = 0;
  for ( const auto& realm : gamestate.Realms )
    count += realm->toplevel_item_count();
  return count;
}

int get_mobile_count()
{
  int count = 0;
  for ( const auto& realm : gamestate.Realms )
    count += realm->mobile_count();
  return count;
}

// 4-17-04 Rac destroyed the world! in favor of splitting its duties amongst the realms
// World world;

void SetCharacterWorldPosition( Mobile::Character* chr, Realms::WorldChangeReason reason )
{
  Zone& zone = getzone( chr->x(), chr->y(), chr->realm() );

  auto set_pos = [&]( ZoneCharacters& set )
  {
    passert( std::find( set.begin(), set.end(), chr ) == set.end() );
    set.push_back( chr );
  };

  if ( chr->isa( Core::UOBJ_CLASS::CLASS_NPC ) )
    set_pos( zone.npcs );
  else
    set_pos( zone.characters );

  chr->realm()->add_mobile( *chr, reason );
}

// Function for reporting the whereabouts of chars which are not in their expected zone
// (hopefully will never be called)
static void find_missing_char_in_zone( Mobile::Character* chr, Realms::WorldChangeReason reason );

void ClrCharacterWorldPosition( Mobile::Character* chr, Realms::WorldChangeReason reason )
{
  Zone& zone = getzone( chr->x(), chr->y(), chr->realm() );

  auto clear_pos = [&]( ZoneCharacters& set )
  {
    auto itr = std::find( set.begin(), set.end(), chr );
    if ( itr == set.end() )
    {
      find_missing_char_in_zone(
          chr, reason );  // Uh-oh, char was not in the expected zone. Find it and report.
      passert( itr != set.end() );
    }
    chr->realm()->remove_mobile( *chr, reason );
    set.erase( itr );
  };

  if ( !chr->isa( Core::UOBJ_CLASS::CLASS_NPC ) )
    clear_pos( zone.characters );
  else
    clear_pos( zone.npcs );
}

void MoveCharacterWorldPosition( unsigned short oldx, unsigned short oldy, unsigned short newx,
                                 unsigned short newy, Mobile::Character* chr,
                                 Realms::Realm* oldrealm )
{
  if ( oldrealm == nullptr )
    oldrealm = chr->realm();

  // If the char is logged in (logged_in is always true for NPCs), update its position
  // in the world zones
  if ( chr->logged_in() )
  {
    Zone& oldzone = getzone( oldx, oldy, oldrealm );
    Zone& newzone = getzone( newx, newy, chr->realm() );

    if ( &oldzone != &newzone )
    {
      auto move_pos = [&]( ZoneCharacters& oldset, ZoneCharacters& newset )
      {
        auto oldset_itr = std::find( oldset.begin(), oldset.end(), chr );

        // ensure it's found in the old realm
        passert( oldset_itr != oldset.end() );
        // and that it's not yet in the new realm
        passert( std::find( newset.begin(), newset.end(), chr ) == newset.end() );

        oldset.erase( oldset_itr );
        newset.push_back( chr );
      };

      if ( !chr->isa( Core::UOBJ_CLASS::CLASS_NPC ) )
        move_pos( oldzone.characters, newzone.characters );
      else
        move_pos( oldzone.npcs, newzone.npcs );
    }
  }

  // Regardless of online or not, tell the realms that we've left
  if ( chr->realm() != oldrealm )
  {
    oldrealm->remove_mobile( *chr, Realms::WorldChangeReason::Moved );
    chr->realm()->add_mobile( *chr, Realms::WorldChangeReason::Moved );
  }
}

void MoveItemWorldPosition( unsigned short oldx, unsigned short oldy, Items::Item* item,
                            Realms::Realm* oldrealm )
{
  if ( oldrealm == nullptr )
    oldrealm = item->realm();

  Zone& oldzone = getzone( oldx, oldy, oldrealm );
  Zone& newzone = getzone( item->x(), item->y(), item->realm() );

  if ( &oldzone != &newzone )
  {
    ZoneItems::iterator itr = std::find( oldzone.items.begin(), oldzone.items.end(), item );

    if ( itr == oldzone.items.end() )
    {
      POLLOG_ERROR.Format(
          "MoveItemWorldPosition: item 0x{:X} at old-x/y({},{} - {}) new-x/y({},{} - {}) does not "
          "exist in world zone. \n" )
          << item->serial << oldx << oldy << oldrealm->name() << item->x() << item->y()
          << item->realm()->name();

      passert( itr != oldzone.items.end() );
    }

    oldzone.items.erase( itr );

    passert( std::find( newzone.items.begin(), newzone.items.end(), item ) == newzone.items.end() );
    newzone.items.push_back( item );
  }

  if ( oldrealm != item->realm() )
  {
    oldrealm->remove_toplevel_item( *item );
    item->realm()->add_toplevel_item( *item );
  }
}

// If the ClrCharacterWorldPosition() fails, this function will find the actual char position and
// report
// TODO: check if this is really needed...
void find_missing_char_in_zone( Mobile::Character* chr, Realms::WorldChangeReason reason )
{
  unsigned wgridx = chr->realm()->grid_width();
  unsigned wgridy = chr->realm()->grid_height();

  std::string msgreason = "unknown reason";
  switch ( reason )
  {
  case Realms::WorldChangeReason::PlayerExit:
    msgreason = "Client Exit";
    break;
  case Realms::WorldChangeReason::NpcDeath:
    msgreason = "NPC death";
    break;
  default:
    break;
  }

  POLLOG_ERROR.Format(
      "ClrCharacterWorldPosition({}): mob (0x{:X},0x{:X}) supposedly at ({},{}) isn't in correct "
      "zone\n" )
      << msgreason << chr->serial << chr->serial_ext << chr->x() << chr->y();

  bool is_npc = chr->isa( Core::UOBJ_CLASS::CLASS_NPC );
  for ( unsigned zonex = 0; zonex < wgridx; ++zonex )
  {
    for ( unsigned zoney = 0; zoney < wgridy; ++zoney )
    {
      bool found = false;
      if ( is_npc )
      {
        auto _z = chr->realm()->zone[zonex][zoney].npcs;
        found = std::find( _z.begin(), _z.end(), chr ) != _z.end();
      }
      else
      {
        auto _z = chr->realm()->zone[zonex][zoney].characters;
        found = std::find( _z.begin(), _z.end(), chr ) != _z.end();
      }
      if ( found )
        POLLOG_ERROR.Format( "ClrCharacterWorldPosition: Found mob in zone ({},{})\n" )
            << zonex << zoney;
    }
  }
}
// Dave added this for debugging a single zone

bool check_single_zone_item_integrity( int x, int y, Realms::Realm* realm )
{
  try
  {
    ZoneItems& witem = realm->zone[x][y].items;

    for ( const auto& item : witem )
    {
      unsigned short wx, wy;
      zone_convert( item->x(), item->y(), &wx, &wy, realm );
      if ( wx != x || wy != y )
      {
        POLLOG_ERROR.Format( "Item 0x{:X} in zone ({},{}) but location is ({},{}) (zone {},{})\n" )
            << item->serial << x << y << item->x() << item->y() << wx << wy;
        return false;
      }
    }
  }
  catch ( ... )
  {
    POLLOG_ERROR.Format( "item integ problem at zone ({},{})\n" ) << x << y;
    return false;
  }
  return true;
}


bool check_item_integrity()
{
  bool ok = true;
  for ( auto& realm : gamestate.Realms )
  {
    unsigned int gridwidth = realm->grid_width();
    unsigned int gridheight = realm->grid_height();

    for ( unsigned x = 0; x < gridwidth; ++x )
    {
      for ( unsigned y = 0; y < gridheight; ++y )
      {
        if ( !check_single_zone_item_integrity( x, y, realm ) )
          ok = false;
      }
    }
  }
  return ok;
}

void check_character_integrity()
{
  // TODO: iterate through the object hash?
  // for( unsigned i = 0; i < characters.size(); ++i )
  //{
  //    Character* chr = characters[i];
  //    unsigned short wx, wy;
  //    w_convert( chr->x, chr->y, wx, wy );
  //    if (!world.zone[wx][wy].characters.count(chr))
  //    {
  //        cout << "Character " << chr->serial << " at " << chr->x << "," << chr->y << " is not in
  //        its zone." << endl;
  //    }
  //}
  for ( auto& realm : gamestate.Realms )
  {
    unsigned int gridwidth = realm->grid_width();
    unsigned int gridheight = realm->grid_height();

    auto check_zone = []( Mobile::Character* chr, unsigned y, unsigned x )
    {
      unsigned short wx, wy;
      zone_convert( chr->x(), chr->y(), &wx, &wy, chr->realm() );
      if ( wx != x || wy != y )
        INFO_PRINT << "Character 0x" << fmt::hexu( chr->serial ) << " in a zone, but elsewhere\n";
    };

    for ( unsigned x = 0; x < gridwidth; ++x )
    {
      for ( unsigned y = 0; y < gridheight; ++y )
      {
        for ( const auto& chr : realm->zone[x][y].characters )
          check_zone( chr, y, x );
        for ( const auto& chr : realm->zone[x][y].npcs )
          check_zone( chr, y, x );
      }
    }
  }
}

// reallocates all vectors to fit the current size
void optimize_zones()
{
  for ( auto& realm : gamestate.Realms )
  {
    unsigned int gridwidth = realm->grid_width();
    unsigned int gridheight = realm->grid_height();

    for ( unsigned x = 0; x < gridwidth; ++x )
    {
      for ( unsigned y = 0; y < gridheight; ++y )
      {
        realm->zone[x][y].characters.shrink_to_fit();
        realm->zone[x][y].npcs.shrink_to_fit();
        realm->zone[x][y].items.shrink_to_fit();
        realm->zone[x][y].multis.shrink_to_fit();
      }
    }
  }
}
}  // namespace Core
}  // namespace Pol
