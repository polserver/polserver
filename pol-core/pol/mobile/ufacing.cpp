#include "ufacing.h"

#include "../../clib/clib.h"
#include "../../clib/random.h"
#include "../../plib/uconst.h"
#include "../mdelta.h"
#include "../uobject.h"
#include "base/position.h"
#include "charactr.h"

namespace Pol
{
namespace Core
{
// FIXME shouldn't be using PKTIN_02_FACING_MASK here
MoveDelta move_delta[8] = { { 0, -1 },   // 0 is N
                            { +1, -1 },  // 1 is NE
                            { +1, 0 },   // ...
                            { +1, +1 }, { 0, +1 }, { -1, +1 }, { -1, 0 }, { -1, -1 } };

Plib::UFACING away_cvt[8] = { Plib::FACING_S, Plib::FACING_SW, Plib::FACING_W, Plib::FACING_NW,
                              Plib::FACING_N, Plib::FACING_NE, Plib::FACING_E, Plib::FACING_SE };

std::array<int, 7> adjustments = { { 0, +1, -1, +2, -2, +3, -3 } };
}  // namespace Core
namespace Mobile
{
Plib::UFACING GetRandomFacing()
{
  int rval = Clib::random_int( 7 );
  return static_cast<Plib::UFACING>( rval );
}

const char* FacingStr( Plib::UFACING facing )
{
  switch ( facing )
  {
  case Plib::FACING_N:
    return "N";
  case Plib::FACING_S:
    return "S";
  case Plib::FACING_E:
    return "E";
  case Plib::FACING_W:
    return "W";
  case Plib::FACING_NE:
    return "NE";
  case Plib::FACING_NW:
    return "NW";
  case Plib::FACING_SE:
    return "SE";
  case Plib::FACING_SW:
    return "SW";
  }
  return "";
}

Plib::UFACING direction_toward( const Character* src, const Core::UObject* idst )
{
  using namespace Core;
  const UObject* dst = idst->toplevel_owner();
  auto f = src->pos().xy().direction_toward( dst->pos().xy() );
  return static_cast<Plib::UFACING>( f );
}

Plib::UFACING direction_toward( const Character* src, Core::xcoord to_x, Core::ycoord to_y )
{
  using namespace Core;
  auto f = src->pos().xy().direction_toward( Pos2d( to_x, to_y ) );
  return static_cast<Plib::UFACING>( f );
}

Plib::UFACING direction_toward( Core::xcoord from_x, Core::ycoord from_y, Core::xcoord to_x,
                                Core::ycoord to_y )
{
  using namespace Core;
  auto f = Pos2d( from_x, from_y ).direction_toward( Pos2d( to_x, to_y ) );
  return static_cast<Plib::UFACING>( f );
}

Plib::UFACING direction_away( const Character* src, const Core::UObject* idst )
{
  Plib::UFACING toward = direction_toward( src, idst );
  Plib::UFACING away = Core::away_cvt[static_cast<int>( toward )];
  return away;
}

Plib::UFACING direction_away( const Character* src, Core::xcoord from_x, Core::ycoord from_y )
{
  Plib::UFACING toward = direction_toward( src, from_x, from_y );
  Plib::UFACING away = Core::away_cvt[static_cast<int>( toward )];
  return away;
}

bool DecodeFacing( const char* dir, Plib::UFACING& facing )
{
  if ( stricmp( dir, "N" ) == 0 )
    facing = Plib::FACING_N;
  else if ( stricmp( dir, "S" ) == 0 )
    facing = Plib::FACING_S;
  else if ( stricmp( dir, "E" ) == 0 )
    facing = Plib::FACING_E;
  else if ( stricmp( dir, "W" ) == 0 )
    facing = Plib::FACING_W;
  else if ( stricmp( dir, "NE" ) == 0 )
    facing = Plib::FACING_NE;
  else if ( stricmp( dir, "SE" ) == 0 )
    facing = Plib::FACING_SE;
  else if ( stricmp( dir, "SW" ) == 0 )
    facing = Plib::FACING_SW;
  else if ( stricmp( dir, "NW" ) == 0 )
    facing = Plib::FACING_NW;
  else
    return false;
  return true;
}
}  // namespace Mobile
}  // namespace Pol
