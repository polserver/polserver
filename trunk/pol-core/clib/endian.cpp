/*
History
=======

Notes
=======

*/

#include "stl_inc.h"

/* split this up so as to not bloat code */

#include "endian.h"
namespace Pol {
  namespace Clib {
	void Flip16( u16 *upVal )
	{
	  u8 *cp = (u8 *)upVal;
	  u8 cTmp;

	  cTmp = cp[1];
	  cp[1] = cp[0];
	  cp[0] = cTmp;
	}

	void Flip32( u32 *upVal )
	{
	  u8 *cp = (u8 *)upVal;
	  u8 cTmp;

	  cTmp = cp[3];
	  cp[3] = cp[0];
	  cp[0] = cTmp;

	  cTmp = cp[2];
	  cp[2] = cp[1];
	  cp[1] = cTmp;

	}

	u32 cu32( u32 x )
	{
	  Flip32( &x );
	  return x;
	}

	u16 cu16( u16 x )
	{
	  Flip16( &x );
	  return x;
	}
  }
}