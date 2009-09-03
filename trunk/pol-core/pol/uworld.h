/*
History
=======
2006/09/27 Shinigami: GCC 3.4.x fix - added "#include "charactr.h" because of ForEachMobileInVisualRange

Notes
=======

*/


#ifndef UWORLD_H
#define UWORLD_H

#include <set>

#include <assert.h>

#include "clib/passert.h"
#include "plib/realm.h"
#include "charactr.h"
#include "uvars.h"
#include "zone.h"

class Item;
class UMulti;
class Realm;

void add_item_to_world( Item* item );
void remove_item_from_world( Item* item );

void add_multi_to_world( UMulti* multi );
void remove_multi_from_world( UMulti* multi );
void move_multi_in_world( unsigned short oldx, unsigned short oldy,
                          unsigned short newx, unsigned short newy,
                          UMulti* multi, Realm* oldrealm );

void SetCharacterWorldPosition( Character* chr );
void ClrCharacterWorldPosition( Character* chr, const char* reason );
void MoveCharacterWorldPosition( unsigned short oldx, unsigned short oldy,
                                 unsigned short newx, unsigned short newy,
                                 Character* chr, Realm* oldrealm );

void SetItemWorldPosition( Item* item );
void ClrItemWorldPosition( Item* item );
void MoveItemWorldPosition( unsigned short oldx, unsigned short oldy,
                            Item* item, Realm* oldrealm );

long get_toplevel_item_count();
long get_mobile_count();

typedef std::set<Character*> ZoneCharacters;
typedef std::vector<UMulti*> ZoneMultis;
typedef std::vector<Item*> ZoneItems;

struct Zone {
    ZoneCharacters characters;
    ZoneItems items;
    ZoneMultis multis;
};

const unsigned WGRID_SIZE = 64;
const unsigned WGRID_SHIFT = 6;

const unsigned WGRID_X = WORLD_X / WGRID_SIZE; // 6144 / 64 = 96
const unsigned WGRID_Y = WORLD_Y / WGRID_SIZE; // 4096 / 64 = 64

class World 
{
public:
    World();

    //Items::iterator find_item( unsigned long serial );
    //Items::iterator find_item( unsigned long serial, unsigned short x, unsigned short y );

    Zone zone[WGRID_X][WGRID_Y];
    unsigned long toplevel_item_count;
    unsigned long mobile_count;
};
extern World world;

inline void zone_convert( unsigned short x, unsigned short y, unsigned short& wx, unsigned short& wy, Realm* realm )
{
    passert( x < realm->width() );
    passert( y < realm->height() );

    wx = x >> WGRID_SHIFT;
    wy = y >> WGRID_SHIFT;
}

inline void zone_convert_clip( int x, int y, const Realm* realm, unsigned short& wx, unsigned short& wy )
{
    if (x < 0)
        x = 0;
    if (y < 0)
        y = 0;
	if ((unsigned)x >= realm->width())
        x = realm->width() - 1;
    if ((unsigned)y >= realm->height())
        y = realm->height() - 1;

    wx = static_cast<unsigned short>(x >> WGRID_SHIFT);
    wy = static_cast<unsigned short>(y >> WGRID_SHIFT);
}

inline Zone& getzone( unsigned short x, unsigned short y, Realm* realm )
{
	passert( x < realm->width() );
	passert( y < realm->height() );

    return realm->zone[x >> WGRID_SHIFT][y >> WGRID_SHIFT];
}

template <class A>
void ForEachMobileInRange( int x, int y, Realm* realm, unsigned range,
                          void (*f)(Character*, A* staticdata), A* staticdata )
{
    unsigned short wxL, wyL, wxH, wyH;

    zone_convert_clip( x - range, y - range, realm, wxL, wyL );
    zone_convert_clip( x + range, y + range, realm, wxH, wyH );
    passert( wxL <= wxH );
    passert( wyL <= wyH );
    for( unsigned short wx = wxL; wx <= wxH; ++wx )
    {
        for( unsigned short wy = wyL; wy <= wyH; ++wy )
        {
            ZoneCharacters& wchr = realm->zone[wx][wy].characters;

            for( ZoneCharacters::iterator itr = wchr.begin(), end = wchr.end(); itr != end; ++itr )
            {
                Character* chr = *itr;
                (*f)(chr, staticdata);
            }
        }
    }
}
template <class A>
void ForEachMobileInVisualRange( Character* chr,
                          void (*f)(Character*, A* staticdata), A* staticdata )
{
    ForEachMobileInRange( chr->x, chr->y, chr->realm, RANGE_VISUAL, f, staticdata );
}

#endif
