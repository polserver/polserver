#include "vector.h"

#include "mdelta.h"
#include "realms/realm.h"

#include <algorithm>
#include <cstdlib>
#include <limits>
#include <tuple>

namespace Pol
{
namespace Core
{
bool Vec2d::operator==( const Vec2d& other ) const
{
  return std::tie( _x, _y ) == std::tie( other._x, other._y );
}
bool Vec2d::operator!=( const Vec2d& other ) const
{
  return !( *this == other );
}

u16 Vec2d::pol_distance( const Vec2d& other ) const
{
  int xd = std::abs( static_cast<int>( _x ) - other._x );
  int yd = std::abs( static_cast<int>( _y ) - other._y );
  return static_cast<u16>( std::max( xd, yd ) );
}


bool Vec3d::operator==( const Vec3d& other ) const
{
  return std::tie( _xy, _z ) == std::tie( other._xy, other._z );
}
bool Vec3d::operator!=( const Vec3d& other ) const
{
  return !( *this == other );
}

u16 Vec3d::pol_distance( const Vec3d& other ) const
{
  return _xy.pol_distance( other._xy );
}


bool Vec4d::operator==( const Vec4d& other ) const
{
  return std::tie( _xyz, _realm ) == std::tie( other._xyz, other._realm );
}
bool Vec4d::operator==( const Vec3d& other ) const
{
  return _xyz == other;
}
bool Vec4d::operator!=( const Vec4d& other ) const
{
  return !( *this == other );
}
bool Vec4d::operator!=( const Vec3d& other ) const
{
  return !( *this == other );
}

void Vec4d::move( Plib::UFACING dir )
{
  int x = _xyz.getX();
  x += Core::move_delta[dir].xmove;
  int y = _xyz.getY();
  y += Core::move_delta[dir].ymove;
  if ( x <= 0 )
    _xyz.setX( 0 );
  else
    _xyz.setX( static_cast<u16>( std::min( x, static_cast<int>( _realm->width() ) ) ) );
  if ( y <= 0 )
    _xyz.setY( 0 );
  else
    _xyz.setY( static_cast<u16>( std::min( y, static_cast<int>( _realm->height() ) ) ) );
}

u16 Vec4d::pol_distance( const Vec4d& other ) const
{
  if ( _realm != other._realm )
    return std::numeric_limits<u16>::max();
  return _xyz.pol_distance( other._xyz );
}
bool Vec4d::inRange( const Vec4d& other, u16 range ) const
{
  return _realm == other._realm && _xyz.pol_distance( other._xyz ) <= range;
}
bool Vec4d::inRange( const Vec3d& other, u16 range ) const
{
  return _xyz.pol_distance( other ) <= range;
}


}  // namespace Core
}  // namespace Pol
