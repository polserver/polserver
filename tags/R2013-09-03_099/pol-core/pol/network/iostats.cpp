/*
History
=======


Notes
=======

*/

#include "../../clib/stl_inc.h"
#include <string.h>

#include "iostats.h"

IOStats::IOStats()
{
    memset( &sent, 0, sizeof sent );
    memset( &received, 0, sizeof received );
}

IOStats iostats;
IOStats queuedmode_iostats;
