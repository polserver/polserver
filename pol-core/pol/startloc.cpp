/*
History
=======


Notes
=======

*/

#include "../clib/stl_inc.h"
#include "../clib/random.h"

#include "startloc.h"

Coordinate StartingLocation::select_coordinate() const
{
    u32 sidx = random_int( static_cast<int>(coords.size()) );

    return coords[ sidx ];
}
