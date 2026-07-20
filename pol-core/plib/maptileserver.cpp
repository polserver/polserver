/** @file
 *
 * @par History
 */


#include "maptileserver.h"


namespace Pol::Plib
{
MapTileServer::MapTileServer( const RealmDescriptor& descriptor )
    : _descriptor( descriptor ),
      _file( _descriptor.path( "maptile.dat" ), std::ios::in ),
      _cur_block_index( -1L )
{
  _file.Read( _cur_block );
  _cur_block_index = 0;
}

MAPTILE_CELL MapTileServer::GetMapTile( unsigned short x, unsigned short y )
{
  unsigned short xcell = x & MAPTILE_CELLMASK;
  unsigned short ycell = y & MAPTILE_CELLMASK;

  // grid_width (the world zone grid, WGRID_SIZE=64, rounded up) matches the
  // maptile block stride; use the canonical maptile index.
  int block_index = static_cast<int>( maptile_index( x, y, _descriptor.width ) );
  if ( block_index != _cur_block_index )
  {
    size_t offset = block_index * sizeof _cur_block;

    _file.Seek( offset );
    _file.Read( _cur_block );

    _cur_block_index = block_index;
  }

  return _cur_block.cell[xcell][ycell];
}

size_t MapTileServer::sizeEstimate() const
{
  return sizeof( *this ) + _descriptor.sizeEstimate() + _file.sizeEstimate();
}
}  // namespace Pol::Plib
