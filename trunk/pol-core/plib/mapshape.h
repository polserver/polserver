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
    int z;
    int height;
    unsigned long flags;
};

class MapShapeList : public vector<MapShape> {};


#endif
