#include "area.h"
#include "realms/realm.h"

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
  _nw = p1.min( p2 );
  _se = p1.max( p2 );
  if ( realm != nullptr )
  {
    _nw.crop( realm );
    _se.crop( realm );
  }
}
Area2d::Area2d( const Pos4d& p1, const Pos4d& p2 )
{
  _nw = p1.xy().min( p2.xy() );
  _se = p1.xy().max( p2.xy() );
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

bool Area2d::operator==( const Area2d& other ) const
{
  return _nw == other._nw && _se == other._se;
}
bool Area2d::operator!=( const Area2d& other ) const
{
  return !( *this == other );
}

fmt::Writer& operator<<( fmt::Writer& w, const Area2d& v )
{
  w << "( " << v.nw() << " - " << v.se() << " )";
  return w;
}

Area3d::Area3d( const Pos3d& p1, const Pos3d& p2, const Realms::Realm* realm )
    : _area( p1.xy(), p2.xy(), realm )
{
  _z_bottom = std::min( p1.z(), p2.z() );
  _z_top = std::max( p1.z(), p2.z() );
}
Area3d::Area3d( const Pos4d& p1, const Pos4d& p2 ) : _area( p1, p2 )
{
  _z_bottom = std::min( p1.z(), p2.z() );
  _z_top = std::max( p1.z(), p2.z() );
}

bool Area3d::contains( const Pos2d& other ) const
{
  return _area.contains( other );
}
bool Area3d::contains( const Pos3d& other ) const
{
  return contains( other.xy() ) && _z_bottom <= other.z() && _z_top >= other.z();
}

bool Area3d::intersect( const Area2d& other ) const
{
  return _area.intersect( other );
}
bool Area3d::intersect( const Area3d& other ) const
{
  return intersect( other._area ) && _z_bottom <= other._z_top && other._z_bottom <= _z_top;
}
}  // namespace Core
}  // namespace Pol
