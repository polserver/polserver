/** @file
 *
 * @par History
 */


#ifndef __UFACING_H
#define __UFACING_H

#include "plib/poltype.h"

#include "base/position.h"

#include <array>

namespace Pol
{
namespace Core
{
class UObject;

extern std::array<int, 7> adjustments;
}  // namespace Core
namespace Mobile
{
class Character;

bool DecodeFacing( const char* dir, Core::UFACING& facing );

Core::UFACING GetRandomFacing();

const char* FacingStr( Core::UFACING facing );

}  // namespace Mobile
}  // namespace Pol

#endif  // UFACING_H
