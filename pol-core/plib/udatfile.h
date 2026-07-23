/** @file
 *
 * @par History
 */


#ifndef UDATFILE_H
#define UDATFILE_H

#include <array>
#include <vector>

#include "clidata.h"
#include "staticblock.h"


namespace Pol::Plib
{
struct StaticRec
{
  unsigned int flags;
  unsigned short graphic;
  signed char z;
  char height;

  StaticRec( unsigned short graphic, signed char z )
      // valid constructor only for pol! :(
      : flags( tile_uoflags( graphic ) ),
        graphic( graphic ),
        z( z ),
        height( tileheight( graphic ) )
  {
  }
  StaticRec( unsigned short graphic, signed char z, unsigned int flags, char height )
      : flags( flags ), graphic( graphic ), z( z ), height( height )
  {
  }
};

using StaticList = std::vector<StaticRec>;

// One StaticList per cell of an 8x8 static block, indexed [x_offset * 8 + y_offset].
using StaticBuckets = std::array<StaticList, STATICBLOCK_CHUNK * STATICBLOCK_CHUNK>;

// The readstatics/readstatics_block/readallstatics functions that used to be declared
// here (uotool/uoconvert only, client-mul backed) are now UoClientFiles methods --
// see uoclientfiles.h. `findstatic` was declared but never defined; the live
// implementation is StaticServer::findstatic / Realm::findstatic (pol-file backed).
}  // namespace Pol::Plib

#endif
