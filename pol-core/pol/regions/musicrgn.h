/** @file
 *
 * @par History
 */


#ifndef MUSICRGN_H
#define MUSICRGN_H

#include <string>
#include <vector>

#include "regions/region.h"
#include "zone.h"


namespace Pol::Clib
{
class ConfigElem;
}  // namespace Pol::Clib


namespace Pol::Core
{
void read_music_zones();

class MusicRegion : public Region
{
  using base = Region;

public:
  MusicRegion( Clib::ConfigElem& elem, RegionId id );
  size_t estimateSize() const override;


  unsigned short getmidi() const;

private:
  std::vector<unsigned short> midilist_;
};
}  // namespace Pol::Core

#endif
