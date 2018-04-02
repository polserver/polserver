/** @file
 *
 * @par History
 */


#ifndef PLIB_INMEMORYMAPSERVER_H
#define PLIB_INMEMORYMAPSERVER_H

#include <vector>

#include "../clib/binaryfile.h"
#include "../clib/compilerspecifics.h"
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
class InMemoryMapServer : public MapServer
{
public:
  explicit InMemoryMapServer( const RealmDescriptor& descriptor );
  virtual ~InMemoryMapServer();

  virtual MAPCELL GetMapCell( unsigned short x, unsigned short y ) const POL_OVERRIDE;
  virtual size_t sizeEstimate() const POL_OVERRIDE;

private:
  std::vector<MAPBLOCK> _mapblocks;

  // not implemented:
  InMemoryMapServer& operator=( const InMemoryMapServer& );
  InMemoryMapServer( const InMemoryMapServer& );
};
}
}
#endif
