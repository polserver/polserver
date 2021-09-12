#include "ufacing.h"

#include "clib/clib.h"
#include "clib/random.h"
#include "plib/uconst.h"

#include "base/position.h"
#include "charactr.h"
#include "uobject.h"

namespace Pol
{
namespace Core
{
std::array<int, 7> adjustments = { { 0, +1, -1, +2, -2, +3, -3 } };
}  // namespace Core
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
}  // namespace Mobile
}  // namespace Pol
