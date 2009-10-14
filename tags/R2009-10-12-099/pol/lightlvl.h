/*
History
=======


Notes
=======

*/

#ifndef LIGHTLVL_H
#define LIGHTLVL_H

class LightRegion;
int lightlevel( unsigned short x, unsigned short y );
void SetRegionLightLevel( LightRegion* lightregion, int lightlevel );

const unsigned MIN_LIGHTLEVEL = 0;
const unsigned MAX_LIGHTLEVEL = 30;

inline bool VALID_LIGHTLEVEL( int lightlevel )
{
    return (unsigned(lightlevel) <= MAX_LIGHTLEVEL);
}

#endif
