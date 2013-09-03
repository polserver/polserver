/*
History
=======

Notes
=======

*/

#ifndef PLIB_LANDTILESERVER_H
#define PLIB_LANDTILESERVER_H

#include "../clib/binaryfile.h"

#include "maptile.h"
#include "realmdescriptor.h"

class MapTileServer
{
public:
    explicit MapTileServer( const RealmDescriptor& descriptor );
    ~MapTileServer();

    MAPTILE_CELL GetMapTile( unsigned short x, unsigned short y );

private:
    RealmDescriptor _descriptor;

    BinaryFile _file;
    int _cur_block_index;
    MAPTILE_BLOCK _cur_block;
};

#endif
