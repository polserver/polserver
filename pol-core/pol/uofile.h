/*
History
=======
2005/07/05 Shinigami: added uoconvert.cfg option *StaticsPerBlock (hard limit is set to 10000)
2005/07/16 Shinigami: added uoconvert.cfg flag ShowIllegalGraphicWarning
2006/04/09 Shinigami: added uoconvert.cfg flag ShowRoofAndPlatformWarning

Notes
=======

*/

#ifndef UOFILE_H
#define UOFILE_H

#include "ustruct.h"

#include "clidata.h"
#include "uconst.h"

#define MAX_STATICS_PER_BLOCK 10000

class Character;
class UMulti;
class Item;

extern signed char rawmapinfo( unsigned short x, unsigned short y, struct USTRUCT_MAPINFO* gi );
void rawmapfullread();
void getmapinfo( unsigned short x, unsigned short y, short* z, USTRUCT_MAPINFO* mi );
void readtile(unsigned short tilenum, USTRUCT_TILE *tile);
void readtile(unsigned short tilenum, USTRUCT_TILE_HSA *tile);
void clear_tiledata();
void readstaticblock( std::vector<USTRUCT_STATIC>* ppst, int* pnum, unsigned short x, unsigned short y );
void rawstaticfullread();
bool uo_passable( unsigned short x, unsigned short y );

/* All these moved to clidata.h
bool groundheight( unsigned short x, unsigned short y, int* z );
char tileheight(unsigned short tilenum);
unsigned char tilelayer( unsigned short tilenum );
u32 tile_flags( unsigned short tilenum );
*/

void read_objinfo( u16 graphic, struct USTRUCT_TILE& objinfo );
void read_objinfo( u16 graphic, struct USTRUCT_TILE_HSA& objinfo );
void readlandtile( unsigned short tilenum, USTRUCT_LAND_TILE* landtile );
void readlandtile( unsigned short tilenum, USTRUCT_LAND_TILE_HSA* landtile );

    // returns false if blocked.  new Z given new X, Y, and old Z.

bool uo_walkheight(unsigned short x, unsigned short y, short oldz, 
                short* newz, 
                UMulti** psupporting_multi,
                Item** pwalkon_item,
                bool doors_block = true,
                MOVEMODE movemode = MOVEMODE_LAND); 

    
    // does 'canmove' checking so GMs can walk through stuff
bool uo_walkheight(const Character* chr,
                unsigned short x, unsigned short y, short oldz, 
                short* newz, 
                UMulti** pmulti, Item** pwalkon);

bool uo_dropheight(unsigned short x, unsigned short y, short oldz, 
                short* newz, 
                UMulti** pmulti);

UMulti* uo_find_supporting_multi( unsigned short x, unsigned short y, short z );
//UMulti* uo_find_supporting_multi( MultiList& mvec, int z );

bool uo_lowest_standheight( unsigned short x, unsigned short y, short* z );

void open_uo_data_files( void );
void read_uo_data( void );
void readwater();

extern int uo_mapid;
extern int uo_usedif;
extern unsigned short uo_map_width;
extern unsigned short uo_map_height;

extern int cfg_max_statics_per_block;
extern int cfg_warning_statics_per_block;
extern bool cfg_show_illegal_graphic_warning;
extern bool cfg_show_roof_and_platform_warning;
extern bool cfg_use_new_hsa_format;

#endif
