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

// TODO: the functions below are only used for uotool or uoconvert. Consider moving to a separate
// include.
void readstatics( StaticList& vec, unsigned short x, unsigned short y );
void readstatics( StaticList& vec, unsigned short x, unsigned short y, unsigned int flags );
// Bucket a whole static block by cell in one pass: equivalent to calling the flags
// overload of readstatics() for all 64 tiles of the block containing (x,y), but scans
// the block's records once instead of 64 times. Buckets are cleared first (capacity
// retained); records land in storage order, which callers rely on.
void readstatics_block( StaticBuckets& buckets, unsigned short x, unsigned short y,
                        unsigned int flags );
void readallstatics( StaticList& vec, unsigned short x, unsigned short y );
bool findstatic( unsigned short x, unsigned short y, unsigned short graphic );
}  // namespace Pol::Plib

#endif
