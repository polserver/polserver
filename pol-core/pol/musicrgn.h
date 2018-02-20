/** @file
 *
 * @par History
 */


#ifndef MUSICRGN_H
#define MUSICRGN_H

#include <string>
#include <vector>

#include "../clib/compilerspecifics.h"
#include "region.h"
#include "zone.h"

namespace Pol
{
namespace Clib
{
class ConfigElem;
}  // namespace Clib
}  // namespace Pol

namespace Pol
{
namespace Core
{
void read_music_zones();

class MusicRegion : public Region
{
  typedef Region base;

public:
  MusicRegion( Clib::ConfigElem& elem, RegionId id );
  virtual size_t estimateSize() const POL_OVERRIDE;


  unsigned short getmidi() const;

private:
  std::vector<unsigned short> midilist_;
};
}
}
#endif
