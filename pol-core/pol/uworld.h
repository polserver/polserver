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
void MoveCharacterWorldPosition( unsigned short oldx, unsigned short oldy, unsigned short newx,
                                 unsigned short newy, Mobile::Character* chr,
                                 Realms::Realm* oldrealm );  // TODO Pos dont pass both new and old

void SetItemWorldPosition( Items::Item* item );
void ClrItemWorldPosition( Items::Item* item );
void MoveItemWorldPosition( unsigned short oldx, unsigned short oldy, Items::Item* item,
                            Realms::Realm* oldrealm );  // TODO Pos

int get_toplevel_item_count();
int get_mobile_count();

void optimize_zones();
bool check_single_zone_item_integrity( const Pos2d& pos, Realms::Realm* realm );

inline void zone_convert( unsigned short x, unsigned short y, unsigned short* wx,
                          unsigned short* wy, const Realms::Realm* realm )
{
  passert( x < realm->width() );
  passert( y < realm->height() );

  ( *wx ) = x >> Plib::WGRID_SHIFT;
  ( *wy ) = y >> Plib::WGRID_SHIFT;
}


inline void zone_convert_clip( int x, int y, const Realms::Realm* realm, unsigned short* wx,
                               unsigned short* wy )
{
  if ( x < 0 )
    x = 0;
  if ( y < 0 )
    y = 0;
  if ( (unsigned)x >= realm->width() )
    x = realm->width() - 1;
  if ( (unsigned)y >= realm->height() )
    y = realm->height() - 1;

  ( *wx ) = static_cast<unsigned short>( x >> Plib::WGRID_SHIFT );
  ( *wy ) = static_cast<unsigned short>( y >> Plib::WGRID_SHIFT );
}

inline Zone& getzone( unsigned short x, unsigned short y, Realms::Realm* realm )
{
  passert( x < realm->width() );
  passert( y < realm->height() );

  return realm->getzone_grid( x >> Plib::WGRID_SHIFT, y >> Plib::WGRID_SHIFT );
}

inline Pos2d zone_convert( const Pos4d& p )
{
  return Pos2d( static_cast<unsigned short>( p.x() >> Plib::WGRID_SHIFT ),
                static_cast<unsigned short>( p.y() >> Plib::WGRID_SHIFT ) );
}

inline Zone& getzone( const Pos4d& p )
{
  return p.realm()->getzone_grid( Pos2d( p.x() >> Plib::WGRID_SHIFT, p.y() >> Plib::WGRID_SHIFT ) );
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
  static void InBox( u16 x1, u16 y1, u16 x2, u16 y2, const Realms::Realm* realm,
                     F&& f );  // TODO Pos
  template <typename F>
  static void InBox( const Range2d& area, const Realms::Realm* realm, F&& f );

protected:
  template <typename F>
  static void _forEach( const CoordsArea& coords, const Realms::Realm* realm, F&& f );
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
  CoordsArea( u16 x, u16 y, const Realms::Realm* realm, unsigned range );    // create from range
  CoordsArea( u16 x1, u16 y1, u16 x2, u16 y2, const Realms::Realm* realm );  // create from box
  bool inRange( const UObject* obj ) const;

  // shifted coords
  u16 wxL;
  u16 wyL;
  u16 wxH;
  u16 wyH;

private:
  void convert( int xL, int yL, int xH, int yH, const Realms::Realm* realm );

  // plain coords
  int _xL;
  int _yL;
  int _xH;
  int _yH;
};
}  // namespace
///////////////
// imp
namespace
{
inline CoordsArea::CoordsArea( u16 x, u16 y, const Realms::Realm* realm, unsigned range )
{
  convert( x - range, y - range, x + range, y + range, realm );
  _xL = x - range;
  if ( _xL < 0 )
    _xL = 0;
  _yL = y - range;
  if ( _yL < 0 )
    _yL = 0;
  _xH = x + range;
  _yH = y + range;
}

inline CoordsArea::CoordsArea( u16 x1, u16 y1, u16 x2, u16 y2, const Realms::Realm* realm )
{
  convert( x1, y1, x2, y2, realm );
  _xL = x1;
  _yL = y1;
  _xH = x2;
  _yH = y2;
}

inline bool CoordsArea::inRange( const UObject* obj ) const
{
  return ( obj->x() >= _xL && obj->x() <= _xH && obj->y() >= _yL && obj->y() <= _yH );
}

inline void CoordsArea::convert( int xL, int yL, int xH, int yH, const Realms::Realm* realm )
{
  zone_convert_clip( xL, yL, realm, &wxL, &wyL );
  zone_convert_clip( xH, yH, realm, &wxH, &wyH );
  passert( wxL <= wxH );
  passert( wyL <= wyH );
}
}  // namespace

template <class Filter>
template <typename F>
void WorldIterator<Filter>::InRange( u16 x, u16 y, const Realms::Realm* realm, unsigned range,
                                     F&& f )
{
  if ( realm == nullptr )
    return;
  CoordsArea coords( x, y, realm, range );
  _forEach( coords, realm, std::forward<F>( f ) );
}
template <class Filter>
template <typename F>
void WorldIterator<Filter>::InRange( const Pos2d& pos, const Realms::Realm* realm, unsigned range,
                                     F&& f )
{
  InRange( pos.x(), pos.y(), realm, range, f );
}
template <class Filter>
template <typename F>
void WorldIterator<Filter>::InRange( const Pos4d& pos, unsigned range, F&& f )
{
  InRange( pos.x(), pos.y(), pos.realm(), range, f );
}

template <class Filter>
template <typename F>
void WorldIterator<Filter>::InVisualRange( const UObject* obj, F&& f )
{
  InRange( obj->toplevel_owner()->x(), obj->toplevel_owner()->y(), obj->toplevel_owner()->realm(),
           RANGE_VISUAL, std::forward<F>( f ) );
}
template <class Filter>
template <typename F>
void WorldIterator<Filter>::InBox( u16 x1, u16 y1, u16 x2, u16 y2, const Realms::Realm* realm,
                                   F&& f )
{
  if ( realm == nullptr )
    return;
  CoordsArea coords( x1, y1, x2, y2, realm );
  _forEach( coords, realm, std::forward<F>( f ) );
}
template <class Filter>
template <typename F>
void WorldIterator<Filter>::InBox( const Range2d& area, const Realms::Realm* realm, F&& f )
{
  InBox( area.nw().x(), area.nw().y(), area.se().x(), area.se().y(), realm, f );
}

template <class Filter>
template <typename F>
void WorldIterator<Filter>::_forEach( const CoordsArea& coords, const Realms::Realm* realm, F&& f )
{
  for ( u16 wy = coords.wyL; wy <= coords.wyH; ++wy )
  {
    for ( u16 wx = coords.wxL; wx <= coords.wxH; ++wx )
    {
      Filter::call( realm->getzone_grid( wx, wy ), coords, f );
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
