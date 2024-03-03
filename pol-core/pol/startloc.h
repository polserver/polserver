/** @file
 *
 * @par History
 */


#ifndef STARTLOC_H
#define STARTLOC_H

#include "../clib/rawtypes.h"
#include "base/position.h"
#include <string>
#include <vector>

namespace Pol
{
namespace Realms
{
class Realm;
}
namespace Core
{
class StartingLocation
{
public:
  std::string city;
  std::string desc;
  Realms::Realm* realm;
  unsigned short mapid;
  unsigned long cliloc_desc;
  /* NOTE: this list will be selected from randomly.
     By placing only one coordinate in a StartingLocation,
     the system will behave as standard UO - ie, you pick
     the exact point you come into the world.
     */
  std::vector<Pos3d> coords;

  Pos4d select_coordinate() const;
  size_t estimateSize() const;
};
}  // namespace Core
}  // namespace Pol
#endif
