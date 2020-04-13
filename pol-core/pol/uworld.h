/** @file
 *
 * @par History
 * - 2006/09/27 Shinigami: GCC 3.4.x fix - added "#include "charactr.h" because of
 * ForEachMobileInVisualRange
 * - 2009/10/17 Turley:    added ForEachItemInRange & ForEachItemInVisualRange
 */


#ifndef UWORLD_H
#define UWORLD_H

#ifndef __UOBJECT_H
#include "uobject.h"
#endif
#ifndef ITEM_H
#include "item/item.h"
#endif
#ifndef MULTI_H
#include "multi/multi.h"
#endif
#ifndef __CHARACTR_H
#include "mobile/charactr.h"
#endif
#include <algorithm>
#include <vector>

#include "../clib/passert.h"
#include "../clib/rawtypes.h"
#include "../plib/realmdescriptor.h"
#include "../plib/uconst.h"
#include "base/position.h"
#include "realms/WorldChangeReasons.h"
#include "realms/realm.h"
#include "zone.h"

namespace Pol
{
namespace Core
{
void add_item_to_world( Items::Item* item );
void remove_item_from_world( Items::Item* item );

void add_multi_to_world( Multi::UMulti* multi );
void remove_multi_from_world( Multi::UMulti* multi );
void move_multi_in_world( const Pos4d& oldpos, Multi::UMulti* multi );

void SetCharacterWorldPosition( Mobile::Character* chr, Realms::WorldChangeReason reason );
void ClrCharacterWorldPosition( Mobile::Character* chr, Realms::WorldChangeReason reason );
void MoveCharacterWorldPosition( const Pos4d& oldpos, Mobile::Character* chr );

void SetItemWorldPosition( Items::Item* item );
void ClrItemWorldPosition( Items::Item* item );
void MoveItemWorldPosition( const Pos4d& oldpos, Items::Item* item );

int get_toplevel_item_count();
int get_mobile_count();

bool check_single_zone_item_integrity( const Pos2d& grid_xy, Realms::Realm* realm );
void optimize_zones();
bool check_item_integrity();

typedef std::vector<Mobile::Character*> ZoneCharacters;
typedef std::vector<Multi::UMulti*> ZoneMultis;
typedef std::vector<Items::Item*> ZoneItems;

struct Zone
{
  ZoneCharacters characters;
  ZoneCharacters npcs;
  ZoneItems items;
  ZoneMultis multis;
};

inline Pos2d zone_convert( const Pos4d& p )
{
  return Pos2d( static_cast<unsigned short>( p.x() >> Plib::WGRID_SHIFT ),
                static_cast<unsigned short>( p.y() >> Plib::WGRID_SHIFT ) );
}

inline Zone& getzone( const Pos4d& p )
{
  return p.realm()->zone[p.x() >> Plib::WGRID_SHIFT][p.y() >> Plib::WGRID_SHIFT];
}

namespace
{
struct CoordsArea;
}
// Helper functions to iterator over world items in given realm an range
// takes any function with only one open parameter
// its recommended to use lambdas even if std::bind can also be used, but a small benchmark showed
// that std::bind is a bit slower (compiler can optimize better lambdas)
// Usage:
// WorldIterator<PlayerFilter>::InRange(...)

// main struct, define as template param which filter to use
template <class Filter>
struct WorldIterator
{
  template <typename F>
  static void InRange( const Pos2d& p, const Realms::Realm* realm, unsigned range, F&& f );
  template <typename F>
  static void InRange( const Pos4d& p, unsigned range, F&& f );
  template <typename F>
  static void InVisualRange( const UObject* obj, F&& f );
  template <typename F>
  static void InBox( const Pos4d& p1, const Pos4d& p2, F&& f );

protected:
  template <typename F>
  static void _forEach( const CoordsArea& coords, F&& f );
};

enum class FilterType
{
  Mobile,        // iterator over npcs and players
  Player,        // iterator over player
  OnlinePlayer,  // iterator over online player
  NPC,           // iterator over npcs
  Item,          // iterator over items
  Multi          // iterator over multis
};

// Filter implementation struct,
// specializations for the enum values are given
template <FilterType T>
struct FilterImp
{
  friend struct WorldIterator<FilterImp<T>>;

protected:
  template <typename F>
  static void call( Core::Zone& zone, const CoordsArea& coords, F&& f );
};

// shortcuts for filtering
typedef FilterImp<FilterType::Mobile> MobileFilter;
typedef FilterImp<FilterType::Player> PlayerFilter;
typedef FilterImp<FilterType::OnlinePlayer> OnlinePlayerFilter;
typedef FilterImp<FilterType::NPC> NPCFilter;
typedef FilterImp<FilterType::Item> ItemFilter;
typedef FilterImp<FilterType::Multi> MultiFilter;

namespace
{
// template independent code
struct CoordsArea
{
  // structure to hold the world and shifted coords
  CoordsArea( const Pos2d& p, const Realms::Realm* posrealm, unsigned range );  // create from range
  CoordsArea( const Pos4d& p, unsigned range );                                 // create from range
  CoordsArea( const Pos4d& p1, const Pos4d& p2 );                               // create from box
  bool inRange( const UObject* obj ) const;

  // shifted coords
  Pos2d wL;
  Pos2d wH;
  const Realms::Realm* realm;

private:
  static Pos2d convert( const Pos2d& p );

  // plain coords
  Pos2d _posL;
  Pos2d _posH;
};
}  // namespace
///////////////
// imp
namespace
{
inline CoordsArea::CoordsArea( const Pos2d& p, const Realms::Realm* posrealm, unsigned range )
{
  realm = posrealm;
  Vec2d r( range, range );
  _posL = p - r;
  _posH = p + r;
  _posL.crop( realm );
  _posH.crop( realm );


  wL = convert( _posL );
  wH = convert( _posH );
  passert( wL <= wH );
}
inline CoordsArea::CoordsArea( const Pos4d& p, unsigned range )
{
  realm = p.realm();
  Vec2d r( range, range );
  _posL = ( p - r ).xy();
  _posH = ( p + r ).xy();

  wL = convert( _posL );
  wH = convert( _posH );
  passert( wL <= wH );
}

inline CoordsArea::CoordsArea( const Pos4d& p1, const Pos4d& p2 )
{
  realm = p1.realm();
  _posL = p1.xy();
  _posH = p2.xy();
  wL = convert( _posL );
  wH = convert( _posH );
  passert( wL <= wH );
}

inline bool CoordsArea::inRange( const UObject* obj ) const
{
  return ( obj->pos() >= _posL && obj->pos() <= _posH );
}

inline Pos2d CoordsArea::convert( const Pos2d& p )
{
  // zone_convert, but without Pos4d.
  // the guarantee of Pos4d (realm size) isnt needed here
  return Pos2d( static_cast<unsigned short>( p.x() >> Plib::WGRID_SHIFT ),
                static_cast<unsigned short>( p.y() >> Plib::WGRID_SHIFT ) );
}
}  // namespace

template <class Filter>
template <typename F>
void WorldIterator<Filter>::InRange( const Pos2d& p, const Realms::Realm* realm, unsigned range,
                                     F&& f )
{
  if ( realm == nullptr )
    return;
  CoordsArea coords( p, realm, range );
  _forEach( coords, std::forward<F>( f ) );
}
template <class Filter>
template <typename F>
void WorldIterator<Filter>::InRange( const Pos4d& p, unsigned range, F&& f )
{
  if ( p.realm() == nullptr )
    return;
  CoordsArea coords( p, range );
  _forEach( coords, std::forward<F>( f ) );
}

// Iterates over the items within visual range of the top-level position of this object or its
// top-level container
template <class Filter>
template <typename F>
void WorldIterator<Filter>::InVisualRange( const UObject* obj, F&& f )
{
  InRange( obj->toplevel_pos(), RANGE_VISUAL, std::forward<F>( f ) );
}
template <class Filter>
template <typename F>
void WorldIterator<Filter>::InBox( const Pos4d& p1, const Pos4d& p2, F&& f )
{
  if ( p1.realm() == nullptr || p1.realm() != p2.realm() )
    return;
  CoordsArea coords( p1, p2 );
  _forEach( coords, std::forward<F>( f ) );
}

template <class Filter>
template <typename F>
void WorldIterator<Filter>::_forEach( const CoordsArea& coords, F&& f )
{
  for ( u16 wx = coords.wL.x(); wx <= coords.wH.x(); ++wx )
  {
    for ( u16 wy = coords.wL.y(); wy <= coords.wH.y(); ++wy )
    {
      Filter::call( coords.realm->zone[wx][wy], coords, f );
    }
  }
}

// specializations of FilterImp

template <>
template <typename F>
void FilterImp<FilterType::Mobile>::call( Core::Zone& zone, const CoordsArea& coords, F&& f )
{
  for ( auto& chr : zone.characters )
  {
    if ( coords.inRange( chr ) )
      f( chr );
  }
  for ( auto& npc : zone.npcs )
  {
    if ( coords.inRange( npc ) )
      f( npc );
  }
}

template <>
template <typename F>
void FilterImp<FilterType::Player>::call( Core::Zone& zone, const CoordsArea& coords, F&& f )
{
  for ( auto& chr : zone.characters )
  {
    if ( coords.inRange( chr ) )
      f( chr );
  }
}

template <>
template <typename F>
void FilterImp<FilterType::OnlinePlayer>::call( Core::Zone& zone, const CoordsArea& coords, F&& f )
{
  for ( auto& chr : zone.characters )
  {
    if ( chr->has_active_client() )
    {
      if ( coords.inRange( chr ) )
        f( chr );
    }
  }
}

template <>
template <typename F>
void FilterImp<FilterType::NPC>::call( Core::Zone& zone, const CoordsArea& coords, F&& f )
{
  for ( auto& npc : zone.npcs )
  {
    if ( coords.inRange( npc ) )
      f( npc );
  }
}

template <>
template <typename F>
void FilterImp<FilterType::Item>::call( Core::Zone& zone, const CoordsArea& coords, F&& f )
{
  for ( auto& item : zone.items )
  {
    if ( coords.inRange( item ) )
      f( item );
  }
}

template <>
template <typename F>
void FilterImp<FilterType::Multi>::call( Core::Zone& zone, const CoordsArea& coords, F&& f )
{
  for ( auto& multi : zone.multis )
  {
    if ( coords.inRange( multi ) )
      f( multi );
  }
}
}  // namespace Core
}  // namespace Pol
#endif
