#include "area.h"
#include "../realms/realm.h"

#include <algorithm>

namespace Pol
{
namespace Core
{
Area2dItr::Area2dItr( Pos2d v, const Pos2d& v_max )
    : _v( std::move( v ) ), _xbound( v_max.x() ), _xstart( _v.x() ){};

bool Area2dItr::operator==( const Area2dItr& other ) const
{
  return _v == other._v;
}
bool Area2dItr::operator!=( const Area2dItr& other ) const
{
  return !( *this == other );
}
Area2dItr& Area2dItr::operator++()
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

Area2d::Area2d( const Pos2d& p1, const Pos2d& p2, const Realms::Realm* realm )
{
  _pL = Pos2d( std::min( p1.x(), p2.x() ), std::min( p1.y(), p2.y() ) );
  _pH = Pos2d( std::max( p1.x(), p2.x() ), std::max( p1.y(), p2.y() ) );
  if ( realm != nullptr )
  {
    _pL.crop( realm );
    _pH.crop( realm );
  }
}
Area2d::Area2d( const Pos4d& p1, const Pos4d& p2 )
{
  _pL = Pos2d( std::min( p1.x(), p2.x() ), std::min( p1.y(), p2.y() ) );
  _pH = Pos2d( std::max( p1.x(), p2.x() ), std::max( p1.y(), p2.y() ) );
}

Area2d& Area2d::posL( const Pos2d& p )
{
  if ( p.x() > _pH.x() )
    _pH.x( p.x() );
  else
    _pL.x( p.x() );
  if ( p.y() > _pH.y() )
    _pH.y( p.y() );
  else
    _pL.y( p.y() );
  return *this;
}
Area2d& Area2d::posH( const Pos2d& p )
{
  return posL( p );
}
Area2dItr Area2d::begin() const
{
  return Area2dItr( _pL, _pH );
}
Area2dItr Area2d::end() const
{
  return Area2dItr( Pos2d( _pL.x(), _pH.y() ) + Vec2d( 0, 1 ), _pH );
}

bool Area2d::contains( const Pos2d& other ) const
{
  return _pL <= other && _pH >= other;
}

bool Area2d::intersect( const Area2d& other ) const
{
  return _pL.x() <= other._pH.x() && other._pL.x() <= _pH.x() && _pL.y() <= other._pH.y() &&
         other._pL.y() <= _pH.y();
}
}  // namespace Core
}  // namespace Pol

