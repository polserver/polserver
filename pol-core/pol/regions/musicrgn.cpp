/** @file
 *
 * @par History
 */


#include "musicrgn.h"

#include "clib/cfgelem.h"
#include "clib/random.h"
#include "clib/stlutil.h"

#include "globals/uvars.h"

namespace Pol
{
namespace Core
{
MusicRegion::MusicRegion( Clib::ConfigElem& elem, RegionId id ) : Region( elem, id )
{
  unsigned short midi_add;
  while ( elem.remove_prop( "Midi", &midi_add ) )
  {
    midilist_.push_back( midi_add );
  }
}

size_t MusicRegion::estimateSize() const
{
  return base::estimateSize() + Clib::memsize( midilist_ );
}

unsigned short MusicRegion::getmidi() const
{
  if ( midilist_.empty() )
    return 0;
  int idx = Clib::random_int( static_cast<int>( midilist_.size() - 1 ) );
  return midilist_[idx];
}

void read_music_zones()
{
  gamestate.musicdef = new MusicDef( "Music" );
  read_region_data( *gamestate.musicdef,
                    "regions/music.cfg",    // preferred
                    "regions/regions.cfg",  // other
                    "MusicRegion Region" );
}
}  // namespace Core
}  // namespace Pol
