/*
History
=======


Notes
=======

*/

#ifndef MUSICRGN_H
#define MUSICRGN_H

#include <string>

#include "region.h"

void read_music_zones();

class MusicRegion : public Region
{
    typedef Region base;
public:
    MusicRegion( ConfigElem& elem, RegionId id );

    unsigned short getmidi() const;
private:
    std::vector<unsigned short> midilist_;
};


typedef RegionGroup<MusicRegion> MusicDef;

extern MusicDef* musicdef;

#endif
