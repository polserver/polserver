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
	unsigned int flags;
    unsigned short graphic;
    signed char z;
    char height;
    
    StaticRec( unsigned short graphic, signed char z ) : flags(tile_uoflags(graphic)), graphic(graphic), z(z), height(tileheight(graphic)) {}
    StaticRec( unsigned short graphic, signed char z, unsigned int flags, char height ) : 
        flags(flags), graphic(graphic), z(z), height(height) {}
};

typedef std::vector<StaticRec> StaticList;


void readstatics( StaticList& vec, unsigned short x, unsigned short y );
void readstatics( StaticList& vec, unsigned short x, unsigned short y, unsigned int flags );
void readallstatics( StaticList& vec, unsigned short x, unsigned short y );
bool findstatic( unsigned short x, unsigned short y, unsigned short graphic );

void readmultis( StaticList& vec, unsigned short x, unsigned short y );

#endif
