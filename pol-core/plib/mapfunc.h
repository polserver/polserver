/*
History
=======
2006/04/09 Shinigami: added uoconvert.cfg flag ShowRoofAndPlatformWarning

Notes
=======

*/

#ifndef PLIB_MAPFUNC_H
#define PLIB_MAPFUNC_H

#include "clib/rawtypes.h"

class ConfigElem;

u32 polflags_from_tileflags( unsigned short tile, u32 uoflags, bool use_no_shoot, bool LOS_through_windows );
u32 polflags_from_landtileflags( unsigned short tile, u32 lt_flags );

unsigned long readflags( ConfigElem& elem );

#endif

