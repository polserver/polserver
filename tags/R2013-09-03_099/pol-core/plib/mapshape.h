/*
History
=======

Notes
=======

*/

#ifndef PLIB_MAPSHAPE_H
#define PLIB_MAPSHAPE_H

struct MapShape
{
    short z;
    short height;
    unsigned int flags;
};

class MapShapeList : public vector<MapShape> {};


#endif
