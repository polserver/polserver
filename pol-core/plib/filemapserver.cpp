/*
History
=======

Notes
=======

*/

#include "../clib/stl_inc.h"

#include "../clib/passert.h"

#include "filemapserver.h"

FileMapServer::FileMapServer( const RealmDescriptor& descriptor ) :
    MapServer( descriptor ),
    _mapfile(),
    _cur_mapblock_index(-1L)
{
    string filename = _descriptor.path( "base.dat" );

    _mapfile.Open( filename, ios::in );

    _mapfile.Read( _cur_mapblock );
    _cur_mapblock_index = 0;
}

MAPCELL FileMapServer::GetMapCell( unsigned short x, unsigned short y ) const
{
    passert( x < _descriptor.width && y < _descriptor.height );

    unsigned short xblock = x >>  MAPBLOCK_SHIFT;
    unsigned short xcell  = x &   MAPBLOCK_CELLMASK;
    unsigned short yblock = y >>  MAPBLOCK_SHIFT;
    unsigned short ycell  = y &   MAPBLOCK_CELLMASK;

    int block_index = yblock * (_descriptor.width >> MAPBLOCK_SHIFT) + xblock;
    if (block_index != _cur_mapblock_index)
    {
        // read the existing block in
        fstream::pos_type offset = block_index * sizeof(_cur_mapblock);
        _mapfile.Seek( offset );
        _mapfile.Read( _cur_mapblock );

        _cur_mapblock_index = block_index;
    }

    return _cur_mapblock.cell[xcell][ycell];
}
