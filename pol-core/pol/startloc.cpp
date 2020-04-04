/** @file
 *
 * @par History
 */


#include "startloc.h"

#include "../clib/random.h"

namespace Pol
{
namespace Core
{
Pos3d StartingLocation::select_coordinate() const
{
  int sidx = Clib::random_int( static_cast<int>( coords.size() - 1 ) );

  return coords[sidx];
}

size_t StartingLocation::estimateSize() const
{
  size_t size = city.capacity() + desc.capacity() + sizeof( Realms::Realm* ) /*realm*/
                + sizeof( unsigned short )                                   /*mapid*/
                + sizeof( unsigned long )                                    /*cliloc_desc*/
                + 3 * sizeof( Pos3d* ) + coords.capacity() * sizeof( Pos3d );
  return size;
}
}  // namespace Core
}  // namespace Pol
