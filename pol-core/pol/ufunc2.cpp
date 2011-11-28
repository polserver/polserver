/*
History
=======
2009/07/23 MuadDib:   updates for new Enum::Packet Out ID
2009/09/06 Turley:    Changed Version checks to bitfield client->ClientType
2009/10/12 Turley:    whisper/yell/say-range ssopt definition

Notes
=======

*/

#include "../clib/stl_inc.h"
#include <string.h>

#include "../clib/endian.h"
#include "../clib/clib.h"

#include "network/client.h"
#include "network/packets.h"
#include "network/clienttransmit.h"
#include "sockio.h"
#include "menu.h"
#include "pktout.h"
#include "tooltips.h"
#include "ufunc.h"
#include "uworld.h"

bool send_menu( Client *client, Menu *menu )
{
	// build up the message so it gets sent as a unit.
	if (menu->menuitems_.size() > 255)
        return false;

	PktOut_7C* msg = REQUESTPACKET(PktOut_7C,PKTOUT_7C_ID);
	msg->offset+=2;
	msg->offset+=4; //used_item_serial
	msg->WriteFlipped(menu->menu_id);
	size_t stringlen = strlen( menu->title );
	if (stringlen > 80)
		stringlen = 80;
	msg->Write(static_cast<u8>(stringlen));// NOTE null-term not included!
	msg->Write(menu->title, static_cast<u16>(stringlen), false);
	msg->Write(static_cast<u8>(menu->menuitems_.size()));

	for( unsigned idx = 0; idx < menu->menuitems_.size(); idx++ )
	{
		if (msg->offset + 85 > static_cast<u16>(sizeof msg->buffer))
		{
			READDPACKET(msg);        
            return false;
		}
		MenuItem* mi = &menu->menuitems_[ idx ];
		msg->WriteFlipped(mi->graphic_);
		msg->WriteFlipped(mi->color_);
		stringlen = strlen( mi->title );
		if (stringlen > 80)
			stringlen = 80;
		msg->Write(static_cast<u8>(stringlen));// NOTE null-term not included!
		msg->Write(mi->title, static_cast<u16>(stringlen), false);
	}
	u16 len = msg->offset;
	msg->offset = 1;
	msg->WriteFlipped(len);
	transmit( client, &msg->buffer, len );
	READDPACKET(msg);
    return true;
}

#include "npc.h"
#include "uvars.h"
//DAVE added texttype, ranges
void for_nearby_npcs( void (*f)(NPC& npc, Character *chr, const char *text, int textlen, u8 texttype),
                      Character *p_chr, const char *p_text, int p_textlen, u8 texttype )
{
    int range;
    switch( texttype )
    {
    case TEXTTYPE_WHISPER:  range = ssopt.whisper_range;  break;
    case TEXTTYPE_YELL:     range = ssopt.yell_range;     break;
    default:                range = ssopt.speech_range;   break;
    }

    unsigned short wxL, wyL, wxH, wyH;
    zone_convert_clip( p_chr->x - range, p_chr->y - range, p_chr->realm, wxL, wyL );
    zone_convert_clip( p_chr->x + range, p_chr->y + range, p_chr->realm, wxH, wyH );
    for( unsigned short wx = wxL; wx <= wxH; ++wx )
    {
        for( unsigned short wy = wyL; wy <= wyH; ++wy )
        {
            ZoneCharacters& wchr = p_chr->realm->zone[wx][wy].characters;

            for( ZoneCharacters::iterator itr = wchr.begin(), end = wchr.end(); itr != end; ++itr )
            {
                Character* chr = *itr;

                if (chr->isa(UObject::CLASS_NPC))
                {
                    NPC* npc = static_cast<NPC*>(chr);

                    if (inrangex( npc, p_chr, range ))
                        (*f)( *npc, p_chr, p_text, p_textlen, texttype );
                }
            }
        }
    }
}

// UNICODE version
void for_nearby_npcs( void (*f)(NPC& npc, Character *chr, const char *text, int textlen, u8 texttype, const u16 *wtext, const char lang[4], int wtextlen),
                      Character *p_chr, const char *p_text, int p_textlen, u8 texttype, const u16 *p_wtext, const char p_lang[4], int p_wtextlen )
{
    int range;
    switch( texttype )
    {
    case TEXTTYPE_WHISPER:  range = ssopt.whisper_range;  break;
    case TEXTTYPE_YELL:     range = ssopt.yell_range;     break;
    default:                range = ssopt.speech_range;   break;
    }

    unsigned short wxL, wyL, wxH, wyH;
    zone_convert_clip( p_chr->x - range, p_chr->y - range, p_chr->realm, wxL, wyL );
    zone_convert_clip( p_chr->x + range, p_chr->y + range, p_chr->realm, wxH, wyH );
    for( unsigned short wx = wxL; wx <= wxH; ++wx )
    {
        for( unsigned short wy = wyL; wy <= wyH; ++wy )
        {
            ZoneCharacters& wchr = p_chr->realm->zone[wx][wy].characters;

            for( ZoneCharacters::iterator itr = wchr.begin(), end = wchr.end(); itr != end; ++itr )
            {
                Character* chr = *itr;
                if (chr->isa(UObject::CLASS_NPC))
                {
                    NPC* npc = static_cast<NPC*>(chr);

			        if (inrangex( p_chr, npc, range ))
                        (*f)( *npc, p_chr, p_text, p_textlen, texttype, p_wtext, p_lang, p_wtextlen );
                }
            }
        }
    }
}

void send_open_gump( Client *client, const UContainer& cont )
{
	PktOut_24* msg = REQUESTPACKET(PktOut_24,PKTOUT_24_ID);
	msg->Write(cont.serial_ext);
	msg->WriteFlipped(cont.gump());
	if (client->ClientType & CLIENTTYPE_7090)
		msg->WriteFlipped(static_cast<u16>(0x7D));
	transmit( client, &msg->buffer, msg->offset );
	READDPACKET(msg);
}

//dave changed 11/9/3, don't send invis items to those who can't see invis
void send_container_contents( Client *client, const UContainer& cont, bool show_invis )
{
	PktOut_3C* msg = REQUESTPACKET(PktOut_3C,PKTOUT_3C_ID);
	msg->offset+=4; //msglen+count
	u16 count = 0;
	UContainer::const_iterator itr = cont.begin();
	for( ; itr != cont.end(); ++itr )
	{
		const Item* item = GET_ITEM_PTR( itr );
		if ( show_invis || (!item->invisible() || client->chr->can_seeinvisitems()) )
		{
			msg->Write(item->serial_ext);
			msg->Write(ctBEu16(item->graphic));
			msg->offset++; //unk6
			msg->WriteFlipped(item->get_senditem_amount());
			msg->WriteFlipped(item->x);
			msg->WriteFlipped(item->y);
			if ( client->ClientType & CLIENTTYPE_6017 )
				msg->Write(item->slot_index());
			msg->Write(cont.serial_ext);
			msg->Write(item->color_ext); //color
			++count;
		}
		else
		{
			send_remove_object(client,item);
		}
	}
	u16 len=msg->offset;
	msg->offset=1;
	msg->WriteFlipped(len);
	msg->WriteFlipped(count);
	ADDTOSENDQUEUE(client, &msg->buffer, len );
	READDPACKET(msg);

	if(client->UOExpansionFlag & AOS)
	{
		// 07/11/09 Turley: moved to bottom first the client needs to know the item then we can send revision
		UContainer::const_iterator itr = cont.begin();
		for( ; itr != cont.end(); ++itr )
		{
			const Item* item = GET_ITEM_PTR( itr );
		    if ( show_invis || (!item->invisible() || client->chr->can_seeinvisitems()) )
	        {
				send_object_cache(client, dynamic_cast<const UObject*>(item));
			}
		}
	}
}
