/*
History
=======
2009/07/23 MuadDib:   updates for new Enum::Packet Out ID
2009/09/03 MuadDib:   Relocation of multi related cpp/h
2010/01/22 Turley:    Speedhack Prevention System

Notes
=======

*/

#include "../clib/stl_inc.h"

#include "../clib/endian.h"
#include "../clib/stlutil.h"

#include "mobile/charactr.h"
#include "network/client.h"
#include "multi/customhouses.h"
#include "multi/house.h"
#include "msghandl.h"
#include "pktin.h"
#include "pktout.h"
#include "pktboth.h"
#include "uvars.h"
#include "multi/multi.h"
#include "ssopt.h"
#include "ufunc.h"
#include "uworld.h"
#include "network/packets.h"
#include "network/clienttransmit.h"
#include "../clib/pkthelper.h"


void send_char_if_newly_inrange( Character *chr, Client *client	)
{
	if (  inrange( chr, client->chr ) &&  !inrange( chr->x, chr->y, client->chr->lastx, client->chr->lasty ) &&
          client->chr->is_visible_to_me( chr ) && client->chr != chr)
	{
		send_owncreate( client, chr );
	}
}

void send_item_if_newly_inrange( Item *item, Client *client )
{
	if (  inrange( client->chr, item ) && !inrange( item->x, item->y, client->chr->lastx, client->chr->lasty ))
	{
		send_item( client, item );
	}
}

void send_multi_if_newly_inrange( UMulti *multi, Client *client )
{
	if (  multi_inrange( client->chr, multi ) && !multi_inrange( multi->x, multi->y, client->chr->lastx, client->chr->lasty ))
	{
		send_multi( client, multi );
        UHouse* house = multi->as_house();
        if( (client->UOExpansionFlag & AOS) && house != NULL && house->IsCustom())
            CustomHousesSendShort(house, client);
	}
}

void send_objects_newly_inrange( Client* client )
{
    Character* chr = client->chr;
    unsigned short wxL, wyL, wxH, wyH;

	zone_convert_clip( chr->x - RANGE_VISUAL_LARGE_BUILDINGS, chr->y - RANGE_VISUAL_LARGE_BUILDINGS, chr->realm, wxL, wyL );
    zone_convert_clip( chr->x + RANGE_VISUAL_LARGE_BUILDINGS, chr->y + RANGE_VISUAL_LARGE_BUILDINGS, chr->realm, wxH, wyH );
    for( unsigned short wx = wxL; wx <= wxH; ++wx )
    {
        for( unsigned short wy = wyL; wy <= wyH; ++wy )
        {
            ZoneMultis& wmulti = chr->realm->zone[wx][wy].multis;
            for( ZoneMultis::iterator itr = wmulti.begin(), end = wmulti.end(); itr != end; ++itr )
            {
                UMulti* multi = *itr;
                send_multi_if_newly_inrange( multi, client );
            }
        }
    }

    zone_convert_clip( chr->x - RANGE_VISUAL, chr->y - RANGE_VISUAL, chr->realm, wxL, wyL );
    zone_convert_clip( chr->x + RANGE_VISUAL, chr->y + RANGE_VISUAL, chr->realm, wxH, wyH );
    for( unsigned short wx = wxL; wx <= wxH; ++wx )
    {
        for( unsigned short wy = wyL; wy <= wyH; ++wy )
        {
            ZoneCharacters& wchr = chr->realm->zone[wx][wy].characters;
            for( ZoneCharacters::iterator itr = wchr.begin(), end = wchr.end(); itr != end; ++itr )
            {
                Character* chr = *itr;
                send_char_if_newly_inrange( chr, client );
            }


            ZoneItems& witem = chr->realm->zone[wx][wy].items;
            for( ZoneItems::iterator itr = witem.begin(), end = witem.end(); itr != end; ++itr )
            {
                Item* item = *itr;
                send_item_if_newly_inrange( item, client );
            }
        }
    }
}

void remove_objects_inrange( Client* client )
{
    Character* chr = client->chr;
    unsigned short wxL, wyL, wxH, wyH;
	PktOut_1D* msgremove = REQUESTPACKET(PktOut_1D,PKTOUT_1D_ID);
	zone_convert_clip( chr->x - RANGE_VISUAL_LARGE_BUILDINGS, chr->y - RANGE_VISUAL_LARGE_BUILDINGS, chr->realm, wxL, wyL );
    zone_convert_clip( chr->x + RANGE_VISUAL_LARGE_BUILDINGS, chr->y + RANGE_VISUAL_LARGE_BUILDINGS, chr->realm, wxH, wyH );
    for( unsigned short wx = wxL; wx <= wxH; ++wx )
    {
        for( unsigned short wy = wyL; wy <= wyH; ++wy )
        {
            ZoneMultis& wmulti = chr->realm->zone[wx][wy].multis;
            for( ZoneMultis::iterator itr = wmulti.begin(), end = wmulti.end(); itr != end; ++itr )
            {
                UMulti* multi = *itr;
				send_remove_object( client, static_cast<const Item*>(multi), msgremove );
            }
        }
    }

    zone_convert_clip( chr->x - RANGE_VISUAL, chr->y - RANGE_VISUAL, chr->realm, wxL, wyL );
    zone_convert_clip( chr->x + RANGE_VISUAL, chr->y + RANGE_VISUAL, chr->realm, wxH, wyH );
    for( unsigned short wx = wxL; wx <= wxH; ++wx )
    {
        for( unsigned short wy = wyL; wy <= wyH; ++wy )
        {
            ZoneCharacters& wchr = chr->realm->zone[wx][wy].characters;
            for( ZoneCharacters::iterator itr = wchr.begin(), end = wchr.end(); itr != end; ++itr )
            {
                Character* chr = *itr;
				send_remove_character( client, chr, msgremove );
            }


            ZoneItems& witem = chr->realm->zone[wx][wy].items;
            for( ZoneItems::iterator itr = witem.begin(), end = witem.end(); itr != end; ++itr )
            {
                Item* item = *itr;
				send_remove_object( client, item, msgremove );
            }
        }
    }
	READDPACKET(msgremove);
}

void cancel_trade( Character* chr1 );
void handle_walk( Client *client, PKTIN_02 *msg02 )
{
	Character *chr = client->chr;

	if ( (client->movementsequence == 0 ) && ( msg02->movenum != 0 ) )
	{
		//drop pkt if last request was denied, should fix the "client hopping"

		/*PktOut_21* msg = REQUESTPACKET(PktOut_21,PKTOUT_21_ID);
		msg->Write(msg02->movenum);
		msg->WriteFlipped(chr->x);
		msg->WriteFlipped(chr->y);
		msg->Write(chr->facing);
		msg->Write(chr->z);
		client->transmit( &msg->buffer, msg->offset );
		READDPACKET(msg);*/

		return;
	}
	else
	{	
		u8 oldfacing=chr->facing;

		if (chr->move( msg02->dir ))
		{
			// If facing is dir they are walking, check to see if already 4 tiles away
			// from the person trading with. If so, cancel trading!!!!
			if ( !ssopt.allow_moving_trade )
			{
				if ( chr->is_trading() )
				{
					if ( (oldfacing == (msg02->dir& PKTIN_02_FACING_MASK)) && (pol_distance(chr->x, chr->y, chr->trading_with->x, chr->trading_with->y) > 3) )
					{
						cancel_trade( chr );
					}
				}
			}
			client->pause();
			PktOut_22* msg = REQUESTPACKET(PktOut_22,PKTBI_22_APPROVED_ID);
			msg->Write(msg02->movenum);
			msg->Write(client->chr->hilite_color_idx( client->chr ));
			ADDTOSENDQUEUE(client, &msg->buffer, msg->offset );
			READDPACKET(msg);

			client->movementsequence = msg02->movenum;
			if (client->movementsequence == 255)
				client->movementsequence = 1;
			else
				client->movementsequence++;


			// FIXME: Make sure we only tell those who can see us.
			chr->tellmove();

			send_objects_newly_inrange( client );

			client->restart();

			// here we set the delay for SpeedHackPrevention see Client::SpeedHackPrevention()
			if (oldfacing == (msg02->dir & PKTIN_02_FACING_MASK))
			{
				if( client->chr->on_mount() )
					client->next_movement += (msg02->dir & PKTIN_02_DIR_RUNNING_BIT) ? PKTIN_02_MOUNT_RUN : PKTIN_02_MOUNT_WALK;
				else
					client->next_movement += (msg02->dir & PKTIN_02_DIR_RUNNING_BIT) ? PKTIN_02_FOOT_RUN  : PKTIN_02_FOOT_WALK;
			}
			else // changing only facing is fast
				client->next_movement += PKTIN_02_MOUNT_RUN;
		}
		else
		{
			PktOut_21* msg = REQUESTPACKET(PktOut_21,PKTOUT_21_ID);
			msg->Write(msg02->movenum);
			msg->WriteFlipped(chr->x);
			msg->WriteFlipped(chr->y);
			msg->Write(chr->facing);
			msg->Write(chr->z);
			ADDTOSENDQUEUE(client, &msg->buffer, msg->offset );
			READDPACKET(msg);

			client->movementsequence = 0;
		}
	}
}
MESSAGE_HANDLER( PKTIN_02, handle_walk );
