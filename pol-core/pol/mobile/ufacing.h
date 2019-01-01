/** @file
 *
 * @par History
 */


#ifndef __UFACING_H
#define __UFACING_H

#include "../../plib/poltype.h"
#include "../../plib/uconst.h"

#include <array>

namespace Pol
{
namespace Core
{
class UObject;

extern Plib::UFACING away_cvt[8];
extern std::array<int, 7> adjustments;
}  // namespace Core
namespace Mobile
{
class Character;

bool DecodeFacing( const char* dir, Plib::UFACING& facing );

Plib::UFACING GetRandomFacing();

const char* FacingStr( Plib::UFACING facing );

Plib::UFACING direction_toward( const Character* src, const Core::UObject* idst );
Plib::UFACING direction_toward( const Character* src, Core::xcoord to_x, Core::ycoord to_y );
Plib::UFACING direction_toward( Core::xcoord from_x, Core::ycoord from_y, Core::xcoord to_x,
                                Core::ycoord to_y );

Plib::UFACING direction_away( const Character* src, const Core::UObject* idst );
Plib::UFACING direction_away( const Character* src, Core::xcoord from_x, Core::ycoord from_y );
}  // namespace Mobile
}  // namespace Pol

#endif  // UFACING_H
