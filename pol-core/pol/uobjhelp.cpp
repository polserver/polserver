/*
History
=======


Notes
=======

*/

#include "../clib/stl_inc.h"
#ifdef _MSC_VER
#pragma warning( disable: 4786 )
#endif

#include "../clib/stlutil.h"

#include "ufunc.h"
#include "ufuncstd.h"
#include "uobjhelp.h"
#include "uobject.h"
#include "network/client.h"

void UObjectHelper::ShowProperties( Client* client, PropertyList& proplist )
{
    for( PropertyList::Properties::iterator itr = proplist.properties.begin();
         itr != proplist.properties.end();
         ++itr )
    {
        OSTRINGSTREAM os;
        os << (*itr).first << ": " << (*itr).second;
        send_sysmessage( client, OSTRINGSTREAM_STR(os) );
    }
}

void UObjectHelper::ShowProperties( Client* client, UObject* obj )
{
    ShowProperties( client, obj->proplist_ );
}
