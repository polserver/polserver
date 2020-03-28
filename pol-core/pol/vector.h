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
  s16 _x;
  s16 _y;

public:
  Vec2d() = default;
  Vec2d( s16 x, s16 y );
  Vec2d( const Vec2d& other ) = default;
  Vec2d( Vec2d&& other ) = default;
  ~Vec2d() = default;
  Vec2d& operator=( const Vec2d& other ) = default;
  Vec2d& operator=( Vec2d&& other ) = default;

  s16 getX() const;
  s16 getY() const;
};

class Pos2d
{
  u16 _x;
  u16 _y;

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

  Pos2d& operator-=( u16 range );
  Pos2d& operator+=( u16 range );
  Pos2d& operator-=( const Pos2d& other );
  Pos2d& operator+=( const Pos2d& other );

  u16 getX() const;
  u16 getY() const;

  Pos2d& setX( u16 x );
  Pos2d& setY( u16 y );

  Vec2d relative( const Pos2d& other ) const;
  u16 pol_distance( const Pos2d& other ) const;
};
Pos2d operator-( Pos2d lhs, u16 rhs );
Pos2d operator+( Pos2d lhs, u16 rhs );
Pos2d operator-( Pos2d lhs, const Pos2d& rhs );
Pos2d operator+( Pos2d lhs, const Pos2d& rhs );

class Pos3d
{
  Pos2d _xy;
  s8 _z;

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

  Pos3d& operator-=( u16 range );
  Pos3d& operator+=( u16 range );
  Pos3d& operator-=( const Pos2d& other );
  Pos3d& operator+=( const Pos2d& other );
  Pos3d& operator-=( const Pos3d& other );
  Pos3d& operator+=( const Pos3d& other );

  u16 getX() const;
  u16 getY() const;
  s8 getZ() const;
  const Pos2d& getPos2d() const;

  Pos3d& setX( u16 x );
  Pos3d& setY( u16 y );
  Pos3d& setZ( s8 z );

  u16 pol_distance( const Pos3d& other ) const;
};
Pos3d operator-( Pos3d lhs, u16 rhs );
Pos3d operator+( Pos3d lhs, u16 rhs );
Pos3d operator-( Pos3d lhs, const Pos2d& rhs );
Pos3d operator+( Pos3d lhs, const Pos2d& rhs );
Pos3d operator-( Pos3d lhs, const Pos3d& rhs );
Pos3d operator+( Pos3d lhs, const Pos3d& rhs );


class Pos4d
{
  Pos3d _xyz;
  Realms::Realm* _realm;

public:
  Pos4d() = default;
  Pos4d( Pos3d xyz, Realms::Realm* realm );
  Pos4d( u16 x, u16 y, s8 z, Realms::Realm* realm );
  ~Pos4d() = default;
  Pos4d( const Pos4d& other ) = default;
  Pos4d( Pos4d&& other ) = default;
  Pos4d& operator=( const Pos4d& other ) = default;
  Pos4d& operator=( Pos4d&& other ) = default;

  bool operator==( const Pos4d& other ) const;
  bool operator==( const Pos3d& other ) const;
  bool operator!=( const Pos4d& other ) const;
  bool operator!=( const Pos3d& other ) const;

  Pos4d& operator-=( u16 range );
  Pos4d& operator+=( u16 range );
  Pos4d& operator-=( const Pos2d& other );
  Pos4d& operator+=( const Pos2d& other );
  Pos4d& operator-=( const Pos3d& other );
  Pos4d& operator+=( const Pos3d& other );
  Pos4d& operator-=( const Pos4d& other );
  Pos4d& operator+=( const Pos4d& other );

  u16 getX() const;
  u16 getY() const;
  s8 getZ() const;
  Realms::Realm* getRealm() const;
  const Pos3d& getPos3d() const;
  const Pos2d& getPos2d() const;

  Pos4d& setX( u16 x );
  Pos4d& setY( u16 y );
  Pos4d& setZ( s8 z );
  //  Pos4d& setRealm( Realms::Realm* realm ); // removed on purpose

  void move( Plib::UFACING dir );

  bool inRange( const Pos4d& other, u16 range ) const;
  bool inRange( const Pos3d& other, u16 range ) const;

  u16 pol_distance( const Pos4d& other ) const;

private:
  void crop();
  u16 cropX( u16 x ) const;
  u16 cropY( u16 y ) const;
};
Pos4d operator-( Pos4d lhs, u16 rhs );
Pos4d operator+( Pos4d lhs, u16 rhs );
Pos4d operator-( Pos4d lhs, const Pos2d& rhs );
Pos4d operator+( Pos4d lhs, const Pos2d& rhs );
Pos4d operator-( Pos4d lhs, const Pos3d& rhs );
Pos4d operator+( Pos4d lhs, const Pos3d& rhs );
Pos4d operator-( Pos4d lhs, const Pos4d& rhs );
Pos4d operator+( Pos4d lhs, const Pos4d& rhs );

inline Vec2d::Vec2d( s16 x, s16 y ) : _x( x ), _y( y ) {}

inline s16 Vec2d::getX() const
{
  return _x;
}
inline s16 Vec2d::getY() const
{
  return _y;
}


inline Pos2d::Pos2d( u16 x, u16 y ) : _x( x ), _y( y ) {}

inline u16 Pos2d::getX() const
{
  return _x;
}
inline u16 Pos2d::getY() const
{
  return _y;
}

inline Pos2d& Pos2d::setX( u16 x )
{
  _x = x;
  return *this;
}
inline Pos2d& Pos2d::setY( u16 y )
{
  _y = y;
  return *this;
}

inline Pos3d::Pos3d( u16 x, u16 y, s8 z ) : _xy( x, y ), _z( z ) {}
inline Pos3d::Pos3d( Pos2d xy, s8 z ) : _xy( std::move( xy ) ), _z( z ) {}

inline u16 Pos3d::getX() const
{
  return _xy.getX();
}
inline u16 Pos3d::getY() const
{
  return _xy.getY();
}
inline s8 Pos3d::getZ() const
{
  return _z;
}
const Pos2d& Pos3d::getPos2d() const
{
  return _xy;
}

inline Pos3d& Pos3d::setX( u16 x )
{
  _xy.setX( x );
  return *this;
}
inline Pos3d& Pos3d::setY( u16 y )
{
  _xy.setY( y );
  return *this;
}
inline Pos3d& Pos3d::setZ( s8 z )
{
  _z = z;
  return *this;
}


inline Pos4d::Pos4d( u16 x, u16 y, s8 z, Realms::Realm* realm ) : _xyz( x, y, z ), _realm( realm )
{
  crop();
}
inline Pos4d::Pos4d( Pos3d xyz, Realms::Realm* realm ) : _xyz( std::move( xyz ) ), _realm( realm )
{
  crop();
}

inline u16 Pos4d::getX() const
{
  return _xyz.getX();
}
inline u16 Pos4d::getY() const
{
  return _xyz.getY();
}
inline s8 Pos4d::getZ() const
{
  return _xyz.getZ();
}
inline Realms::Realm* Pos4d::getRealm() const
{
  return _realm;
}
inline const Pos3d& Pos4d::getPos3d() const
{
  return _xyz;
}
inline const Pos2d& Pos4d::getPos2d() const
{
  return _xyz.getPos2d();
}

inline Pos4d& Pos4d::setX( u16 x )
{
  _xyz.setX( cropX( x ) );
  return *this;
}
inline Pos4d& Pos4d::setY( u16 y )
{
  _xyz.setY( cropY( y ) );
  return *this;
}
inline Pos4d& Pos4d::setZ( s8 z )
{
  _xyz.setZ( z );
  return *this;
}

}  // namespace Core
}  // namespace Pol
#endif
