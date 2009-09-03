/*
History
=======


Notes
=======

*/

#include "clib/stl_inc.h"
#ifdef _MSC_VER
#pragma warning( disable: 4786 )
#endif

#include <set>

#include "clib/dirlist.h"

#include "tiplist.h"


void load_tips()
{
    tipfilenames.clear();

    for( DirList dl( "tips/" ); !dl.at_end(); dl.next() )
    {
        string name = dl.name();
        if (name[0] == '.') continue;
        if (name.find( ".txt" ) != string::npos)
        {
            tipfilenames.push_back( name.c_str() );
        }
    }
}

