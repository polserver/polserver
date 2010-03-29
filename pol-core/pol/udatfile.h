/*
History
=======


Notes
=======

*/

#ifndef UDATFILE_H
#define UDATFILE_H

#include <vector>

#ifndef CLIDATA_H
#include "clidata.h"
#endif

struct StaticRec 
{
    unsigned short graphic;
    signed char z;
    unsigned int flags;
    char height;
    
    StaticRec( unsigned short graphic, signed char z ) : graphic(graphic),z(z), flags(tile_uoflags(graphic)),height(tileheight(graphic)) {}
    StaticRec( unsigned short graphic, signed char z, unsigned int flags, char height ) : 
        graphic(graphic),z(z), flags(flags),height(height) {}
};

typedef std::vector<StaticRec> StaticList;


void readstatics( StaticList& vec, unsigned short x, unsigned short y );
void readstatics( StaticList& vec, unsigned short x, unsigned short y, unsigned int flags );
void readallstatics( StaticList& vec, unsigned short x, unsigned short y );
bool findstatic( unsigned short x, unsigned short y, unsigned short graphic );

void readmultis( StaticList& vec, unsigned short x, unsigned short y );

#endif
