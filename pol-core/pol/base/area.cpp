#include "area.h"
#include "../realms/realm.h"

#include <algorithm>

namespace Pol
{
namespace Core
{
Area2dItr::Area2dItr( Pos2d v, const Pos2d& v_max )
    : _v( std::move( v ) ), _xbound( v_max.x() ), _xstart( _v.x() ){};

bool Area2dItr::operator==( const Area2dItr& other ) const
{
  return _v == other._v;
}
bool Area2dItr::operator!=( const Area2dItr& other ) const
{
  return !( *this == other );
}
Area2dItr& Area2dItr::operator++()
{
  if ( _v.x() < _xbound )
    _v += Vec2d( 1, 0 );
  else
  {
    _v.x( _xstart );
    _v += Vec2d( 0, 1 );
  }
  return *this;
}

Area2d::Area2d( const Pos2d& p1, const Pos2d& p2, const Realms::Realm* realm )
{
  _nw = Pos2d( std::min( p1.x(), p2.x() ), std::min( p1.y(), p2.y() ) );
  _se = Pos2d( std::max( p1.x(), p2.x() ), std::max( p1.y(), p2.y() ) );
  if ( realm != nullptr )
  {
    _nw.crop( realm );
    _se.crop( realm );
  }
}
Area2d::Area2d( const Pos4d& p1, const Pos4d& p2 )
{
  _nw = Pos2d( std::min( p1.x(), p2.x() ), std::min( p1.y(), p2.y() ) );
  _se = Pos2d( std::max( p1.x(), p2.x() ), std::max( p1.y(), p2.y() ) );
}

Area2d& Area2d::nw( const Pos2d& p )
{
  if ( p.x() > _se.x() )
    _se.x( p.x() );
  else
    _nw.x( p.x() );
  if ( p.y() > _se.y() )
    _se.y( p.y() );
  else
    _nw.y( p.y() );
  return *this;
}
Area2d& Area2d::se( const Pos2d& p )
{
  return nw( p );
}
Area2dItr Area2d::begin() const
{
  return Area2dItr( _nw, _se );
}
Area2dItr Area2d::end() const
{
  return Area2dItr( Pos2d( _nw.x(), _se.y() ) + Vec2d( 0, 1 ), _se );
}

bool Area2d::contains( const Pos2d& other ) const
{
  return _nw <= other && _se >= other;
}

bool Area2d::intersect( const Area2d& other ) const
{
  return _nw.x() <= other._se.x() && other._nw.x() <= _se.x() && _nw.y() <= other._se.y() &&
         other._nw.y() <= _se.y();
}
}  // namespace Core
}  // namespace Pol

