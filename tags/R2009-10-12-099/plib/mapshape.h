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
    unsigned long flags;
};

class MapShapeList : public vector<MapShape> {};


#endif
