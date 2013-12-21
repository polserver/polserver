/*
History
=======


Notes
=======

*/

#include "../clib/stl_inc.h"

#ifdef _MSC_VER
#	pragma warning( disable: 4786 )
#endif

#include "../clib/cfgelem.h"
#include "../clib/rawtypes.h"
#include "../clib/stlutil.h"

#include "poltype.h"
#include "zone.h"
namespace Pol {
  namespace Core {
	void XyToZone( xcoord x, ycoord y, unsigned* zonex, unsigned* zoney )
	{
	  *zonex = x >> ZONE_SHIFT;
	  *zoney = y >> ZONE_SHIFT;
	}
  }
}