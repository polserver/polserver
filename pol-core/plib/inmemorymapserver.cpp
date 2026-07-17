/** @file
 *
 * @par History
 */

#include "inmemorymapserver.h"

#include "../clib/binaryfile.h"
#include "../clib/passert.h"
#include "../clib/stlutil.h"
#include "mapblock.h"


namespace Pol::Plib
{
InMemoryMapServer::InMemoryMapServer( const RealmDescriptor& descriptor ) : MapServer( descriptor )
{
  unsigned n_blocks =
      ( _descriptor.width >> MAPBLOCK_SHIFT ) * ( _descriptor.height >> MAPBLOCK_SHIFT );

  Clib::BinaryFile mapfile( descriptor.path( "base.dat" ), std::ios::in );

  _mapblocks.resize( n_blocks );

  mapfile.Read( &_mapblocks[0], n_blocks );
}

MAPCELL InMemoryMapServer::GetMapCell( unsigned short x, unsigned short y ) const
{
  passert( x < _descriptor.width && y < _descriptor.height );

  unsigned short xcell = x & MAPBLOCK_CELLMASK;
  unsigned short ycell = y & MAPBLOCK_CELLMASK;

  const MAPBLOCK& mapblock = _mapblocks[realm_block_index( x, y, _descriptor.width )];
  return mapblock.cell[xcell][ycell];
}

size_t InMemoryMapServer::sizeEstimate() const
{
  return sizeof( *this ) + MapServer::sizeEstimate() + Clib::memsize( _mapblocks );
}
}  // namespace Pol::Plib
