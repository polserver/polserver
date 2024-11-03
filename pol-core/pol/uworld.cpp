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
  Zone& zone = item->realm()->getzone( item->pos().xy() );

  passert( std::find( zone.items.begin(), zone.items.end(), item ) == zone.items.end() );

  zone.items.push_back( item );
  item->realm()->add_toplevel_item( item );
}

void remove_item_from_world( Items::Item* item )
{
  // Unregister the item if it is on a multi
  if ( item->container == nullptr && !item->has_gotten_by() )
  {
    Multi::UMulti* multi = item->realm()->find_supporting_multi( item->pos().xyz() );

    if ( multi != nullptr )
      multi->unregister_object( item );
  }

  Zone& zone = item->realm()->getzone( item->pos().xy() );

  ZoneItems::iterator itr = std::find( zone.items.begin(), zone.items.end(), item );
  if ( itr == zone.items.end() )
  {
    POLLOG_ERRORLN(
        "remove_item_from_world: item {:#x} at {} does not exist in world zone ( Old Serial: {:#x} "
        ")",
        item->serial, item->pos2d(), ( cfBEu32( item->serial_ext ) ) );

    passert( itr != zone.items.end() );
  }

  zone.items.erase( itr );
  item->realm()->remove_toplevel_item( item );
}

void add_multi_to_world( Multi::UMulti* multi )
{
  Zone& zone = multi->realm()->getzone( multi->pos2d() );
  zone.multis.push_back( multi );
  multi->realm()->add_multi( *multi );  // needs to be the last
}

void remove_multi_from_world( Multi::UMulti* multi )
{
  Zone& zone = multi->realm()->getzone( multi->pos2d() );
  ZoneMultis::iterator itr = std::find( zone.multis.begin(), zone.multis.end(), multi );

  passert( itr != zone.multis.end() );

  zone.multis.erase( itr );
  multi->realm()->remove_multi( *multi );  // needs to be the last
}

void move_multi_in_world( Multi::UMulti* multi, const Core::Pos4d& oldpos )
{
  Zone& oldzone = oldpos.realm()->getzone( oldpos.xy() );
  Zone& newzone = multi->realm()->getzone( multi->pos2d() );

  if ( &oldzone != &newzone )
  {
    ZoneMultis::iterator itr = std::find( oldzone.multis.begin(), oldzone.multis.end(), multi );
    passert( itr != oldzone.multis.end() );

    oldzone.multis.erase( itr );
    newzone.multis.push_back( multi );
  }

  if ( multi->realm() != oldpos.realm() )
  {
    oldpos.realm()->remove_multi( *multi );
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
  Zone& zone = chr->realm()->getzone( chr->pos().xy() );

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
  Zone& zone = chr->realm()->getzone( chr->pos().xy() );

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

void MoveCharacterWorldPosition( const Core::Pos4d& oldpos, Mobile::Character* chr )
{
  // If the char is logged in (logged_in is always true for NPCs), update its position
  // in the world zones
  if ( chr->logged_in() )
  {
    Zone& oldzone = oldpos.realm()->getzone( oldpos.xy() );
    Zone& newzone = chr->realm()->getzone( chr->pos2d() );
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
  if ( chr->realm() != oldpos.realm() )
  {
    oldpos.realm()->remove_mobile( *chr, Realms::WorldChangeReason::Moved );
    chr->realm()->add_mobile( *chr, Realms::WorldChangeReason::Moved );
  }
}

void MoveItemWorldPosition( const Core::Pos4d& oldpos, Items::Item* item )
{
  Zone& oldzone = oldpos.realm()->getzone( oldpos.xy() );
  Zone& newzone = item->realm()->getzone( item->pos().xy() );

  if ( &oldzone != &newzone )
  {
    ZoneItems::iterator itr = std::find( oldzone.items.begin(), oldzone.items.end(), item );

    if ( itr == oldzone.items.end() )
    {
      POLLOG_ERRORLN(
          "MoveItemWorldPosition: item {:#x} at old {} new {} does not exist in world zone.",
          item->serial, oldpos, item->pos() );

      passert( itr != oldzone.items.end() );
    }

    oldzone.items.erase( itr );

    passert( std::find( newzone.items.begin(), newzone.items.end(), item ) == newzone.items.end() );
    newzone.items.push_back( item );
  }

  if ( oldpos.realm() != item->realm() )
  {
    oldpos.realm()->remove_toplevel_item( item );
    item->realm()->add_toplevel_item( item );
  }
}

// If the ClrCharacterWorldPosition() fails, this function will find the actual char position and
// report
// TODO: check if this is really needed...
void find_missing_char_in_zone( Mobile::Character* chr, Realms::WorldChangeReason reason )
{
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

  POLLOG_ERRORLN(
      "ClrCharacterWorldPosition({}): mob ({:#x},{:#x}) supposedly at {} isn't in correct zone.",
      msgreason, chr->serial, chr->serial_ext, chr->pos() );

  bool is_npc = chr->isa( Core::UOBJ_CLASS::CLASS_NPC );
  for ( const auto& p : chr->realm()->gridarea() )
  {
    bool found = false;
    if ( is_npc )
    {
      auto _z = chr->realm()->getzone_grid( p ).npcs;
      found = std::find( _z.begin(), _z.end(), chr ) != _z.end();
    }
    else
    {
      auto _z = chr->realm()->getzone_grid( p ).characters;
      found = std::find( _z.begin(), _z.end(), chr ) != _z.end();
    }
    if ( found )
      POLLOG_ERRORLN( "ClrCharacterWorldPosition: Found mob in zone {}", p );
  }
}
// Dave added this for debugging a single zone

bool check_single_zone_item_integrity( const Pos2d& pos, Realms::Realm* realm )
{
  try
  {
    ZoneItems& witem = realm->getzone_grid( pos ).items;

    for ( const auto& item : witem )
    {
      Core::Pos2d wpos = zone_convert( item->pos() );
      if ( wpos != pos )
      {
        POLLOG_ERRORLN( "Item {:#x} in zone {} but location is {} (zone {})", item->serial, pos,
                        item->pos(), wpos );
        return false;
      }
    }
  }
  catch ( ... )
  {
    POLLOG_ERRORLN( "item integ problem at zone {}", pos );
    return false;
  }
  return true;
}


bool check_item_integrity()
{
  bool ok = true;
  for ( auto& realm : gamestate.Realms )
  {
    for ( const auto& p : realm->gridarea() )
    {
      if ( !check_single_zone_item_integrity( p, realm ) )
        ok = false;
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
    auto check_zone = []( Mobile::Character* chr, const Pos2d& p )
    {
      Core::Pos2d wpos = zone_convert( chr->pos() );
      if ( wpos != p )
        INFO_PRINTLN( "Character {:#x} in a zone, but elsewhere", chr->serial );
    };

    for ( const auto& p : realm->gridarea() )
    {
      for ( const auto& chr : realm->getzone_grid( p ).characters )
        check_zone( chr, p );
      for ( const auto& chr : realm->getzone_grid( p ).npcs )
        check_zone( chr, p );
    }
  }
}

// reallocates all vectors to fit the current size
void optimize_zones()
{
  for ( auto& realm : gamestate.Realms )
  {
    for ( const auto& p : realm->gridarea() )
    {
      realm->getzone_grid( p ).characters.shrink_to_fit();
      realm->getzone_grid( p ).npcs.shrink_to_fit();
      realm->getzone_grid( p ).items.shrink_to_fit();
      realm->getzone_grid( p ).multis.shrink_to_fit();
    }
  }
}
}  // namespace Core
}  // namespace Pol
