#include "vector.h"

#include <algorithm>
#include <cstdlib>
#include <limits>

namespace Pol
{
namespace Core
{
s16 Vec2d::clip( int v )
{
  return static_cast<s16>( std::clamp( v, static_cast<int>( std::numeric_limits<s16>::min() ),
                                       static_cast<int>( std::numeric_limits<s16>::max() ) ) );
}

bool Vec2d::operator==( const Vec2d& other ) const
{
  return _x == other._x && _y == other._y;
}
bool Vec2d::operator!=( const Vec2d& other ) const
{
  return !( *this == other );
}
bool Vec2d::operator<( const Vec2d& other ) const
{
  if ( _x == other._x )
    return _y < other._y;
  return _x < other._x;
}
bool Vec2d::operator>( const Vec2d& other ) const
{
  if ( _x == other._x )
    return _y > other._y;
  return _x > other._x;
}
bool Vec2d::operator<=( const Vec2d& other ) const
{
  return *this == other || *this < other;
}
bool Vec2d::operator>=( const Vec2d& other ) const
{
  return *this == other || *this > other;
}

Vec2d& Vec2d::operator-=( const Vec2d& other )
{
  int x = static_cast<int>( _x ) - other._x;
  int y = static_cast<int>( _y ) - other._y;
  _x = clip( x );
  _y = clip( y );
  return *this;
}
Vec2d& Vec2d::operator+=( const Vec2d& other )
{
  int x = static_cast<int>( _x ) + other._x;
  int y = static_cast<int>( _y ) + other._y;
  _x = clip( x );
  _y = clip( y );
  return *this;
}

void Vec2d::update_min( const Vec2d& v )
{
  if ( v._x < _x )
    _x = v._x;
  if ( v._y < _y )
    _y = v._y;
}
void Vec2d::update_max( const Vec2d& v )
{
  if ( v._x > _x )
    _x = v._x;
  if ( v._y > _y )
    _y = v._y;
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
fmt::Writer& operator<<( fmt::Writer& w, const Vec2d& v )
{
  w << "( " << v.x() << ", " << v.y() << " )";
  return w;
}

bool Vec3d::operator==( const Vec3d& other ) const
{
  return _xy == other._xy && _z == other._z;
}
bool Vec3d::operator!=( const Vec3d& other ) const
{
  return !( *this == other );
}
bool Vec3d::operator<( const Vec3d& other ) const
{
  if ( _xy == other._xy )
    return _z < other._z;
  return _xy < other._xy;
}
bool Vec3d::operator>( const Vec3d& other ) const
{
  if ( _xy == other._xy )
    return _z > other._z;
  return _xy > other._xy;
}
bool Vec3d::operator<=( const Vec3d& other ) const
{
  return *this == other || *this < other;
}
bool Vec3d::operator>=( const Vec3d& other ) const
{
  return *this == other || *this > other;
}
bool Vec3d::operator==( const Vec2d& other ) const
{
  return _xy == other;
}
bool Vec3d::operator!=( const Vec2d& other ) const
{
  return _xy != other;
}
bool Vec3d::operator<( const Vec2d& other ) const
{
  return _xy < other;
}
bool Vec3d::operator>( const Vec2d& other ) const
{
  return _xy > other;
}
bool Vec3d::operator<=( const Vec2d& other ) const
{
  return _xy <= other;
}
bool Vec3d::operator>=( const Vec2d& other ) const
{
  return _xy >= other;
}

Vec3d& Vec3d::operator-=( const Vec3d& other )
{
  _xy -= other._xy;
  int z = static_cast<int>( _z ) - other._z;
  _z = Vec2d::clip( z );
  return *this;
}
Vec3d& Vec3d::operator+=( const Vec3d& other )
{
  _xy += other._xy;
  int z = static_cast<int>( _z ) + other._z;
  _z = Vec2d::clip( z );
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

void Vec3d::update_min( const Vec3d& v )
{
  _xy.update_min( v.xy() );
  if ( v._z < _z )
    _z = v._z;
}
void Vec3d::update_max( const Vec3d& v )
{
  _xy.update_max( v.xy() );
  if ( v._z > _z )
    _z = v._z;
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

fmt::Writer& operator<<( fmt::Writer& w, const Vec3d& v )
{
  w << "( " << v.x() << ", " << v.y() << ", " << v.z() << " )";
  return w;
}
}  // namespace Core
}  // namespace Pol
