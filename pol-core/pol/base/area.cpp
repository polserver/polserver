#include "area.h"
#include "../realms/realm.h"

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

Area2d::Area2d( const Pos2d& p1, const Pos2d& p2, Realms::Realm* realm )
{
  _pL = Pos2d( std::min( p1.x(), p2.x() ), std::min( p1.y(), p2.y() ) );
  _pH = Pos2d( std::max( p1.x(), p2.x() ), std::max( p1.y(), p2.y() ) );
  _realm = realm;
  if ( _realm != nullptr )
  {
    _pL.crop( _realm );
    _pH.crop( _realm );
  }
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

}  // namespace Core
}  // namespace Pol

