/** @file
 *
 * @par History
 */


#include "filemapserver.h"

#include <string>

#include "../clib/passert.h"


namespace Pol::Plib
{
FileMapServer::FileMapServer( const RealmDescriptor& descriptor )
    : MapServer( descriptor ), _mapfile(), _cur_mapblock_index( -1L )
{
  std::string filename = _descriptor.path( "base.dat" );

  _mapfile.Open( filename, std::ios::in );

  _mapfile.Read( _cur_mapblock );
  _cur_mapblock_index = 0;
}

MAPCELL FileMapServer::GetMapCell( unsigned short x, unsigned short y ) const
{
  passert( x < _descriptor.width && y < _descriptor.height );

  unsigned short xblock = x >> MAPBLOCK_SHIFT;
  unsigned short xcell = x & MAPBLOCK_CELLMASK;
  unsigned short yblock = y >> MAPBLOCK_SHIFT;
  unsigned short ycell = y & MAPBLOCK_CELLMASK;

  int block_index = yblock * ( _descriptor.width >> MAPBLOCK_SHIFT ) + xblock;
  if ( block_index != _cur_mapblock_index )
  {
    // read the existing block in
    std::fstream::pos_type offset = block_index * sizeof( _cur_mapblock );
    _mapfile.Seek( offset );
    _mapfile.Read( _cur_mapblock );

    _cur_mapblock_index = block_index;
  }

  return _cur_mapblock.cell[xcell][ycell];
}

size_t FileMapServer::sizeEstimate() const
{
  return sizeof( *this ) + MapServer::sizeEstimate() + _mapfile.sizeEstimate();
}
}  // namespace Pol::Plib
