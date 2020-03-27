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

Vec2d& Vec2d::operator-=( u16 range )
{
  int x = static_cast<int>( _x ) - range;
  int y = static_cast<int>( _y ) - range;
  _x = static_cast<u16>( std::max( 0, x ) );
  _y = static_cast<u16>( std::max( 0, y ) );
  return *this;
}
Vec2d& Vec2d::operator+=( u16 range )
{
  int x = static_cast<int>( _x ) + range;
  int y = static_cast<int>( _y ) + range;
  _x = static_cast<u16>( std::min( static_cast<int>( std::numeric_limits<u16>::max() ), x ) );
  _y = static_cast<u16>( std::min( static_cast<int>( std::numeric_limits<u16>::max() ), y ) );
  return *this;
}
Vec2d& Vec2d::operator-=( const Vec2d& other )
{
  int x = static_cast<int>( _x ) - other._x;
  int y = static_cast<int>( _y ) - other._y;
  _x = static_cast<u16>( std::max( 0, x ) );
  _y = static_cast<u16>( std::max( 0, y ) );
  return *this;
}
Vec2d& Vec2d::operator+=( const Vec2d& other )
{
  int x = static_cast<int>( _x ) + other._x;
  int y = static_cast<int>( _y ) + other._y;
  _x = static_cast<u16>( std::min( static_cast<int>( std::numeric_limits<u16>::max() ), x ) );
  _y = static_cast<u16>( std::min( static_cast<int>( std::numeric_limits<u16>::max() ), y ) );
  return *this;
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

Vec3d& Vec3d::operator-=( u16 range )
{
  _xy -= range;
  return *this;
}
Vec3d& Vec3d::operator+=( u16 range )
{
  _xy += range;
  return *this;
}
Vec3d& Vec3d::operator-=( const Vec2d& other )
{
  _xy -= other;
  return *this;
}
Vec3d& Vec3d::operator+=( const Vec2d& other )
{
  _xy += other;
  return *this;
}
Vec3d& Vec3d::operator-=( const Vec3d& other )
{
  _xy -= other._xy;
  int z = static_cast<int>( _z ) - other._z;
  _z = static_cast<s8>( std::max( static_cast<int>( std::numeric_limits<s8>::min() ), z ) );
  return *this;
}
Vec3d& Vec3d::operator+=( const Vec3d& other )
{
  _xy += other._xy;
  int z = static_cast<int>( _z ) + other._z;
  _z = static_cast<s8>( std::min( static_cast<int>( std::numeric_limits<s8>::max() ), z ) );
  return *this;
}

u16 Vec3d::pol_distance( const Vec3d& other ) const
{
  return _xy.pol_distance( other._xy );
}


void Vec4d::crop()
{
  if ( getX() >= _realm->width() )
    setX( _realm->width() - 1 );
  if ( getY() >= _realm->height() )
    setY( _realm->height() - 1 );
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

Vec4d& Vec4d::operator-=( u16 range )
{
  _xyz -= range;
  return *this;
}
Vec4d& Vec4d::operator+=( u16 range )
{
  _xyz += range;
  crop();
  return *this;
}

Vec4d& Vec4d::operator-=( const Vec2d& other )
{
  _xyz -= other;
  return *this;
}
Vec4d& Vec4d::operator+=( const Vec2d& other )
{
  _xyz += other;
  return *this;
}
Vec4d& Vec4d::operator-=( const Vec3d& other )
{
  _xyz -= other;
  return *this;
}
Vec4d& Vec4d::operator+=( const Vec3d& other )
{
  _xyz += other;
  return *this;
}
Vec4d& Vec4d::operator-=( const Vec4d& other )
{
  _xyz -= other._xyz;
  return *this;
}
Vec4d& Vec4d::operator+=( const Vec4d& other )
{
  _xyz += other._xyz;
  return *this;
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
    _xyz.setX( static_cast<u16>( std::min( x, static_cast<int>( _realm->width() ) - 1 ) ) );
  if ( y <= 0 )
    _xyz.setY( 0 );
  else
    _xyz.setY( static_cast<u16>( std::min( y, static_cast<int>( _realm->height() ) - 1 ) ) );
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
