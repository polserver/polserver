/*
History
=======


Notes
=======

*/

#ifndef ZONE_H
#define ZONE_H

#include "../plib/realm.h"
#include "poltype.h"
#include "wrldsize.h"

#include <string>
#include <vector>

namespace Pol {
  namespace Core {
	const unsigned ZONE_SIZE = 4;
	const unsigned ZONE_SHIFT = 2;

	void XyToZone( xcoord x, ycoord y, unsigned* zonex, unsigned* zoney );

	typedef unsigned short RegionId;
  }
}
#endif
