#include "vector.h"

#include "realms/realm.h"

#include <cstdlib>
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
  int xd = std::abs( _x - other._x );
  int yd = std::abs( _y - other._y );
  if ( xd > yd )
    return static_cast<u16>( xd );
  else
    return static_cast<u16>( yd );
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

u16 Vec4d::pol_distance( const Vec4d& other ) const
{
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
