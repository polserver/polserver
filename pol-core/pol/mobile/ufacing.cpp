#include "ufacing.h"

#include "../../clib/clib.h"
#include "../../clib/random.h"
#include "../mdelta.h"
#include "../poltype.h"
#include "../uconst.h"
#include "../uobject.h"
#include "charactr.h"

#ifdef _MSC_VER
#pragma warning( disable : 4996 )  // disable deprecation warning for stricmp
#endif

namespace Pol
{
namespace Core
{
// FIXME shouldn't be using PKTIN_02_FACING_MASK here
MoveDelta move_delta[8] = {{0, -1},   // 0 is N
                           {+1, -1},  // 1 is NE
                           {+1, 0},   // ...
                           {+1, +1}, {0, +1}, {-1, +1}, {-1, 0}, {-1, -1}};

UFACING away_cvt[8] = {FACING_S, FACING_SW, FACING_W, FACING_NW,
                       FACING_N, FACING_NE, FACING_E, FACING_SE};

std::array<int, 7> adjustments = {{0, +1, -1, +2, -2, +3, -3}};
}
namespace Mobile
{
Core::UFACING GetRandomFacing()
{
  int rval = Clib::random_int( 7 );
  return static_cast<Core::UFACING>( rval );
}

const char* FacingStr( Core::UFACING facing )
{
  switch ( facing )
  {
  case Core::FACING_N:
    return "N";
  case Core::FACING_S:
    return "S";
  case Core::FACING_E:
    return "E";
  case Core::FACING_W:
    return "W";
  case Core::FACING_NE:
    return "NE";
  case Core::FACING_NW:
    return "NW";
  case Core::FACING_SE:
    return "SE";
  case Core::FACING_SW:
    return "SW";
  }
  return "";
}

Core::UFACING direction_toward( const Character* src, const Core::UObject* idst )
{
  using namespace Core;
  const UObject* dst = idst->toplevel_owner();
  if ( src->x < dst->x )  // East to target
  {
    if ( src->y < dst->y )
      return FACING_SE;
    else if ( src->y == dst->y )
      return FACING_E;
    else /* src->y > dst->y */
      return FACING_NE;
  }
  else if ( src->x == dst->x )
  {
    if ( src->y < dst->y )
      return FACING_S;
    else if ( src->y > dst->y )
      return FACING_N;
  }
  else /* src->x > dst->x */  // West to target
  {
    if ( src->y < dst->y )
      return FACING_SW;
    else if ( src->y == dst->y )
      return FACING_W;
    else /* src->y > dst->y */
      return FACING_NW;
  }
  return FACING_N;
}

Core::UFACING direction_toward( const Character* src, Core::xcoord to_x, Core::ycoord to_y )
{
  using namespace Core;
  if ( src->x < to_x )  // East to target
  {
    if ( src->y < to_y )
      return FACING_SE;
    else if ( src->y == to_y )
      return FACING_E;
    else /* src->y > dst->y */
      return FACING_NE;
  }
  else if ( src->x == to_x )
  {
    if ( src->y < to_y )
      return FACING_S;
    else if ( src->y > to_y )
      return FACING_N;
  }
  else /* src->x > dst->x */  // West to target
  {
    if ( src->y < to_y )
      return FACING_SW;
    else if ( src->y == to_y )
      return FACING_W;
    else /* src->y > dst->y */
      return FACING_NW;
  }
  return FACING_N;
}

Core::UFACING direction_toward( Core::xcoord from_x, Core::ycoord from_y, Core::xcoord to_x,
                                Core::ycoord to_y )
{
  using namespace Core;
  if ( from_x < to_x )  // East to target
  {
    if ( from_y < to_y )
      return FACING_SE;
    else if ( from_y == to_y )
      return FACING_E;
    else /* from_y > to_y */
      return FACING_NE;
  }
  else if ( from_x == to_x )
  {
    if ( from_y < to_y )
      return FACING_S;
    else if ( from_y > to_y )
      return FACING_N;
  }
  else /* from_x > to_x */  // West to target
  {
    if ( from_y < to_y )
      return FACING_SW;
    else if ( from_y == to_y )
      return FACING_W;
    else /* from_y > to_y */
      return FACING_NW;
  }
  return FACING_N;
}

Core::UFACING direction_away( const Character* src, const Core::UObject* idst )
{
  Core::UFACING toward = direction_toward( src, idst );
  Core::UFACING away = Core::away_cvt[static_cast<int>( toward )];
  return away;
}

Core::UFACING direction_away( const Character* src, Core::xcoord from_x, Core::ycoord from_y )
{
  Core::UFACING toward = direction_toward( src, from_x, from_y );
  Core::UFACING away = Core::away_cvt[static_cast<int>( toward )];
  return away;
}

bool DecodeFacing( const char* dir, Core::UFACING& facing )
{
  if ( stricmp( dir, "N" ) == 0 )
    facing = Core::FACING_N;
  else if ( stricmp( dir, "S" ) == 0 )
    facing = Core::FACING_S;
  else if ( stricmp( dir, "E" ) == 0 )
    facing = Core::FACING_E;
  else if ( stricmp( dir, "W" ) == 0 )
    facing = Core::FACING_W;
  else if ( stricmp( dir, "NE" ) == 0 )
    facing = Core::FACING_NE;
  else if ( stricmp( dir, "SE" ) == 0 )
    facing = Core::FACING_SE;
  else if ( stricmp( dir, "SW" ) == 0 )
    facing = Core::FACING_SW;
  else if ( stricmp( dir, "NW" ) == 0 )
    facing = Core::FACING_NW;
  else
    return false;
  return true;
}
}
}
