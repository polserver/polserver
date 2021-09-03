#ifndef POL_BASE_AREA_H
#define POL_BASE_AREA_H

#include "position.h"
#include <format/format.h>
#include <iterator>

namespace Pol
{
namespace Core
{
class Area2dItr
{
private:
  Pos2d _v;
  u16 _xbound;
  u16 _xstart;

public:
  Area2dItr( Pos2d v, const Pos2d& v_max );

  typedef Pos2d value_type;
  typedef std::ptrdiff_t difference_type;
  typedef const Pos2d* pointer;
  typedef const Pos2d& reference;
  typedef std::input_iterator_tag iterator_category;

  reference operator*() const;
  bool operator==( const Area2dItr& other ) const;
  bool operator!=( const Area2dItr& other ) const;
  // does someone really need itr++?
  Area2dItr& operator++();
};

class Area2d
{
private:
  Pos2d _nw;
  Pos2d _se;

public:
  Area2d() = default;
  Area2d( const Pos2d& p1, const Pos2d& p2, const Realms::Realm* realm );
  Area2d( const Pos4d& p1, const Pos4d& p2 );
  Area2d( const Area2d& other ) = default;
  Area2d( Area2d&& other ) = default;
  ~Area2d() = default;
  Area2d& operator=( const Area2d& other ) = default;
  Area2d& operator=( Area2d&& other ) = default;

  const Pos2d& nw() const;
  const Pos2d& se() const;
  // not implemented on purpose, forced min/max could give weird results
  // Area2d& nw( const Pos2d& p );
  // Area2d& se( const Pos2d& p );

  Area2dItr begin() const;
  Area2dItr end() const;

  bool contains( const Pos2d& other ) const;
  bool intersect( const Area2d& other ) const;

  bool operator==( const Area2d& other ) const;
  bool operator!=( const Area2d& other ) const;
};
fmt::Writer& operator<<( fmt::Writer& w, const Area2d& v );

class Area3d
{
private:
  Area2d _area;
  s8 _z_bottom;
  s8 _z_top;

public:
  Area3d() = default;
  Area3d( const Pos3d& p1, const Pos3d& p2, const Realms::Realm* realm );
  Area3d( const Pos4d& p1, const Pos4d& p2 );
  Area3d( const Area3d& other ) = default;
  Area3d( Area3d&& other ) = default;
  ~Area3d() = default;
  Area3d& operator=( const Area3d& other ) = default;
  Area3d& operator=( Area3d&& other ) = default;

  const Pos2d& nw() const;
  const Pos2d& se() const;
  Pos3d nw_b() const;
  Pos3d se_t() const;
  // not implemented on purpose, forced min/max could give weird results
  // Area3d& nw_b( const Pos3d& p );
  // Area3d& se_t( const Pos3d& p );

  const Area2d& area() const;

  bool contains( const Pos2d& other ) const;
  bool contains( const Pos3d& other ) const;
  bool intersect( const Area2d& other ) const;
  bool intersect( const Area3d& other ) const;

  bool operator==( const Area3d& other ) const;
  bool operator!=( const Area3d& other ) const;
};
fmt::Writer& operator<<( fmt::Writer& w, const Area3d& v );

inline Area2dItr::reference Area2dItr::operator*() const
{
  return _v;
}

inline const Pos2d& Area2d::nw() const
{
  return _nw;
}
inline const Pos2d& Area2d::se() const
{
  return _se;
}

inline const Pos2d& Area3d::nw() const
{
  return _area.nw();
}
inline const Pos2d& Area3d::se() const
{
  return _area.se();
}
inline Pos3d Area3d::nw_b() const
{
  return Pos3d( _area.nw(), _z_bottom );
}
inline Pos3d Area3d::se_t() const
{
  return Pos3d( _area.se(), _z_top );
}
inline const Area2d& Area3d::area() const
{
  return _area;
}
}  // namespace Core
}  // namespace Pol

#endif
