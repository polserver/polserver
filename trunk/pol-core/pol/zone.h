/*
History
=======


Notes
=======

*/

#ifndef ZONE_H
#define ZONE_H

#include <string>
#include <vector>

#include "plib/realm.h"
#include "poltype.h"
#include "wrldsize.h"

const unsigned WORLD_MIN_X = 0;
const unsigned WORLD_MAX_X = 6142;

const unsigned WORLD_MIN_Y = 0;
const unsigned WORLD_MAX_Y = 4094;

inline bool VALID_WORLD_LOC( unsigned short x, unsigned short y, Realm* realm )
{
	return realm->valid(x,y,0);
}
inline bool VALID_WORLD_LOC( unsigned short x, unsigned short y, short z, Realm* realm )
{
	return realm->valid(x,y,z);
}


const unsigned ZONE_SIZE = 4;
const unsigned ZONE_SHIFT = 2;

const unsigned ZONE_X = WORLD_X / ZONE_SIZE;
const unsigned ZONE_Y = WORLD_Y / ZONE_SIZE;

void XyToZone( xcoord x, ycoord y, unsigned* zonex, unsigned* zoney );

typedef unsigned short RegionId;

class ConfigElem;

#endif
