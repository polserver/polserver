/** @file
 *
 * @par History
 */


#ifndef PLIB_LANDTILESERVER_H
#define PLIB_LANDTILESERVER_H

#include "../clib/binaryfile.h"

#include "maptile.h"
#include "realmdescriptor.h"

namespace Pol::Plib
{
class MapTileServer
{
public:
  explicit MapTileServer( const RealmDescriptor& descriptor );
  ~MapTileServer() = default;

  MAPTILE_CELL GetMapTile( unsigned short x, unsigned short y );
  size_t sizeEstimate() const;

private:
  const RealmDescriptor _descriptor;

  Clib::BinaryFile _file;
  int _cur_block_index;
  MAPTILE_BLOCK _cur_block;
};
}  // namespace Pol::Plib

#endif
