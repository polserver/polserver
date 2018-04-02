/** @file
 *
 * @par History
 */


#ifndef __UFACING_H
#define __UFACING_H

#include "../poltype.h"
#include "../uconst.h"

#include <array>

namespace Pol
{
namespace Core
{
class UObject;

extern UFACING away_cvt[8];
extern std::array<int, 7> adjustments;
}
namespace Mobile
{
class Character;

bool DecodeFacing( const char* dir, Core::UFACING& facing );

Core::UFACING GetRandomFacing();

const char* FacingStr( Core::UFACING facing );

Core::UFACING direction_toward( const Character* src, const Core::UObject* idst );
Core::UFACING direction_toward( const Character* src, Core::xcoord to_x, Core::ycoord to_y );
Core::UFACING direction_toward( Core::xcoord from_x, Core::ycoord from_y, Core::xcoord to_x,
                                Core::ycoord to_y );

Core::UFACING direction_away( const Character* src, const Core::UObject* idst );
Core::UFACING direction_away( const Character* src, Core::xcoord from_x, Core::ycoord from_y );
}
}

#endif  // UFACING_H
