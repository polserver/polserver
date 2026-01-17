/** @file
 *
 * @par History
 */


#ifndef PLIB_INMEMORYMAPSERVER_H
#define PLIB_INMEMORYMAPSERVER_H

#include <vector>

#include "../clib/binaryfile.h"
#include "mapblock.h"
#include "mapcell.h"
#include "mapserver.h"


namespace Pol::Plib
{
class RealmDescriptor;
}  // namespace Pol::Plib


namespace Pol::Plib
{
class InMemoryMapServer : public MapServer
{
public:
  explicit InMemoryMapServer( const RealmDescriptor& descriptor );
  ~InMemoryMapServer() override = default;

  MAPCELL GetMapCell( unsigned short x, unsigned short y ) const override;
  size_t sizeEstimate() const override;

private:
  std::vector<MAPBLOCK> _mapblocks;

  // not implemented:
  InMemoryMapServer& operator=( const InMemoryMapServer& ) = delete;
  InMemoryMapServer( const InMemoryMapServer& ) = delete;
};
}  // namespace Pol::Plib

#endif
