/** @file
 *
 * @par History
 */


#ifndef UDATFILE_H
#define UDATFILE_H

#include <vector>

#include "clidata.h"

namespace Pol
{
namespace Plib
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

// TODO: the functions below are only used for uotool or uoconvert. Consider moving to a separate
// include.
void readstatics( StaticList& vec, unsigned short x, unsigned short y );
void readstatics( StaticList& vec, unsigned short x, unsigned short y, unsigned int flags );
void readallstatics( StaticList& vec, unsigned short x, unsigned short y );
bool findstatic( unsigned short x, unsigned short y, unsigned short graphic );
}  // namespace Plib
}  // namespace Pol
#endif
