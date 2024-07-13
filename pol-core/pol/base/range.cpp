#include "range.h"
#include "realms/realm.h"

#include <algorithm>
#include <tuple>

namespace Pol
{
namespace Core
{
Range2dItr::Range2dItr( Pos2d v, const Pos2d& v_max )
    : _v( std::move( v ) ), _xbound( v_max.x() ), _xstart( _v.x() ){};

bool Range2dItr::operator==( const Range2dItr& other ) const
{
  return _v == other._v;
}
bool Range2dItr::operator!=( const Range2dItr& other ) const
{
  return !( *this == other );
}
Range2dItr& Range2dItr::operator++()
{
  if ( _v.x() < _xbound )
    _v += Vec2d( 1, 0 );
  else
  {
    _v.x( _xstart );
    _v += Vec2d( 0, 1 );
  }
  return *this;
}
Range2dItr& Range2dItr::operator--()
{
  if ( _v.x() > _xstart )
    _v -= Vec2d( 1, 0 );
  else
  {
    _v.x( _xbound );
    _v -= Vec2d( 0, 1 );
  }
  return *this;
}

Range2d::Range2d( const Pos2d& p1, const Pos2d& p2, const Realms::Realm* realm )
{
  _nw = p1.min( p2 );
  _se = p1.max( p2 );
  if ( realm != nullptr )
  {
    _nw.crop( realm );
    _se.crop( realm );
  }
}
Range2d::Range2d( const Pos4d& p1, const Pos4d& p2 )
{
  _nw = p1.xy().min( p2.xy() );
  _se = p1.xy().max( p2.xy() );
}
Range2d::Range2d( const Pos4d& p1, u8 radius )
{
  const Vec2d r = Vec2d( radius, radius );
  _nw = ( p1 - r ).xy();
  _se = ( p1 + r ).xy();
}

Range2dItr Range2d::begin() const
{
  return Range2dItr( _nw, _se );
}
Range2dItr Range2d::end() const
{
  return Range2dItr( Pos2d( _nw.x(), _se.y() ) + Vec2d( 0, 1 ), _se );
}

bool Range2d::contains( const Pos2d& other ) const
{
  return _nw.x() <= other.x() && _nw.y() <= other.y() && other.x() <= _se.x() &&
         other.y() <= _se.y();
}

bool Range2d::intersect( const Range2d& other ) const
{
  return _nw.x() <= other._se.x() && other._nw.x() <= _se.x() && _nw.y() <= other._se.y() &&
         other._nw.y() <= _se.y();
}

bool Range2d::operator==( const Range2d& other ) const
{
  return _nw == other._nw && _se == other._se;
}
bool Range2d::operator!=( const Range2d& other ) const
{
  return !( *this == other );
}

Range3d::Range3d( const Pos3d& p1, const Pos3d& p2, const Realms::Realm* realm )
    : _range( p1.xy(), p2.xy(), realm )
{
  std::tie( _z_bottom, _z_top ) = std::minmax( p1.z(), p2.z() );
}
Range3d::Range3d( const Pos4d& p1, const Pos4d& p2 ) : _range( p1, p2 )
{
  std::tie( _z_bottom, _z_top ) = std::minmax( p1.z(), p2.z() );
}

bool Range3d::contains( const Pos2d& other ) const
{
  return _range.contains( other );
}
bool Range3d::contains( const Pos3d& other ) const
{
  return contains( other.xy() ) && _z_bottom <= other.z() && _z_top >= other.z();
}

bool Range3d::intersect( const Range2d& other ) const
{
  return _range.intersect( other );
}
bool Range3d::intersect( const Range3d& other ) const
{
  return intersect( other._range ) && _z_bottom <= other._z_top && other._z_bottom <= _z_top;
}

bool Range3d::operator==( const Range3d& other ) const
{
  return _range == other._range && _z_bottom == other._z_bottom && _z_top == other._z_top;
}
bool Range3d::operator!=( const Range3d& other ) const
{
  return !( *this == other );
}
}  // namespace Core
}  // namespace Pol

fmt::format_context::iterator fmt::formatter<Pol::Core::Range2d>::format(
    const Pol::Core::Range2d& r, fmt::format_context& ctx ) const
{
  return fmt::formatter<std::string>::format( fmt::format( "( {} - {} )", r.nw(), r.se() ), ctx );
}

fmt::format_context::iterator fmt::formatter<Pol::Core::Range3d>::format(
    const Pol::Core::Range3d& r, fmt::format_context& ctx ) const
{
  return fmt::formatter<std::string>::format( fmt::format( "( {} - {} )", r.nw_b(), r.se_t() ),
                                              ctx );
}
