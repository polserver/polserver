/*
History
=======
2007/04/07 MuadDib:   send_object_cache_to_inrange updated from just UObject* to
                      const UObject* for compatibility across more areas.

Notes
=======

*/

#ifndef __TOOLTIPS_H
#define __TOOLTIPS_H

#include "network/client.h"
#include "uconst.h"
#include "uobject.h"

void send_object_cache(Client* client, const UObject* obj);
void send_object_cache_to_inrange(const UObject* obj);
unsigned char* BuildObjCache (const UObject* obj, bool new_style);
void SendAOSTooltip(Client* client,UObject* item, bool vendor_content = false);

#endif
