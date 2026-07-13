/** @file
 *
 * @par History
 */

#include "../../clib/logfacility.h"
#include "../../plib/maptile.h"
#include "testenv.h"


namespace Pol::Testing
{
// mirrors the block-index formula used by MapWriter::SetMapTile /
// MapTileServer::GetMapTile
static unsigned block_index( unsigned width, unsigned xblock, unsigned yblock )
{
  return yblock * Plib::maptile_blocks_across( width ) + xblock;
}

void maptile_geometry_test()
{
#define T_MAPTILE( f, res, msg ) UnitTest( []() { return f; }, res, msg )

  // Tokuno (1448 wide): old `1448 >> 6` truncated to 22, undercounting the
  // partial 23rd column.
  T_MAPTILE( Plib::maptile_blocks_across( 1448 ), 23u, "maptile_blocks_across(1448) == ceil" );

  // exact multiples of MAPTILE_CHUNK (e.g. Britannia's 6144) must be
  // unaffected -- same result as the old floor-shift formula.
  T_MAPTILE( Plib::maptile_blocks_across( 6144 ), 96u, "maptile_blocks_across(6144) == exact" );
  T_MAPTILE( Plib::maptile_blocks_across( 64 ), 1u, "maptile_blocks_across(64) == exact" );

  // the spec's worked collision example, in reverse: with the ceil-rounded
  // stride, the partial-column block and the start of the next row must
  // land on distinct indices.
  T_MAPTILE( block_index( 1448, 22, 0 ) != block_index( 1448, 0, 1 ), true,
             "Tokuno xblock=22,yblock=0 vs xblock=0,yblock=1 no longer collide" );

#undef T_MAPTILE
}

}  // namespace Pol::Testing
