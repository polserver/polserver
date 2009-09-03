/*
History
=======


Notes
=======

*/

#ifndef POLSTATS_H
#define POLSTATS_H
#include "clib/rawtypes.h"

class PolStats
{
public:
    u64 bytes_received;
    u64 bytes_sent;
};

extern PolStats polstats;
//extern PolStats auxstats; (Not yet... -- Nando)
//extern PolStats webstats;

#endif
