/*
History
=======


Notes
=======

*/

#include "../clib/stl_inc.h"

#include "../clib/cfgelem.h"
#include "../clib/random.h"

#include "musicrgn.h"
namespace Pol {
  namespace Core {
	MusicRegion::MusicRegion( Clib::ConfigElem& elem, RegionId id ) :
	  Region( elem, id )
	{
	  unsigned short midi_add;
	  while ( elem.remove_prop( "Midi", &midi_add ) )
	  {
		midilist_.push_back( midi_add );
	  }
	}

	unsigned short MusicRegion::getmidi() const
	{
	  if ( !midilist_.empty() )
	  {
        int idx = Clib::random_int( static_cast<int>( midilist_.size( )-1 ) );
		return midilist_[idx];
	  }
	  else
	  {
		return 0;
	  }
	}

	MusicDef* musicdef;

	void read_music_zones()
	{
	  musicdef = new MusicDef( "Music" );
	  read_region_data( *musicdef,
						"regions/music.cfg", // preferred
						"regions/regions.cfg", // other
						"MusicRegion Region" );


	}
  }
}