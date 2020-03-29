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

  bool operator==( const Vec2d& other ) const;
  bool operator!=( const Vec2d& other ) const;

  Vec2d& operator-=( const Vec2d& other );
  Vec2d& operator+=( const Vec2d& other );

  s16 x() const;
  s16 y() const;

  Vec2d& x( s16 x );
  Vec2d& y( s16 y );
};
Vec2d operator-( Vec2d lhs, const Vec2d& rhs );
Vec2d operator+( Vec2d lhs, const Vec2d& rhs );

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
  Pos2d& operator-=( const Vec2d& other );
  Pos2d& operator+=( const Vec2d& other );

  u16 x() const;
  u16 y() const;

  Pos2d& x( u16 x );
  Pos2d& y( u16 y );

  u16 pol_distance( const Pos2d& other ) const;
};
Pos2d operator-( Pos2d lhs, u16 rhs );
Pos2d operator+( Pos2d lhs, u16 rhs );
Pos2d operator-( Pos2d lhs, const Vec2d& rhs );
Pos2d operator+( Pos2d lhs, const Vec2d& rhs );
Vec2d operator-( const Pos2d& lhs, const Pos2d& rhs );

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
  Pos3d& operator-=( const Vec2d& other );
  Pos3d& operator+=( const Vec2d& other );

  u16 x() const;
  u16 y() const;
  s8 z() const;
  const Pos2d& pos2d() const;

  Pos3d& x( u16 x );
  Pos3d& y( u16 y );
  Pos3d& z( s8 z );

  u16 pol_distance( const Pos3d& other ) const;
};
Pos3d operator-( Pos3d lhs, u16 rhs );
Pos3d operator+( Pos3d lhs, u16 rhs );
Pos3d operator-( Pos3d lhs, const Vec2d& rhs );
Pos3d operator+( Pos3d lhs, const Vec2d& rhs );
Vec2d operator-( const Pos3d& lhs, const Pos2d& rhs );
Vec2d operator-( const Pos3d& lhs, const Pos3d& rhs );

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
  Pos4d& operator-=( const Vec2d& other );
  Pos4d& operator+=( const Vec2d& other );

  u16 x() const;
  u16 y() const;
  s8 z() const;
  Realms::Realm* realm() const;
  const Pos3d& pos3d() const;
  const Pos2d& pos2d() const;

  Pos4d& x( u16 x );
  Pos4d& y( u16 y );
  Pos4d& z( s8 z );
  //  Pos4d& realm( Realms::Realm* realm ); // removed on purpose

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
Pos4d operator-( Pos4d lhs, const Vec2d& rhs );
Pos4d operator+( Pos4d lhs, const Vec2d& rhs );
Vec2d operator-( const Pos4d& lhs, const Pos2d& rhs );
Vec2d operator-( const Pos4d& lhs, const Pos3d& rhs );
Vec2d operator-( const Pos4d& lhs, const Pos4d& rhs );

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
inline const Pos2d& Pos3d::pos2d() const
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


inline Pos4d::Pos4d( u16 x, u16 y, s8 z, Realms::Realm* realm ) : _xyz( x, y, z ), _realm( realm )
{
  crop();
}
inline Pos4d::Pos4d( Pos3d xyz, Realms::Realm* realm ) : _xyz( std::move( xyz ) ), _realm( realm )
{
  crop();
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
inline const Pos3d& Pos4d::pos3d() const
{
  return _xyz;
}
inline const Pos2d& Pos4d::pos2d() const
{
  return _xyz.pos2d();
}

inline Pos4d& Pos4d::x( u16 x )
{
  _xyz.x( cropX( x ) );
  return *this;
}
inline Pos4d& Pos4d::y( u16 y )
{
  _xyz.y( cropY( y ) );
  return *this;
}
inline Pos4d& Pos4d::z( s8 z )
{
  _xyz.z( z );
  return *this;
}

}  // namespace Core
}  // namespace Pol
#endif
