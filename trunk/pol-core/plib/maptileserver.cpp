/*
History
=======

Notes
=======

*/

#include "clib/stl_inc.h"

#include "maptile.h"
#include "maptileserver.h"
#include "realmdescriptor.h"

MapTileServer::MapTileServer( const RealmDescriptor& descriptor ) :
    _descriptor( descriptor ),
    _file( _descriptor.path( "maptile.dat" ), ios::in ),
    _cur_block_index(-1L)
{
    _file.Read( _cur_block );   
    _cur_block_index = 0;
}

MapTileServer::~MapTileServer()
{
}

MAPTILE_CELL MapTileServer::GetMapTile( unsigned short x, unsigned short y )
{
    unsigned xblock = x >>  MAPTILE_SHIFT;
    unsigned xcell  = x &   MAPTILE_CELLMASK;
    unsigned yblock = y >>  MAPTILE_SHIFT;
    unsigned ycell  = y &   MAPTILE_CELLMASK;

    long block_index = yblock * (_descriptor.width >> MAPTILE_SHIFT) + xblock;
    if (block_index != _cur_block_index)
    {
        unsigned offset = block_index * sizeof _cur_block;

        _file.Seek( offset );
        _file.Read( _cur_block );

        _cur_block_index = block_index;
    }

    return _cur_block.cell[xcell][ycell];
}
