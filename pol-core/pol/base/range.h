#pragma once

#include "position.h"
#include <fmt/format.h>

#include <iterator>

namespace Pol::Core
{
class Range2dItr
{
private:
  Pos2d _v;
  u16 _xbound;
  u16 _xstart;

public:
  Range2dItr() = default;
  Range2dItr( Pos2d v, const Pos2d& v_max );

  using value_type = Pos2d;
  using difference_type = std::ptrdiff_t;
  using pointer = const Pos2d*;
  using reference = const Pos2d&;
  using iterator_category = std::input_iterator_tag;

  reference operator*() const;
  bool operator==( const Range2dItr& other ) const;
  bool operator!=( const Range2dItr& other ) const;
  // does someone really need itr++?
  Range2dItr& operator++();
  Range2dItr& operator--();
};

class Range2d
{
private:
  Pos2d _nw;
  Pos2d _se;

public:
  Range2d() = default;
  Range2d( const Pos2d& p1, const Pos2d& p2, const Realms::Realm* realm );
  Range2d( const Pos4d& p1, const Pos4d& p2 );
  Range2d( const Pos4d& p1, u8 radius );
  Range2d( const Range2d& other ) = default;
  Range2d( Range2d&& other ) = default;
  ~Range2d() = default;
  Range2d& operator=( const Range2d& other ) = default;
  Range2d& operator=( Range2d&& other ) = default;

  const Pos2d& nw() const;
  const Pos2d& se() const;
  // not implemented on purpose, forced min/max could give weird results
  // Range2d& nw( const Pos2d& p );
  // Range2d& se( const Pos2d& p );

  Range2dItr begin() const;
  Range2dItr end() const;

  bool contains( const Pos2d& other ) const;
  bool intersect( const Range2d& other ) const;

  bool operator==( const Range2d& other ) const;
  bool operator!=( const Range2d& other ) const;
};

class Range3d
{
private:
  Range2d _range;
  s8 _z_bottom;
  s8 _z_top;

public:
  Range3d() = default;
  Range3d( const Pos3d& p1, const Pos3d& p2, const Realms::Realm* realm );
  Range3d( const Pos4d& p1, const Pos4d& p2 );
  Range3d( const Range3d& other ) = default;
  Range3d( Range3d&& other ) = default;
  ~Range3d() = default;
  Range3d& operator=( const Range3d& other ) = default;
  Range3d& operator=( Range3d&& other ) = default;

  const Pos2d& nw() const;
  const Pos2d& se() const;
  Pos3d nw_b() const;
  Pos3d se_t() const;
  // not implemented on purpose, forced min/max could give weird results
  // Range3d& nw_b( const Pos3d& p );
  // Range3d& se_t( const Pos3d& p );

  const Range2d& range() const;

  bool contains( const Pos2d& other ) const;
  bool contains( const Pos3d& other ) const;
  bool intersect( const Range2d& other ) const;
  bool intersect( const Range3d& other ) const;

  bool operator==( const Range3d& other ) const;
  bool operator!=( const Range3d& other ) const;
};

inline Range2dItr::reference Range2dItr::operator*() const
{
  return _v;
}

inline const Pos2d& Range2d::nw() const
{
  return _nw;
}
inline const Pos2d& Range2d::se() const
{
  return _se;
}

inline const Pos2d& Range3d::nw() const
{
  return _range.nw();
}
inline const Pos2d& Range3d::se() const
{
  return _range.se();
}
inline Pos3d Range3d::nw_b() const
{
  return Pos3d( _range.nw(), _z_bottom );
}
inline Pos3d Range3d::se_t() const
{
  return Pos3d( _range.se(), _z_top );
}
inline const Range2d& Range3d::range() const
{
  return _range;
}
}  // namespace Pol::Core

// derive from std::string formatter to support eg padding
template <>
struct fmt::formatter<Pol::Core::Range2d> : fmt::formatter<std::string>
{
  fmt::format_context::iterator format( const Pol::Core::Range2d& r,
                                        fmt::format_context& ctx ) const;
};
template <>
struct fmt::formatter<Pol::Core::Range3d> : fmt::formatter<std::string>
{
  fmt::format_context::iterator format( const Pol::Core::Range3d& r,
                                        fmt::format_context& ctx ) const;
};
