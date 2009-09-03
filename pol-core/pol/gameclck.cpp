/*
History
=======


Notes
=======

*/

#include "clib/stl_inc.h"
#include <time.h>

#include "clib/stlutil.h"

#include "gameclck.h"
#include "gprops.h"
#include "polclock.h"

static unsigned long gameclock;

static time_t last_read;

void start_gameclock()
{
    string gameclock_str;
    if (global_properties.getprop( "gameclock", gameclock_str ))
    {
        char ch_s;
        ISTRINGSTREAM is(gameclock_str);
        is >> ch_s >> gameclock;
    }
    else
    {
        gameclock = 0;
    }

    last_read = poltime();
}

void update_gameclock()
{
    OSTRINGSTREAM os;
    os << "s" << read_gameclock();
    global_properties.setprop( "gameclock", OSTRINGSTREAM_STR(os) );
}

void stop_gameclock()
{
    update_gameclock();
}

gameclock_t read_gameclock()
{
    time_t new_last_read = poltime();
    unsigned long diff = static_cast<unsigned long>(new_last_read - last_read);
    gameclock += diff;
    last_read = new_last_read;
    return gameclock;
}
