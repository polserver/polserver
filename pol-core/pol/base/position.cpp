#include "position.h"

#include "../realms/realm.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <limits>
#include <tuple>

namespace Pol
{
namespace Core
{
namespace
{
const std::array<Vec2d, 8> move_delta = {{{0, -1},   // 0 is N
                                          {+1, -1},  // 1 is NE
                                          {+1, 0},   // ...
                                          {+1, +1},
                                          {0, +1},
                                          {-1, +1},
                                          {-1, 0},
                                          {-1, -1}}};
const std::array<UFACING, 8> away_cvt = {FACING_S, FACING_SW, FACING_W, FACING_NW,
                                         FACING_N, FACING_NE, FACING_E, FACING_SE};

s8 clip_s8( int v )
{
  return static_cast<s8>(
      std::min( static_cast<int>( std::numeric_limits<s8>::max() ),
                std::max( static_cast<int>( std::numeric_limits<s8>::min() ), v ) ) );
}
s16 clip_s16( int v )
{
  return static_cast<s16>(
      std::min( static_cast<int>( std::numeric_limits<s16>::max() ),
                std::max( static_cast<int>( std::numeric_limits<s16>::min() ), v ) ) );
}
u16 clip_u16( int v )
{
  return static_cast<u16>(
      std::max( 0, std::min( static_cast<int>( std::numeric_limits<u16>::max() ), v ) ) );
}
}  // namespace

bool Pos2d::operator==( const Pos2d& other ) const
{
  return _x == other._x && _y == other._y;
}
bool Pos2d::operator!=( const Pos2d& other ) const
{
  return !( *this == other );
}
bool Pos2d::operator<( const Pos2d& other ) const
{
  return _x < other._x && _y < other._y;
}
bool Pos2d::operator>( const Pos2d& other ) const
{
  return _x > other._x && _y > other._y;
}
bool Pos2d::operator<=( const Pos2d& other ) const
{
  return _x <= other._x && _y <= other._y;
}
bool Pos2d::operator>=( const Pos2d& other ) const
{
  return _x >= other._x && _y >= other._y;
}

Pos2d& Pos2d::operator-=( const Vec2d& other )
{
  int x = static_cast<int>( _x ) - other.x();
  int y = static_cast<int>( _y ) - other.y();
  _x = clip_u16( x );
  _y = clip_u16( y );
  return *this;
}
Pos2d& Pos2d::operator+=( const Vec2d& other )
{
  int x = static_cast<int>( _x ) + other.x();
  int y = static_cast<int>( _y ) + other.y();
  _x = clip_u16( x );
  _y = clip_u16( y );
  return *this;
}
Vec2d operator-( const Pos2d& lhs, const Pos2d& rhs )
{
  int x = static_cast<int>( lhs.x() ) - rhs.x();
  int y = static_cast<int>( lhs.y() ) - rhs.y();
  return Vec2d( clip_s16( x ), clip_s16( y ) );
}
Pos2d operator-( Pos2d lhs, const Vec2d& rhs )
{
  lhs -= rhs;
  return lhs;
}
Pos2d operator+( Pos2d lhs, const Vec2d& rhs )
{
  lhs += rhs;
  return lhs;
}

u16 Pos2d::pol_distance( const Pos2d& other ) const
{
  int xd = std::abs( static_cast<int>( _x ) - other._x );
  int yd = std::abs( static_cast<int>( _y ) - other._y );
  return static_cast<u16>( std::max( xd, yd ) );
}

void Pos2d::crop( const Realms::Realm* realm )
{
  if ( realm == nullptr )
    return;
  if ( _x >= realm->width() )
    _x = realm->width() - 1;
  if ( _y >= realm->height() )
    _y = realm->height() - 1;
}

bool Pos2d::can_move_to( const Vec2d& displacement, const Realms::Realm* realm ) const
{
  Vec2d vec_fromorigin = this->from_origin() + displacement;

  if ( vec_fromorigin.x() < 0 && vec_fromorigin.y() < 0 )
    return false;

  if ( realm == nullptr ||
       ( vec_fromorigin.x() < realm->width() && vec_fromorigin.y() < realm->height() ) )
    return true;

  return false;
}

bool Pos3d::operator==( const Pos3d& other ) const
{
  return _xy == other._xy && _z == other._z;
}
bool Pos3d::operator!=( const Pos3d& other ) const
{
  return !( *this == other );
}
bool Pos3d::operator<( const Pos3d& other ) const
{
  return _xy < other._xy && _z < other._z;
}
bool Pos3d::operator>( const Pos3d& other ) const
{
  return _xy > other._xy && _z > other._z;
}
bool Pos3d::operator<=( const Pos3d& other ) const
{
  return _xy <= other._xy && _z <= other._z;
}
bool Pos3d::operator>=( const Pos3d& other ) const
{
  return _xy >= other._xy && _z >= other._z;
}
bool Pos3d::operator==( const Pos2d& other ) const
{
  return _xy == other;
}
bool Pos3d::operator!=( const Pos2d& other ) const
{
  return _xy != other;
}
bool Pos3d::operator<( const Pos2d& other ) const
{
  return _xy < other;
}
bool Pos3d::operator>( const Pos2d& other ) const
{
  return _xy > other;
}
bool Pos3d::operator<=( const Pos2d& other ) const
{
  return _xy <= other;
}
bool Pos3d::operator>=( const Pos2d& other ) const
{
  return _xy >= other;
}

Pos3d& Pos3d::operator-=( const Vec2d& other )
{
  _xy -= other;
  return *this;
}
Pos3d& Pos3d::operator+=( const Vec2d& other )
{
  _xy += other;
  return *this;
}
Pos3d& Pos3d::operator-=( const Vec3d& other )
{
  _xy -= other.xy();
  int z = static_cast<int>( _z ) - other.z();
  _z = clip_s8( z );
  return *this;
}
Pos3d& Pos3d::operator+=( const Vec3d& other )
{
  _xy += other.xy();
  int z = static_cast<int>( _z ) + other.z();
  _z = clip_s8( z );
  return *this;
}

Pos3d operator-( Pos3d lhs, const Vec2d& rhs )
{
  lhs -= rhs;
  return lhs;
}
Pos3d operator+( Pos3d lhs, const Vec2d& rhs )
{
  lhs += rhs;
  return lhs;
}
Pos3d operator-( Pos3d lhs, const Vec3d& rhs )
{
  lhs -= rhs;
  return lhs;
}
Pos3d operator+( Pos3d lhs, const Vec3d& rhs )
{
  lhs += rhs;
  return lhs;
}
Vec2d operator-( const Pos3d& lhs, const Pos2d& rhs )
{
  return lhs.xy() - rhs;
}
Vec2d operator-( const Pos2d& lhs, const Pos3d& rhs )
{
  return lhs - rhs.xy();
}
Vec3d operator-( const Pos3d& lhs, const Pos3d& rhs )
{
  Vec2d xy = lhs.xy() - rhs.xy();
  int z = static_cast<int>( lhs.z() ) - rhs.z();
  return Vec3d( xy, clip_s16( z ) );
}

u16 Pos3d::pol_distance( const Pos3d& other ) const
{
  return _xy.pol_distance( other._xy );
}
void Pos3d::crop( const Realms::Realm* realm )
{
  _xy.crop( realm );
}


u16 Pos4d::cropX( u16 x ) const
{
  if ( _realm != nullptr && x >= _realm->width() )
    return _realm->width() - 1;
  return x;
}
u16 Pos4d::cropY( u16 y ) const
{
  if ( _realm != nullptr && y >= _realm->height() )
    return _realm->height() - 1;
  return y;
}

bool Pos4d::operator==( const Pos4d& other ) const
{
  return _xyz == other._xyz && _realm == other._realm;
}
bool Pos4d::operator!=( const Pos4d& other ) const
{
  return !( *this == other );
}
bool Pos4d::operator<( const Pos4d& other ) const
{
  if ( _realm != other._realm )
    return false;
  return _xyz < other._xyz;
}
bool Pos4d::operator>( const Pos4d& other ) const
{
  if ( _realm != other._realm )
    return false;
  return _xyz > other._xyz;
}
bool Pos4d::operator<=( const Pos4d& other ) const
{
  if ( _realm != other._realm )
    return false;
  return _xyz <= other._xyz;
}
bool Pos4d::operator>=( const Pos4d& other ) const
{
  if ( _realm != other._realm )
    return false;
  return _xyz >= other._xyz;
}
bool Pos4d::operator==( const Pos3d& other ) const
{
  return _xyz == other;
}
bool Pos4d::operator!=( const Pos3d& other ) const
{
  return !( *this == other );
}
bool Pos4d::operator<( const Pos3d& other ) const
{
  return _xyz < other;
}
bool Pos4d::operator>( const Pos3d& other ) const
{
  return _xyz > other;
}
bool Pos4d::operator<=( const Pos3d& other ) const
{
  return _xyz <= other;
}
bool Pos4d::operator>=( const Pos3d& other ) const
{
  return _xyz >= other;
}
bool Pos4d::operator==( const Pos2d& other ) const
{
  return _xyz.xy() == other;
}
bool Pos4d::operator!=( const Pos2d& other ) const
{
  return !( *this == other );
}
bool Pos4d::operator<( const Pos2d& other ) const
{
  return _xyz.xy() < other;
}
bool Pos4d::operator>( const Pos2d& other ) const
{
  return _xyz.xy() > other;
}
bool Pos4d::operator<=( const Pos2d& other ) const
{
  return _xyz.xy() <= other;
}
bool Pos4d::operator>=( const Pos2d& other ) const
{
  return _xyz.xy() >= other;
}

Pos4d& Pos4d::operator-=( const Vec2d& other )
{
  _xyz -= other;
  _xyz.crop( _realm );
  return *this;
}
Pos4d& Pos4d::operator+=( const Vec2d& other )
{
  _xyz += other;
  _xyz.crop( _realm );
  return *this;
}
Pos4d& Pos4d::operator-=( const Vec3d& other )
{
  _xyz -= other;
  _xyz.crop( _realm );
  return *this;
}
Pos4d& Pos4d::operator+=( const Vec3d& other )
{
  _xyz += other;
  _xyz.crop( _realm );
  return *this;
}

Pos4d operator-( Pos4d lhs, const Vec2d& rhs )
{
  lhs -= rhs;
  return lhs;
}
Pos4d operator+( Pos4d lhs, const Vec2d& rhs )
{
  lhs += rhs;
  return lhs;
}
Pos4d operator-( Pos4d lhs, const Vec3d& rhs )
{
  lhs -= rhs;
  return lhs;
}
Pos4d operator+( Pos4d lhs, const Vec3d& rhs )
{
  lhs += rhs;
  return lhs;
}
Vec2d operator-( const Pos4d& lhs, const Pos2d& rhs )
{
  return lhs.xyz() - rhs;
}
Vec2d operator-( const Pos2d& lhs, const Pos4d& rhs )
{
  return lhs - rhs.xyz();
}
Vec3d operator-( const Pos4d& lhs, const Pos3d& rhs )
{
  return lhs.xyz() - rhs;
}
Vec3d operator-( const Pos3d& lhs, const Pos4d& rhs )
{
  return lhs - rhs.xyz();
}
/*Vec3d operator-( const Pos4d& lhs, const Pos4d& rhs )
{
  return lhs.xyz() - rhs.xyz();
}*/

Pos4d Pos4d::move( UFACING dir ) const
{
  return *this + move_delta[dir];
}

u16 Pos4d::pol_distance( const Pos4d& other ) const
{
  if ( _realm != other._realm )
    return std::numeric_limits<u16>::max();
  return _xyz.pol_distance( other._xyz );
}
bool Pos4d::inRange( const Pos4d& other, u16 range ) const
{
  return _realm == other._realm && _xyz.pol_distance( other._xyz ) <= range;
}
bool Pos4d::inRange( const Pos3d& other, u16 range ) const
{
  return _xyz.pol_distance( other ) <= range;
}


UFACING Pos4d::direction_toward( const Pos4d& dst ) const
{
  if ( x() < dst.x() )  // East to target
  {
    if ( y() < dst.y() )
      return FACING_SE;
    else if ( y() == dst.y() )
      return FACING_E;
    else /* y() > dst.y() */
      return FACING_NE;
  }
  else if ( x() == dst.x() )
  {
    if ( y() < dst.y() )
      return FACING_S;
    else if ( y() > dst.y() )
      return FACING_N;
  }
  else /* x() > dst.x() */  // West to target
  {
    if ( y() < dst.y() )
      return FACING_SW;
    else if ( y() == dst.y() )
      return FACING_W;
    else /* y() > dst.y() */
      return FACING_NW;
  }
  return FACING_N;
}

UFACING Pos4d::direction_away( const Pos4d& dst ) const
{
  UFACING toward = direction_toward( dst );
  return away_cvt[static_cast<int>( toward )];
}
}  // namespace Core
}  // namespace Pol
