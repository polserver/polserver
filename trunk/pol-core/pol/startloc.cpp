/*
History
=======


Notes
=======

*/

#include "clib/stl_inc.h"
#include "clib/random.h"

#include "startloc.h"

Coordinate StartingLocation::select_coordinate() const
{
    int sidx = random_int( coords.size() );

    return coords[ sidx ];
}
