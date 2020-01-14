/** @file
 *
 * @par History
 */


#ifndef STARTLOC_H
#define STARTLOC_H

#include <string>
#include <vector>
#include "../clib/rawtypes.h"

namespace Pol
{
namespace Realms
{
class Realm;
}
namespace Core
{
class Coordinate
{
public:
  u16 x;
  u16 y;
  s8 z;

  Coordinate( u16 i_x, u16 i_y, s8 i_z ) : x( i_x ), y( i_y ), z( i_z ) {}
  /*Coordinate( const Coordinate& );
  const Coordinate& operator=( const Coordinate& );*/
};

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
  std::vector<Coordinate> coords;

  Coordinate select_coordinate() const;
  size_t estimateSize() const;
};
}
}
#endif
