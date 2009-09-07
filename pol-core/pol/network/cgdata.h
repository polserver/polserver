/*
History
=======


Notes
=======

*/

#ifndef CGDATA_H
#define CGDATA_H

#include "../../clib/refptr.h"

#include "../containr.h"
#include "../npc.h"
#include "../reftypes.h"

class JusticeRegion;
class LightRegion;
class MusicRegion;
class WeatherRegion;

class UnicodeExecutorModule;

class ClientGameData
{
public:
    ClientGameData();
    ~ClientGameData();
    void clear();


    ref_ptr<UContainer> vendor_bought;
    ref_ptr<UContainer> vendor_for_sale;
    NPCRef vendor;

    typedef std::set<UOExecutorModule*> GumpMods;
    GumpMods gumpmods;

    void add_gumpmod( UOExecutorModule* );
    UOExecutorModule* find_gumpmod( u32 pid );
    void remove_gumpmod( UOExecutorModule* );

    UOExecutorModule* textentry_uoemod;


        /* 
         *  If a script is requesting a target cursor from this
         *  character, target_cursor_ex is that script.
         *  Same for menu selection.
         */
    UOExecutorModule* target_cursor_uoemod;
    UOExecutorModule* menu_selection_uoemod;
    UOExecutorModule* prompt_uoemod;
    UOExecutorModule* resurrect_uoemod;
    UOExecutorModule* selcolor_uoemod;

	UnicodeExecutorModule* prompt_uniemod; // UNIemod, not UOemod!!!

    JusticeRegion* justice_region;
    
    int lightlevel;
    // LightRegion* light_region;
    MusicRegion* music_region;
    WeatherRegion* weather_region;
    u32 custom_house_serial;
};

#endif
