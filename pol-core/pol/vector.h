#ifndef PLIB_VECTOR_H
#define PLIB_VECTOR_H

#include "../clib/rawtypes.h"
#include "../plib/uconst.h"
#include <utility>

namespace Pol
{
namespace Realms
{
class Realm;
}
namespace Core
{
class Vec2d
{
  u16 _x;
  u16 _y;

public:
  Vec2d() = default;
  Vec2d( u16 x, u16 y );
  Vec2d( const Vec2d& other ) = default;
  Vec2d( Vec2d&& other ) = default;
  ~Vec2d() = default;
  Vec2d& operator=( const Vec2d& other ) = default;
  Vec2d& operator=( Vec2d&& other ) = default;

  bool operator==( const Vec2d& other ) const;
  bool operator!=( const Vec2d& other ) const;

  Vec2d& operator-=( u16 range );
  Vec2d& operator+=( u16 range );
  Vec2d& operator-=( const Vec2d& other );
  Vec2d& operator+=( const Vec2d& other );

  u16 getX() const;
  u16 getY() const;

  Vec2d& setX( u16 x );
  Vec2d& setY( u16 y );

  u16 pol_distance( const Vec2d& other ) const;
};
Vec2d operator-( Vec2d lhs, u16 rhs );
Vec2d operator+( Vec2d lhs, u16 rhs );
Vec2d operator-( Vec2d lhs, const Vec2d& rhs );
Vec2d operator+( Vec2d lhs, const Vec2d& rhs );

class Vec3d
{
  Vec2d _xy;
  u8 _z;

public:
  Vec3d() = default;
  Vec3d( Vec2d xy, u8 z );
  Vec3d( u16 x, u16 y, u8 z );
  ~Vec3d() = default;
  Vec3d( const Vec3d& other ) = default;
  Vec3d( Vec3d&& other ) = default;
  Vec3d& operator=( const Vec3d& other ) = default;
  Vec3d& operator=( Vec3d&& other ) = default;

  bool operator==( const Vec3d& other ) const;
  bool operator!=( const Vec3d& other ) const;

  Vec3d& operator-=( u16 range );
  Vec3d& operator+=( u16 range );
  Vec3d& operator-=( const Vec2d& other );
  Vec3d& operator+=( const Vec2d& other );
  Vec3d& operator-=( const Vec3d& other );
  Vec3d& operator+=( const Vec3d& other );

  u16 getX() const;
  u16 getY() const;
  u8 getZ() const;

  Vec3d& setX( u16 x );
  Vec3d& setY( u16 y );
  Vec3d& setZ( u8 z );

  u16 pol_distance( const Vec3d& other ) const;
};
Vec3d operator-( Vec3d lhs, u16 rhs );
Vec3d operator+( Vec3d lhs, u16 rhs );
Vec3d operator-( Vec3d lhs, const Vec2d& rhs );
Vec3d operator+( Vec3d lhs, const Vec2d& rhs );
Vec3d operator-( Vec3d lhs, const Vec3d& rhs );
Vec3d operator+( Vec3d lhs, const Vec3d& rhs );


class Vec4d
{
  Vec3d _xyz;
  Realms::Realm* _realm;

public:
  Vec4d() = default;
  Vec4d( Vec3d xyz, Realms::Realm* realm );
  Vec4d( u16 x, u16 y, u8 z, Realms::Realm* realm );
  ~Vec4d() = default;
  Vec4d( const Vec4d& other ) = default;
  Vec4d( Vec4d&& other ) = default;
  Vec4d& operator=( const Vec4d& other ) = default;
  Vec4d& operator=( Vec4d&& other ) = default;

  bool operator==( const Vec4d& other ) const;
  bool operator==( const Vec3d& other ) const;
  bool operator!=( const Vec4d& other ) const;
  bool operator!=( const Vec3d& other ) const;

  Vec4d& operator-=( u16 range );
  Vec4d& operator+=( u16 range );
  Vec4d& operator-=( const Vec2d& other );
  Vec4d& operator+=( const Vec2d& other );
  Vec4d& operator-=( const Vec3d& other );
  Vec4d& operator+=( const Vec3d& other );
  Vec4d& operator-=( const Vec4d& other );
  Vec4d& operator+=( const Vec4d& other );

  u16 getX() const;
  u16 getY() const;
  u8 getZ() const;
  Realms::Realm* getRealm() const;
  const Vec3d& getCoords() const;

  Vec4d& setX( u16 x );
  Vec4d& setY( u16 y );
  Vec4d& setZ( u8 z );
  //  Vec4d& setRealm( Realms::Realm* realm ); // removed on purpose

  void move( Plib::UFACING dir );

  bool inRange( const Vec4d& other, u16 range ) const;
  bool inRange( const Vec3d& other, u16 range ) const;

  u16 pol_distance( const Vec4d& other ) const;

private:
  void crop();
  u16 cropX( u16 x ) const;
  u16 cropY( u16 y ) const;
};
Vec4d operator-( Vec4d lhs, u16 rhs );
Vec4d operator+( Vec4d lhs, u16 rhs );
Vec4d operator-( Vec4d lhs, const Vec2d& rhs );
Vec4d operator+( Vec4d lhs, const Vec2d& rhs );
Vec4d operator-( Vec4d lhs, const Vec3d& rhs );
Vec4d operator+( Vec4d lhs, const Vec3d& rhs );
Vec4d operator-( Vec4d lhs, const Vec4d& rhs );
Vec4d operator+( Vec4d lhs, const Vec4d& rhs );

inline Vec2d::Vec2d( u16 x, u16 y ) : _x( x ), _y( y ) {}

inline u16 Vec2d::getX() const
{
  return _x;
}
inline u16 Vec2d::getY() const
{
  return _y;
}

inline Vec2d& Vec2d::setX( u16 x )
{
  _x = x;
  return *this;
}
inline Vec2d& Vec2d::setY( u16 y )
{
  _y = y;
  return *this;
}

inline Vec3d::Vec3d( u16 x, u16 y, u8 z ) : _xy( x, y ), _z( z ) {}
inline Vec3d::Vec3d( Vec2d xy, u8 z ) : _xy( std::move( xy ) ), _z( z ) {}

inline u16 Vec3d::getX() const
{
  return _xy.getX();
}
inline u16 Vec3d::getY() const
{
  return _xy.getY();
}
inline u8 Vec3d::getZ() const
{
  return _z;
}

inline Vec3d& Vec3d::setX( u16 x )
{
  _xy.setX( x );
  return *this;
}
inline Vec3d& Vec3d::setY( u16 y )
{
  _xy.setY( y );
  return *this;
}
inline Vec3d& Vec3d::setZ( u8 z )
{
  _z = z;
  return *this;
}


inline Vec4d::Vec4d( u16 x, u16 y, u8 z, Realms::Realm* realm ) : _xyz( x, y, z ), _realm( realm )
{
  crop();
}
inline Vec4d::Vec4d( Vec3d xyz, Realms::Realm* realm ) : _xyz( std::move( xyz ) ), _realm( realm )
{
  crop();
}

inline u16 Vec4d::getX() const
{
  return _xyz.getX();
}
inline u16 Vec4d::getY() const
{
  return _xyz.getY();
}
inline u8 Vec4d::getZ() const
{
  return _xyz.getZ();
}
inline Realms::Realm* Vec4d::getRealm() const
{
  return _realm;
}
inline const Vec3d& Vec4d::getCoords() const
{
  return _xyz;
}

inline Vec4d& Vec4d::setX( u16 x )
{
  _xyz.setX( cropX( x ) );
  return *this;
}
inline Vec4d& Vec4d::setY( u16 y )
{
  _xyz.setY( cropY( y ) );
  return *this;
}
inline Vec4d& Vec4d::setZ( u8 z )
{
  _xyz.setZ( z );
  return *this;
}

}  // namespace Core
}  // namespace Pol
#endif
