/*
History
=======


Notes
=======

*/

#include "startloc.h"

#include "../clib/random.h"

namespace Pol {
  namespace Core {
	Coordinate StartingLocation::select_coordinate() const
	{
      int sidx = Clib::random_int( static_cast<int>( coords.size( ) -1) );

	  return coords[sidx];
	}

    size_t StartingLocation::estimateSize() const
    {
      size_t size = city.capacity()
        + desc.capacity()
        +sizeof(Plib::Realm*) /*realm*/
        + sizeof(unsigned short) /*mapid*/
        + sizeof(unsigned long) /*cliloc_desc*/
        + 3 * sizeof(Coordinate*) + coords.capacity() * sizeof( Coordinate );
      return size;
    }
  }
}