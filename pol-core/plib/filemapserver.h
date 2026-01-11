/** @file
 *
 * @par History
 */


#ifndef PLIB_FILEMAPSERVER_H
#define PLIB_FILEMAPSERVER_H

#include "../clib/binaryfile.h"
#include "mapblock.h"
#include "mapcell.h"
#include "mapserver.h"

namespace Pol
{
namespace Plib
{
class RealmDescriptor;
}  // namespace Plib
}  // namespace Pol

namespace Pol
{
namespace Plib
{
class FileMapServer : public MapServer
{
public:
  explicit FileMapServer( const RealmDescriptor& descriptor );
  ~FileMapServer() override {}
  MAPCELL GetMapCell( unsigned short x, unsigned short y ) const override;
  size_t sizeEstimate() const override;

protected:
  mutable Clib::BinaryFile _mapfile;
  mutable int _cur_mapblock_index;
  mutable MAPBLOCK _cur_mapblock;

  // not implemented:
  FileMapServer& operator=( const FileMapServer& );
  FileMapServer( const FileMapServer& );
};
}  // namespace Plib
}  // namespace Pol

#endif
