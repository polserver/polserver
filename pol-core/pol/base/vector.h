#ifndef POL_BASE_VECTOR_H
#define POL_BASE_VECTOR_H

#include "clib/rawtypes.h"
#include <format/format.h>
#include <utility>

namespace Pol
{
namespace Core
{
class Vec2d
{
  s16 _x = 0;
  s16 _y = 0;

public:
  Vec2d() = default;
  Vec2d( s16 x, s16 y );
  Vec2d( const Vec2d& other ) = default;
  Vec2d( Vec2d&& other ) = default;
  ~Vec2d() = default;
  Vec2d& operator=( const Vec2d& other ) = default;
  Vec2d& operator=( Vec2d&& other ) = default;

  bool operator==( const Vec2d& other ) const;
  bool operator!=( const Vec2d& other ) const;
  bool operator<( const Vec2d& other ) const;
  bool operator>( const Vec2d& other ) const;
  bool operator<=( const Vec2d& other ) const;
  bool operator>=( const Vec2d& other ) const;

  Vec2d& operator-=( const Vec2d& other );
  Vec2d& operator+=( const Vec2d& other );

  s16 x() const;
  s16 y() const;

  Vec2d& x( s16 x );
  Vec2d& y( s16 y );

  static s16 clip( int x );
};
Vec2d operator-( Vec2d lhs, const Vec2d& rhs );
Vec2d operator+( Vec2d lhs, const Vec2d& rhs );
fmt::Writer& operator<<( fmt::Writer& w, const Vec2d& v );

class Vec3d
{
  Vec2d _xy;
  s16 _z = 0;

public:
  Vec3d() = default;
  Vec3d( Vec2d xy, s16 z );
  Vec3d( s16 x, s16 y, s16 z );
  Vec3d( const Vec3d& other ) = default;
  Vec3d( Vec3d&& other ) = default;
  ~Vec3d() = default;
  Vec3d& operator=( const Vec3d& other ) = default;
  Vec3d& operator=( Vec3d&& other ) = default;

  bool operator==( const Vec3d& other ) const;
  bool operator!=( const Vec3d& other ) const;
  bool operator<( const Vec3d& other ) const;
  bool operator>( const Vec3d& other ) const;
  bool operator<=( const Vec3d& other ) const;
  bool operator>=( const Vec3d& other ) const;

  bool operator==( const Vec2d& other ) const;
  bool operator!=( const Vec2d& other ) const;
  bool operator<( const Vec2d& other ) const;
  bool operator>( const Vec2d& other ) const;
  bool operator<=( const Vec2d& other ) const;
  bool operator>=( const Vec2d& other ) const;

  Vec3d& operator-=( const Vec3d& other );
  Vec3d& operator-=( const Vec2d& other );
  Vec3d& operator+=( const Vec3d& other );
  Vec3d& operator+=( const Vec2d& other );

  s16 x() const;
  s16 y() const;
  s16 z() const;
  const Vec2d& xy() const;

  Vec3d& x( s16 x );
  Vec3d& y( s16 y );
  Vec3d& z( s16 z );
  Vec3d& xy( Vec2d xy );
};
Vec3d operator-( Vec3d lhs, const Vec3d& rhs );
Vec3d operator-( Vec3d lhs, const Vec2d& rhs );
Vec3d operator+( Vec3d lhs, const Vec3d& rhs );
Vec3d operator+( Vec3d lhs, const Vec2d& rhs );
fmt::Writer& operator<<( fmt::Writer& w, const Vec3d& v );


inline Vec2d::Vec2d( s16 x, s16 y ) : _x( x ), _y( y ) {}

inline s16 Vec2d::x() const
{
  return _x;
}
inline s16 Vec2d::y() const
{
  return _y;
}

inline Vec2d& Vec2d::x( s16 x )
{
  _x = x;
  return *this;
}
inline Vec2d& Vec2d::y( s16 y )
{
  _y = y;
  return *this;
}

inline Vec3d::Vec3d( Vec2d xy, s16 z ) : _xy( std::move( xy ) ), _z( z ) {}
inline Vec3d::Vec3d( s16 x, s16 y, s16 z ) : _xy( x, y ), _z( z ) {}

inline s16 Vec3d::x() const
{
  return _xy.x();
}
inline s16 Vec3d::y() const
{
  return _xy.y();
}
inline s16 Vec3d::z() const
{
  return _z;
}
inline const Vec2d& Vec3d::xy() const
{
  return _xy;
}

inline Vec3d& Vec3d::x( s16 x )
{
  _xy.x( x );
  return *this;
}
inline Vec3d& Vec3d::y( s16 y )
{
  _xy.y( y );
  return *this;
}
inline Vec3d& Vec3d::z( s16 z )
{
  _z = z;
  return *this;
}
inline Vec3d& Vec3d::xy( Vec2d xy )
{
  _xy = std::move( xy );
  return *this;
}
}  // namespace Core
}  // namespace Pol
#endif
