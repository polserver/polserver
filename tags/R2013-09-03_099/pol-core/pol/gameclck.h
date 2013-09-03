/*
History
=======


Notes
=======

*/

#ifndef GAMECLCK_H
#define GAMECLCK_H

#include "../clib/hbmutex.h"

typedef unsigned int gameclock_t;

void start_gameclock();
void stop_gameclock();
void update_gameclock();
gameclock_t read_gameclock();

extern Mutex _gameclock_mutex;

#endif
