/*
History
=======

Notes
=======

*/

#include "clib/stl_inc.h"

#include "clib/passert.h"
#include "clib/strutil.h"

#include "inmemorymapserver.h"

InMemoryMapServer::InMemoryMapServer( const RealmDescriptor& descriptor ) :
    MapServer( descriptor )
{
    unsigned n_blocks = (_descriptor.width >> MAPBLOCK_SHIFT) * (_descriptor.height >> MAPBLOCK_SHIFT);

    BinaryFile mapfile( descriptor.path( "base.dat" ), ios::in );

    _mapblocks.resize( n_blocks );

    mapfile.Read( &_mapblocks[0], n_blocks );
}

InMemoryMapServer::~InMemoryMapServer() 
{ 
}

MAPCELL InMemoryMapServer::GetMapCell( unsigned short x, unsigned short y ) const
{
    passert( x < _descriptor.width && y < _descriptor.height );

    unsigned short xblock = x >>  MAPBLOCK_SHIFT;
    unsigned short xcell  = x &   MAPBLOCK_CELLMASK;
    unsigned short yblock = y >>  MAPBLOCK_SHIFT;
    unsigned short ycell  = y &   MAPBLOCK_CELLMASK;

    long block_index = yblock * (_descriptor.width >> MAPBLOCK_SHIFT) + xblock;
    const MAPBLOCK& mapblock = _mapblocks[ block_index ];
    return mapblock.cell[xcell][ycell];
}

