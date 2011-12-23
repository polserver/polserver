/*
History
=======
2005/03/01 Shinigami: extended error message for passert(pstat[i].graphic < 0x4000)
2005/07/16 Shinigami: added uoconvert.cfg flag ShowIllegalGraphicWarning
2009/12/02 Turley:    added config.max_tile_id - Tomi


Notes
=======

*/

#include "../clib/stl_inc.h"

#include <stdio.h>

#ifdef __unix__
#include <unistd.h>
#endif

#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/fileutil.h"
#include "../clib/passert.h"
#include "../clib/random.h"
#include "../clib/stlutil.h"
#include "../clib/strutil.h"
#include "../clib/wallclock.h"

#include "../plib/realmdescriptor.h"
#include "../plib/staticblock.h"

#include "item/itemdesc.h"
#include "ustruct.h"

#include "polcfg.h"
#include "polfile.h"
#include "profile.h"
#include "uofilei.h"
#include "uofile.h"

bool cfg_show_illegal_graphic_warning = 1;

bool newstat_dont_add( vector<STATIC_ENTRY>& vec, USTRUCT_STATIC* pstat)
{
    char pheight = tileheight( pstat->graphic );

    for( unsigned i = 0; i < vec.size(); ++i )
    {
        STATIC_ENTRY& prec = vec[i];
        passert_always( prec.objtype <= config.max_tile_id );
        char height = tileheight( prec.objtype ); // TODO read from itemdesc?
        unsigned char xy = (pstat->x_offset << 4) | pstat->y_offset;
        if (// flags == pflags &&
            prec.objtype == pstat->graphic && // TODO map objtype->graphic from itemdesc
            height == pheight &&
            prec.xy == xy &&
            prec.z == pstat->z &&
			prec.hue == pstat->hue)
        {
            return true;
        }
    }
    return false;
}

int write_pol_static_files( const string& realm )
{
    unsigned int duplicates = 0;
    unsigned int illegales = 0;
    unsigned int statics = 0;
    unsigned int empties = 0;
    unsigned int nonempties = 0;
    unsigned int maxcount = 0;

    string directory = "realm/" + realm + "/";
    string statidx_dat = directory + "statidx.dat";
    string statics_dat = directory + "statics.dat";
    string statidx_tmp = directory + "statidx.tmp";
    string statics_tmp = directory + "statics.tmp";
    RemoveFile( statidx_dat );
    RemoveFile( statics_dat );
    RemoveFile( statidx_tmp );
    RemoveFile( statics_tmp );

    FILE* fidx = fopen( statidx_tmp.c_str(), "wb" );
    FILE* fdat = fopen( statics_tmp.c_str(), "wb" );

    RealmDescriptor descriptor = RealmDescriptor::Load( realm );

    int lastprogress = -1;
    unsigned int index = 0;
    for( u16 y = 0; y < descriptor.height; y += STATICBLOCK_CHUNK )
    {
        int progress = y*100L/descriptor.height;
        if (progress != lastprogress)
        {
             cout << "\rCreating POL statics files: " << progress << "%";
             lastprogress=progress;
        }
        for( u16 x = 0; x < descriptor.width; x += STATICBLOCK_CHUNK )
        {
            STATIC_INDEX idx;
            idx.index = index;
            fwrite( &idx, sizeof idx, 1, fidx );

            USTRUCT_STATIC* pstat;
            int num;
            vector<STATIC_ENTRY> vec;
            readstaticblock( &pstat, &num, x, y );
            for( int i = 0; i < num; ++i )
            {
                if (pstat[i].graphic <= config.max_tile_id)
                {
                    if (!newstat_dont_add(vec,&pstat[i]))
                    {
                        STATIC_ENTRY nrec;

                        nrec.objtype = pstat[i].graphic; // TODO map these?
                        nrec.xy = (pstat[i].x_offset << 4) | pstat[i].y_offset;
                        nrec.z = pstat[i].z;
						nrec.hue = pstat[i].hue;
                        vec.push_back( nrec );
                        ++statics;
                    }
                    else
                    {
                        ++duplicates;
                    }
                }
                else
                {
                    ++illegales;

                    if (cfg_show_illegal_graphic_warning)
                        cout << " Warning: Item with illegal Graphic 0x" << hex << pstat[i].graphic
                             << " in Area " << dec << x << " " << y << " " << (x + STATICBLOCK_CHUNK - 1)
                             << " " << (y + STATICBLOCK_CHUNK - 1) << endl;
                }
            }
            for( unsigned i = 0; i < vec.size(); ++i )
            {
                fwrite( &vec[i], sizeof(STATIC_ENTRY), 1, fdat );
                ++index;
            }
            if (vec.empty())
                ++empties;
            else
                ++nonempties;
            if (vec.size() > maxcount)
                maxcount = static_cast<unsigned int>(vec.size());
        }
    }
    STATIC_INDEX idx;
    idx.index = index;
    fwrite( &idx, sizeof idx, 1, fidx );

    int errors = ferror( fdat ) || ferror( fidx );
    fclose( fdat );
    fclose( fidx );
    if (!errors)
    {
        cout << "\rCreating POL statics files: Complete" << endl;
        rename( statidx_tmp.c_str(), statidx_dat.c_str() );
        rename( statics_tmp.c_str(), statics_dat.c_str() );
    }
    else
    {
        cout << "\rCreating POL statics files: Error" << endl;
    }


#ifndef NDEBUG
    cout << statics << " statics written" << endl;
    cout << duplicates << " duplicates eliminated" << endl;
    cout << illegales << " illegales eliminated" << endl;
    cout << empties << " empties" << endl;
    cout << nonempties << " nonempties" << endl;
    cout << maxcount << " was the highest count" << endl;
#endif
    return 0;
}
