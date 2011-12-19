/*
History
=======
2005/12/05 MuadDib:   Added check for invul_tag from ssopt. If not 1, don't add tag.
2009/11/19 Turley:    ssopt.core_sends_season & .core_handled_tags - Tomi

Notes
=======

*/

#include "../clib/stl_inc.h"

#include <stdio.h>

#include "../clib/endian.h"

#include "mobile/charactr.h"
#include "network/client.h"
#include "msghandl.h"
#include "pktin.h"
#include "uconst.h"
#include "ufunc.h"
#include "ufuncstd.h"
#include "ustruct.h"
#include "uofile.h"
#include "uworld.h"
#include "ssopt.h"

Item* find_legal_singleclick_item( Character* chr, u32 serial )
{
    Item *item = find_legal_item( chr, serial );
    if (item)
        return item;

    // search equipment of nearby mobiles
    unsigned short wxL, wyL, wxH, wyH;
    zone_convert_clip( chr->x - RANGE_VISUAL, chr->y - RANGE_VISUAL, chr->realm, wxL, wyL );
    zone_convert_clip( chr->x + RANGE_VISUAL, chr->y + RANGE_VISUAL, chr->realm, wxH, wyH );
    for( unsigned short wx = wxL; wx <= wxH; ++wx )
    {
        for( unsigned short wy = wyL; wy <= wyH; ++wy )
        {
            ZoneCharacters& wchr = chr->realm->zone[wx][wy].characters;

            for( ZoneCharacters::iterator itr = wchr.begin(), end = wchr.end(); itr != end; ++itr )
            {
                Character* ochr = *itr;
                Item *_item = ochr->find_wornitem( serial );
                if (_item != NULL) 
                    return _item;
            }
        }
    }
    if (chr->trade_container())
    {
        item = chr->trade_container()->find( serial );
        if (item)
            return item;
    }
    if (chr->trading_with.get() && chr->trading_with->trade_container())
    {
        item = chr->trading_with->trade_container()->find( serial );
        if (item)
            return item;
    }

    return NULL;
}

void singleclick( Client *client, PKTIN_09 *msg)
{
    u32 serial = cfBEu32( msg->serial );
    //printf( "SingleClick: serial=%08lX\n", serial );

    if (IsCharacter( serial ))
    {
        Character *chr = NULL;
        if (serial == client->chr->serial)
            chr = client->chr;
        
        if (!chr)
        {
            chr = find_character( serial );
        }

        if (chr && inrange( client->chr, chr ) && !client->chr->is_concealed_from_me(chr) )
        {
            if (!chr->title_guild.empty() && (ssopt.core_handled_tags & 0x1))
                send_nametext( client, chr, "[" + chr->title_guild + "]" );
            send_nametext( client, chr, chr->name() );
            string tags;
			if (chr->frozen() && (ssopt.core_handled_tags & 0x2))
				tags = "[frozen] ";
			if (chr->paralyzed() && (ssopt.core_handled_tags & 0x4))
				tags += "[paralyzed] ";
			if (chr->squelched() && (ssopt.core_handled_tags & 0x8))
				tags += "[squelched] ";
			if (chr->deafened() && (ssopt.core_handled_tags & 0x10))
				tags += "[deafened] ";

			if (ssopt.invul_tag == 1) {
				if (chr->invul())
					tags += "[invulnerable]";
			}
            if (!tags.empty())
                send_nametext( client, chr, tags );
        }
    }
    else // single clicked on an item
    {
        Item *item = find_legal_singleclick_item( client->chr, serial );
        if (item)
        {
            send_objdesc( client, item );
        }
    }
}
                 
MESSAGE_HANDLER( PKTIN_09, singleclick );
