/** @file
 *
 * @par History
 */


#include "uobjhelp.h"

#include "ufunc.h"
#include "ufuncstd.h"
#include "uobject.h"
#include "network/client.h"

#include "../clib/stlutil.h"

namespace Pol
{
namespace Core
{
void UObjectHelper::ShowProperties( Network::Client* client, PropertyList& proplist )
{
  for ( PropertyList::Properties::iterator itr = proplist.properties.begin();
        itr != proplist.properties.end(); ++itr )
  {
    OSTRINGSTREAM os;
    os << ( *itr ).first << ": " << ( *itr ).second;
    send_sysmessage( client, OSTRINGSTREAM_STR( os ) );
  }
}

void UObjectHelper::ShowProperties( Network::Client* client, UObject* obj )
{
  ShowProperties( client, obj->proplist_ );
}
}
}