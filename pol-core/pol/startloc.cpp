/** @file
 *
 * @par History
 */


#include "startloc.h"

#include "../clib/random.h"
#include "../clib/stlutil.h"

namespace Pol
{
namespace Core
{
Pos4d StartingLocation::select_coordinate() const
{
  int sidx = Clib::random_int( static_cast<int>( coords.size() - 1 ) );

  return Pos4d( coords[sidx], realm );
}

size_t StartingLocation::estimateSize() const
{
  size_t size = city.capacity() + desc.capacity() + sizeof( Realms::Realm* ) /*realm*/
                + sizeof( unsigned short )                                   /*mapid*/
                + sizeof( unsigned long )                                    /*cliloc_desc*/
                + Clib::memsize( coords );
  return size;
}
}  // namespace Core
}  // namespace Pol
