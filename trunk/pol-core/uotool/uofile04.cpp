/*
History
=======

Notes
=======

*/

#ifdef _MSC_VER
#pragma warning( disable: 4786 )
#endif

#include <iostream>
#include <set>
#include <stdexcept>
#include <string>

using std::cout;
using std::cerr;
using std::endl;
using std::runtime_error;
using std::string;

#include <stdio.h>
#include <string.h>

#include "../pol/polcfg.h"
#include "../pol/udatfile.h"
#include "../pol/ustruct.h"

#include "../pol/uofile.h"
#include "../pol/uofilei.h"

std::set<unsigned int> water;

inline bool iswater( u16 objtype )
{
    return (objtype >= 0x1796 && objtype <= 0x17b2);
}

void readwater()
{
    USTRUCT_IDX idxrec;

    fseek( sidxfile, 0, SEEK_SET );
    for( int xblock = 0; xblock < 6144/8; ++xblock )
    {
        cout << xblock << "..";
        for( int yblock = 0; yblock < 4096/8; ++yblock )
        {
            fread( &idxrec, sizeof idxrec, 1, sidxfile );
            int xbase = xblock * 8;
            int ybase = yblock * 8;

            if (idxrec.length != 0xFFffFFffLu)
            {
                fseek( statfile, idxrec.offset, SEEK_SET );

                for( idxrec.length /= 7; idxrec.length > 0; --idxrec.length )
                {
                    USTRUCT_STATIC srec;
                    fread( &srec, sizeof srec, 1, statfile );

                    if (srec.z == -5 && iswater( srec.graphic ) )
                    {
                        int x = xbase + srec.x_offset;
                        int y = ybase + srec.y_offset;
                        unsigned int xy = x<<16|y;
                        water.insert(xy);
                    }

                }
            }
        }
    }
}

