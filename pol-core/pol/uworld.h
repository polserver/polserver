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
#include "base/range.h"
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
void move_multi_in_world( unsigned short oldx, unsigned short oldy, unsigned short newx,
                          unsigned short newy, Multi::UMulti* multi,
                          Realms::Realm* oldrealm );  // TODO Pos dont pass both new and old.

void SetCharacterWorldPosition( Mobile::Character* chr, Realms::WorldChangeReason reason );
void ClrCharacterWorldPosition( Mobile::Character* chr, Realms::WorldChangeReason reason );
void MoveCharacterWorldPosition( const Core::Pos4d& oldpos, Mobile::Character* chr );

void SetItemWorldPosition( Items::Item* item );
void ClrItemWorldPosition( Items::Item* item );
void MoveItemWorldPosition( const Core::Pos4d& oldpos, Items::Item* item );

int get_toplevel_item_count();
int get_mobile_count();

void optimize_zones();
bool check_single_zone_item_integrity( const Pos2d& pos, Realms::Realm* realm );

inline Pos2d zone_convert( const Pos4d& p )
{
  return Pos2d( static_cast<unsigned short>( p.x() >> Plib::WGRID_SHIFT ),
                static_cast<unsigned short>( p.y() >> Plib::WGRID_SHIFT ) );
}
inline Pos2d zone_convert( const Pos2d& p )
{
  return Pos2d( static_cast<unsigned short>( p.x() >> Plib::WGRID_SHIFT ),
                static_cast<unsigned short>( p.y() >> Plib::WGRID_SHIFT ) );
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
  static void InRange( u16 x, u16 y, const Realms::Realm* realm, unsigned range,
                       F&& f );  // TODO Pos
  template <typename F>
  static void InRange( const Pos2d& pos, const Realms::Realm* realm, unsigned range, F&& f );
  template <typename F>
  static void InRange( const Pos4d& pos, unsigned range, F&& f );
  template <typename F>
  static void InVisualRange( const UObject* obj, F&& f );
  template <typename F>
  static void InVisualRange( const Pos2d& pos, const Realms::Realm* realm, F&& f );
  template <typename F>
  static void InVisualRange( const Pos4d& pos, F&& f );
  template <typename F>
  static void InBox( u16 x1, u16 y1, u16 x2, u16 y2, const Realms::Realm* realm,
                     F&& f );  // TODO Pos
  template <typename F>
  static void InBox( Range2d area, const Realms::Realm* realm, F&& f );

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
  CoordsArea( Range2d box, const Realms::Realm* posrealm );                     // create from box

  bool inRange( const UObject* obj ) const;

  // shifted coords
  Range2d warea;
  const Realms::Realm* realm;

private:
  static Pos2d convert( const Pos2d& p );

  // plain coords
  Range2d area;
};
}  // namespace
///////////////
// imp
namespace
{
inline CoordsArea::CoordsArea( const Pos2d& p, const Realms::Realm* posrealm, unsigned range )
{
  realm = posrealm;
  if ( range > static_cast<u32>( std::numeric_limits<s16>::max() ) )
    range = std::numeric_limits<s16>::max();
  Vec2d r( static_cast<s16>( range ), static_cast<s16>( range ) );
  area = Range2d( p - r, p + r, realm );
  warea = Range2d( convert( area.nw() ), convert( area.se() ), nullptr );
}
inline CoordsArea::CoordsArea( const Pos4d& p, unsigned range )
{
  realm = p.realm();
  if ( range > static_cast<u32>( std::numeric_limits<s16>::max() ) )
    range = std::numeric_limits<s16>::max();
  Vec2d r( static_cast<s16>( range ), static_cast<s16>( range ) );
  area = Range2d( p - r, p + r );
  warea = Range2d( convert( area.nw() ), convert( area.se() ), nullptr );
}
inline CoordsArea::CoordsArea( const Pos4d& p1, const Pos4d& p2 )
{
  realm = p1.realm();
  area = Range2d( p1.xy(), p2.xy(), realm );
  warea = Range2d( convert( area.nw() ), convert( area.se() ), nullptr );
}
inline CoordsArea::CoordsArea( Range2d box, const Realms::Realm* posrealm )
{
  realm = posrealm;
  area = std::move( box );
  warea = Range2d( convert( area.nw() ), convert( area.se() ), nullptr );
}

inline bool CoordsArea::inRange( const UObject* obj ) const
{
  // TODO Pos:
  // mmmh i guess we need three versions/have three usecases
  // 1. simple area.contains eg for script functions
  // 2. for pkts and stuff obj->inrange virtual which checks parent multi etc
  // 3. like 2. but with the chr who wants to see the obj (his actual viewrange)
  // keep this dumb with the maximum possible range here and filter further on callerside?
  return area.contains( obj->pos().xy() );
}

inline Pos2d CoordsArea::convert( const Pos2d& p )
{
  // zone_convert, but without Pos4d.
  return Pos2d( static_cast<s16>( p.x() >> Plib::WGRID_SHIFT ),
                static_cast<s16>( p.y() >> Plib::WGRID_SHIFT ) );
}
}  // namespace

template <class Filter>
template <typename F>
void WorldIterator<Filter>::InRange( u16 x, u16 y, const Realms::Realm* realm, unsigned range,
                                     F&& f )
{
  InRange( Pos2d( x, y ), realm, range, f );
}
template <class Filter>
template <typename F>
void WorldIterator<Filter>::InRange( const Pos2d& pos, const Realms::Realm* realm, unsigned range,
                                     F&& f )
{
  if ( realm == nullptr )
    return;
  CoordsArea coords( pos, realm, range );
  _forEach( coords, std::forward<F>( f ) );
}
template <class Filter>
template <typename F>
void WorldIterator<Filter>::InRange( const Pos4d& pos, unsigned range, F&& f )
{
  if ( pos.realm() == nullptr )
    return;
  CoordsArea coords( pos, range );
  _forEach( coords, std::forward<F>( f ) );
}

template <class Filter>
template <typename F>
void WorldIterator<Filter>::InVisualRange( const UObject* obj, F&& f )
{
  // TODO RANGE_VISUAL needs to be something dynamic (client viewrange maximum, max multi size)
  InRange( obj->toplevel_owner()->pos(), RANGE_VISUAL, std::forward<F>( f ) );
}
template <class Filter>
template <typename F>
void WorldIterator<Filter>::InVisualRange( const Pos2d& pos, const Realms::Realm* realm, F&& f )
{
  // TODO RANGE_VISUAL needs to be something dynamic (client viewrange maximum, max multi size)
  InRange( pos, realm, RANGE_VISUAL, std::forward<F>( f ) );
}
template <class Filter>
template <typename F>
void WorldIterator<Filter>::InVisualRange( const Pos4d& pos, F&& f )
{
  // TODO RANGE_VISUAL needs to be something dynamic (client viewrange maximum, max multi size)
  InRange( pos, RANGE_VISUAL, std::forward<F>( f ) );
}
template <class Filter>
template <typename F>
void WorldIterator<Filter>::InBox( u16 x1, u16 y1, u16 x2, u16 y2, const Realms::Realm* realm,
                                   F&& f )
{
  InBox( Range2d( Pos2d( x1, y1 ), Pos2d( x2, y2 ), realm ), realm, std::forward<F>( f ) );
}
template <class Filter>
template <typename F>
void WorldIterator<Filter>::InBox( Range2d area, const Realms::Realm* realm, F&& f )
{
  if ( realm == nullptr )
    return;
  CoordsArea coords( std::move( area ), realm );
  _forEach( coords, std::forward<F>( f ) );
}

template <class Filter>
template <typename F>
void WorldIterator<Filter>::_forEach( const CoordsArea& coords, F&& f )
{
  for ( const auto& p : coords.warea )
  {
    Filter::call( coords.realm->getzone_grid( p ), coords, f );
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
