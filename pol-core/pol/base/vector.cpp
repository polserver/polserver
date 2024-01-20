#include "vector.h"
#include "clib/clib.h"

#include <cstdlib>

namespace Pol
{
namespace Core
{
bool Vec2d::operator==( const Vec2d& other ) const
{
  return _x == other._x && _y == other._y;
}
bool Vec2d::operator!=( const Vec2d& other ) const
{
  return !( *this == other );
}

Vec2d& Vec2d::operator-=( const Vec2d& other )
{
  int x = static_cast<int>( _x ) - other._x;
  int y = static_cast<int>( _y ) - other._y;
  _x = Clib::clamp_convert<s16>( x );
  _y = Clib::clamp_convert<s16>( y );
  return *this;
}
Vec2d& Vec2d::operator+=( const Vec2d& other )
{
  int x = static_cast<int>( _x ) + other._x;
  int y = static_cast<int>( _y ) + other._y;
  _x = Clib::clamp_convert<s16>( x );
  _y = Clib::clamp_convert<s16>( y );
  return *this;
}

Vec2d operator-( Vec2d lhs, const Vec2d& rhs )
{
  lhs -= rhs;
  return lhs;
}
Vec2d operator+( Vec2d lhs, const Vec2d& rhs )
{
  lhs += rhs;
  return lhs;
}

bool Vec3d::operator==( const Vec3d& other ) const
{
  return _xy == other._xy && _z == other._z;
}
bool Vec3d::operator!=( const Vec3d& other ) const
{
  return !( *this == other );
}
bool Vec3d::operator==( const Vec2d& other ) const
{
  return _xy == other;
}
bool Vec3d::operator!=( const Vec2d& other ) const
{
  return _xy != other;
}

Vec3d& Vec3d::operator-=( const Vec3d& other )
{
  _xy -= other._xy;
  int z = static_cast<int>( _z ) - other._z;
  _z = Clib::clamp_convert<s16>( z );
  return *this;
}
Vec3d& Vec3d::operator+=( const Vec3d& other )
{
  _xy += other._xy;
  int z = static_cast<int>( _z ) + other._z;
  _z = Clib::clamp_convert<s16>( z );
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

Vec3d operator-( Vec3d lhs, const Vec3d& rhs )
{
  lhs -= rhs;
  return lhs;
}
Vec3d operator+( Vec3d lhs, const Vec3d& rhs )
{
  lhs += rhs;
  return lhs;
}
Vec3d operator-( Vec3d lhs, const Vec2d& rhs )
{
  lhs -= rhs;
  return lhs;
}
Vec3d operator+( Vec3d lhs, const Vec2d& rhs )
{
  lhs += rhs;
  return lhs;
}

}  // namespace Core
}  // namespace Pol

fmt::format_context::iterator fmt::formatter<Pol::Core::Vec2d>::format(
    const Pol::Core::Vec2d& v, fmt::format_context& ctx ) const
{
  return fmt::formatter<std::string>::format( fmt::format( "( {}, {} )", v.x(), v.y() ), ctx );
}

fmt::format_context::iterator fmt::formatter<Pol::Core::Vec3d>::format(
    const Pol::Core::Vec3d& v, fmt::format_context& ctx ) const
{
  return fmt::formatter<std::string>::format( fmt::format( "( {}, {}, {} )", v.x(), v.y(), v.z() ),
                                              ctx );
}
