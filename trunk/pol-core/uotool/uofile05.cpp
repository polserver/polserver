/*
History
=======

Notes
=======

*/

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

#include "pol/polcfg.h"
#include "pol/udatfile.h"
#include "pol/ustruct.h"

#include "pol/uofile.h"
#include "pol/uofilei.h"


void staticsmax()
{
    unsigned long max = 0;
    USTRUCT_IDX idxrec;

    fseek( sidxfile, 0, SEEK_SET );
    for( int xblock = 0; xblock < 6144/8; ++xblock )
    {
        for( int yblock = 0; yblock < 4096/8; ++yblock )
        {
            fread( &idxrec, sizeof idxrec, 1, sidxfile );

            if (idxrec.length != 0xFFffFFffLu)
            {
                if (idxrec.length > max)
                {
                    max = idxrec.length;
                    cout << "Max: " << max << ", X=" << xblock << ", Y=" << yblock << endl;
                }
            }
        }
    }
}

