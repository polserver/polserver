#ifndef POL_BASE_POSITION_H
#define POL_BASE_POSITION_H

#include "clib/rawtypes.h"
#include "plib/uconst.h"
#include "vector.h"
#include <format/format.h>
#include <utility>

namespace Pol
{
namespace Realms
{
class Realm;
}
namespace Core
{
/* Character Facings:
    N^
    7 0 1
    6 * 2
    5 4 3
*/
enum UFACING : u8
{
  FACING_N = 0,
  FACING_NE = 1,
  FACING_E = 2,
  FACING_SE = 3,
  FACING_S = 4,
  FACING_SW = 5,
  FACING_W = 6,
  FACING_NW = 7
};

class Pos2d
{
  u16 _x = 0;
  u16 _y = 0;

public:
  Pos2d() = default;
  Pos2d( u16 x, u16 y );
  Pos2d( const Pos2d& other ) = default;
  Pos2d( Pos2d&& other ) = default;
  ~Pos2d() = default;
  Pos2d& operator=( const Pos2d& other ) = default;
  Pos2d& operator=( Pos2d&& other ) = default;

  bool operator==( const Pos2d& other ) const;
  bool operator!=( const Pos2d& other ) const;

  Pos2d& operator-=( const Vec2d& other );
  Pos2d& operator+=( const Vec2d& other );

  u16 x() const;
  u16 y() const;

  Pos2d& x( u16 x );
  Pos2d& y( u16 y );

  Vec2d from_origin() const;
  Pos2d& move_to( UFACING dir );
  bool can_move_to( const Vec2d& displacement, const Realms::Realm* realm ) const;

  u16 pol_distance( const Pos2d& other ) const;
  bool in_range( const Pos2d& other, u16 range ) const;
  Pos2d& crop( const Realms::Realm* realm );

  UFACING direction_toward( const Pos2d& other ) const;
  UFACING direction_away( const Pos2d& other ) const;

  Pos2d min( const Pos2d& v ) const;
  Pos2d max( const Pos2d& v ) const;
};
Pos2d operator-( Pos2d lhs, const Vec2d& rhs );
Pos2d operator+( Pos2d lhs, const Vec2d& rhs );
Vec2d operator-( const Pos2d& lhs, const Pos2d& rhs );
fmt::Writer& operator<<( fmt::Writer& w, const Pos2d& v );

class Pos3d
{
  Pos2d _xy;
  s8 _z = 0;

public:
  Pos3d() = default;
  Pos3d( Pos2d xy, s8 z );
  Pos3d( u16 x, u16 y, s8 z );
  ~Pos3d() = default;
  Pos3d( const Pos3d& other ) = default;
  Pos3d( Pos3d&& other ) = default;
  Pos3d& operator=( const Pos3d& other ) = default;
  Pos3d& operator=( Pos3d&& other ) = default;

  bool operator==( const Pos3d& other ) const;
  bool operator!=( const Pos3d& other ) const;

  bool operator==( const Pos2d& other ) const;
  bool operator!=( const Pos2d& other ) const;

  Pos3d& operator-=( const Vec2d& other );
  Pos3d& operator+=( const Vec2d& other );
  Pos3d& operator-=( const Vec3d& other );
  Pos3d& operator+=( const Vec3d& other );

  u16 x() const;
  u16 y() const;
  s8 z() const;
  const Pos2d& xy() const;

  Pos3d& x( u16 x );
  Pos3d& y( u16 y );
  Pos3d& z( s8 z );
  Pos3d& xy( Pos2d xy );

  Vec3d from_origin() const;
  Pos3d& move_to( UFACING dir );
  bool can_move_to( const Vec2d& displacement, const Realms::Realm* realm ) const;

  u16 pol_distance( const Pos3d& other ) const;
  bool in_range( const Pos2d& other, u16 range ) const;
  bool in_range( const Pos3d& other, u16 range ) const;
  Pos3d& crop( const Realms::Realm* realm );
};
Pos3d operator-( Pos3d lhs, const Vec2d& rhs );
Pos3d operator+( Pos3d lhs, const Vec2d& rhs );
Pos3d operator-( Pos3d lhs, const Vec3d& rhs );
Pos3d operator+( Pos3d lhs, const Vec3d& rhs );
Vec2d operator-( const Pos3d& lhs, const Pos2d& rhs );
Vec2d operator-( const Pos2d& lhs, const Pos3d& rhs );
Vec3d operator-( const Pos3d& lhs, const Pos3d& rhs );
fmt::Writer& operator<<( fmt::Writer& w, const Pos3d& v );

class Pos4d
{
  Pos3d _xyz;
  Realms::Realm* _realm = nullptr;

public:
  Pos4d() = default;
  Pos4d( Pos2d xy, s8 z, Realms::Realm* realm );
  Pos4d( Pos3d xyz, Realms::Realm* realm );
  Pos4d( u16 x, u16 y, s8 z, Realms::Realm* realm );
  ~Pos4d() = default;
  Pos4d( const Pos4d& other ) = default;
  Pos4d( Pos4d&& other ) = default;
  Pos4d& operator=( const Pos4d& other ) = default;
  Pos4d& operator=( Pos4d&& other ) = default;

  bool operator==( const Pos4d& other ) const;
  bool operator!=( const Pos4d& other ) const;

  bool operator==( const Pos3d& other ) const;
  bool operator!=( const Pos3d& other ) const;

  bool operator==( const Pos2d& other ) const;
  bool operator!=( const Pos2d& other ) const;

  Pos4d& operator-=( const Vec2d& other );
  Pos4d& operator+=( const Vec2d& other );
  Pos4d& operator-=( const Vec3d& other );
  Pos4d& operator+=( const Vec3d& other );

  u16 x() const;
  u16 y() const;
  s8 z() const;
  Realms::Realm* realm() const;
  const Pos3d& xyz() const;
  const Pos2d& xy() const;

  Pos4d& x( u16 x );
  Pos4d& y( u16 y );
  Pos4d& z( s8 z );
  Pos4d& xy( Pos2d xy );
  Pos4d& xyz( Pos3d xyz );
  //  Pos4d& realm( Realms::Realm* realm ); // removed on purpose

  Pos4d move( UFACING dir ) const;
  Pos4d& move_to( UFACING dir );
  bool can_move_to( const Vec2d& displacement ) const;

  bool in_range( const Pos4d& other, u16 range ) const;
  bool in_range( const Pos3d& other, u16 range ) const;
  bool in_range( const Pos2d& other, u16 range ) const;

  u16 pol_distance( const Pos4d& other ) const;

private:
  u16 crop_x( u16 x ) const;
  u16 crop_y( u16 y ) const;
};
Pos4d operator-( Pos4d lhs, const Vec2d& rhs );
Pos4d operator+( Pos4d lhs, const Vec2d& rhs );
Pos4d operator-( Pos4d lhs, const Vec3d& rhs );
Pos4d operator+( Pos4d lhs, const Vec3d& rhs );
Vec2d operator-( const Pos4d& lhs, const Pos2d& rhs );
Vec2d operator-( const Pos2d& lhs, const Pos4d& rhs );
Vec3d operator-( const Pos4d& lhs, const Pos3d& rhs );
Vec3d operator-( const Pos3d& lhs, const Pos4d& rhs );
// Vec3d operator-( const Pos4d& lhs, const Pos4d& rhs ); // TODO: think if this one makes sense
fmt::Writer& operator<<( fmt::Writer& w, const Pos4d& v );

inline Pos2d::Pos2d( u16 x, u16 y ) : _x( x ), _y( y ) {}

inline u16 Pos2d::x() const
{
  return _x;
}
inline u16 Pos2d::y() const
{
  return _y;
}

inline Pos2d& Pos2d::x( u16 x )
{
  _x = x;
  return *this;
}
inline Pos2d& Pos2d::y( u16 y )
{
  _y = y;
  return *this;
}
inline Vec2d Pos2d::from_origin() const
{
  return *this - Core::Pos2d( 0, 0 );
}

inline Pos3d::Pos3d( u16 x, u16 y, s8 z ) : _xy( x, y ), _z( z ) {}
inline Pos3d::Pos3d( Pos2d xy, s8 z ) : _xy( std::move( xy ) ), _z( z ) {}

inline u16 Pos3d::x() const
{
  return _xy.x();
}
inline u16 Pos3d::y() const
{
  return _xy.y();
}
inline s8 Pos3d::z() const
{
  return _z;
}
inline const Pos2d& Pos3d::xy() const
{
  return _xy;
}

inline Pos3d& Pos3d::x( u16 x )
{
  _xy.x( x );
  return *this;
}
inline Pos3d& Pos3d::y( u16 y )
{
  _xy.y( y );
  return *this;
}
inline Pos3d& Pos3d::z( s8 z )
{
  _z = z;
  return *this;
}
inline Pos3d& Pos3d::xy( Pos2d xy )
{
  _xy = std::move( xy );
  return *this;
}
inline Vec3d Pos3d::from_origin() const
{
  return *this - Core::Pos3d( 0, 0, 0 );
}

inline Pos4d::Pos4d( u16 x, u16 y, s8 z, Realms::Realm* realm ) : _xyz( x, y, z ), _realm( realm )
{
  _xyz.crop( realm );
}
inline Pos4d::Pos4d( Pos3d xyz, Realms::Realm* realm ) : _xyz( std::move( xyz ) ), _realm( realm )
{
  _xyz.crop( realm );
}
inline Pos4d::Pos4d( Pos2d xy, s8 z, Realms::Realm* realm )
    : _xyz( std::move( xy ), z ), _realm( realm )
{
  _xyz.crop( realm );
}

inline u16 Pos4d::x() const
{
  return _xyz.x();
}
inline u16 Pos4d::y() const
{
  return _xyz.y();
}
inline s8 Pos4d::z() const
{
  return _xyz.z();
}
inline Realms::Realm* Pos4d::realm() const
{
  return _realm;
}
inline const Pos3d& Pos4d::xyz() const
{
  return _xyz;
}
inline const Pos2d& Pos4d::xy() const
{
  return _xyz.xy();
}

inline Pos4d& Pos4d::x( u16 x )
{
  _xyz.x( crop_x( x ) );
  return *this;
}
inline Pos4d& Pos4d::y( u16 y )
{
  _xyz.y( crop_y( y ) );
  return *this;
}
inline Pos4d& Pos4d::z( s8 z )
{
  _xyz.z( z );
  return *this;
}
inline Pos4d& Pos4d::xy( Pos2d xy )
{
  _xyz.xy( std::move( xy ) );
  _xyz.crop( _realm );
  return *this;
}
inline Pos4d& Pos4d::xyz( Pos3d xyz )
{
  _xyz = std::move( xyz );
  _xyz.crop( _realm );
  return *this;
}
}  // namespace Core
}  // namespace Pol
#endif
