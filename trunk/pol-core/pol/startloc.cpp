/*
History
=======


Notes
=======

*/

#include "../clib/stl_inc.h"
#include "../clib/random.h"

#include "startloc.h"
namespace Pol {
  namespace Core {
	Coordinate StartingLocation::select_coordinate() const
	{
      int sidx = Clib::random_int( static_cast<int>( coords.size( ) -1) );

	  return coords[sidx];
	}
  }
}