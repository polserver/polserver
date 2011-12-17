/*
History
=======
2007/04/07 MuadDib:   send_object_cache_to_inrange updated from just UObject* to
                      const UObject* for compatibility across more areas.
2009/01/03 MuadDib:   Some generic recoding to remove useless code. O_O
2009/01/27 MuadDib:   Rewrote Obj Cache Building/Sending.
2009/07/26 MuadDib:   Packet struct refactoring.
2009/09/06 Turley:    Changed Version checks to bitfield client->ClientType

Notes
=======

*/

#include "../clib/stl_inc.h"
#include "../clib/endian.h"

#include "mobile/charactr.h"
#include "network/client.h"
#include "network/packets.h"
#include "network/clienttransmit.h"
#include "item/itemdesc.h"
#include "msghandl.h"
#include "pktboth.h"
#include "pktin.h"
#include "ssopt.h"
#include "ufunc.h"
#include "uvars.h"

#include "tooltips.h"

using namespace PktHelper;


void handle_request_tooltip( Client* client, PKTIN_B6* msgin )
{
    u32 serial = cfBEu32( msgin->serial );
    if (!IsCharacter( serial ))
    {
        Item* item = find_legal_item( client->chr, serial );
        if (item != NULL)
        {
            const ItemDesc& id = find_itemdesc( item->objtype_ );
            if (!id.tooltip.empty())
            {
				PacketOut<PktOut_B7> msg;
				msg->offset+=2;
				msg->Write(item->serial_ext);
				const char* string = id.tooltip.c_str();
				while (*string) //unicode
					msg->Write(static_cast<u16>((*string++) << 8));
				msg->offset+=2; //nullterm
				u16 len=msg->offset;
				msg->offset=1;
				msg->WriteFlipped(len);
				msg.Send(client, len);
            }
        }
    }
}
MESSAGE_HANDLER( PKTIN_B6, handle_request_tooltip );

//needed if A9 flag is sent with 0x20, single click no longer works. see about text# 1042971 for 0xD6
void send_object_cache(Client* client, const UObject* obj)
{
	if (ssopt.uo_feature_enable & PKTOUT_A9::FLAG_AOS_FEATURES)
	{
		if ((ssopt.force_new_objcache_packets) || (client->ClientType & CLIENTTYPE_5000))
		{
			PacketOut<PktOut_DC> msgdc;
			msgdc->Write(obj->serial_ext);
			msgdc->WriteFlipped(obj->rev());
			msgdc.Send(client);
		}
		else
		{
			PacketOut<PktOut_BF_Sub10> msgbf10;
			msgbf10->WriteFlipped(static_cast<u16>(0xD));
			msgbf10->offset+=2; //sub
			msgbf10->Write(obj->serial_ext);
			msgbf10->WriteFlipped(obj->rev());
			msgbf10.Send(client);
		}
	}
}

void send_object_cache_to_inrange(const UObject* obj)
{
	if (ssopt.uo_feature_enable & PKTOUT_A9::FLAG_AOS_FEATURES)
	{
		// Since this is an InRange function, at least 1 person. So it isn't too far
		// fetched to build for AOS and UOKR both, since both could be used. At least
		// one will always be used, and this really makes a different in large groups.
		PacketOut<PktOut_DC> msgdc;
		PacketOut<PktOut_BF_Sub10> msgbf10;

		for( Clients::iterator itr = clients.begin(), end = clients.end(); itr != end; ++itr )
		{
	        Client *client2 = *itr;
		    if (!client2->ready)
			    continue;
		    // FIXME need to check character's additional_legal_items.
		    // looks like inrange should be a Character member function.
			if(client2->UOExpansionFlag & AOS)
		    {
				if (inrange(client2->chr, obj))
				{
					//send_object_cache(client2, obj);
					if ((ssopt.force_new_objcache_packets) || (client2->ClientType & CLIENTTYPE_5000))
					{
						if (msgdc->offset==1)
						{
							msgdc->Write(obj->serial_ext);
							msgdc->WriteFlipped(obj->rev());
						}
						msgdc.Send(client2);						
					}
					else
					{
						if (msgbf10->offset==1)
						{
							msgbf10->WriteFlipped(static_cast<u16>(0xD));
							msgbf10->offset+=2; //sub
							msgbf10->Write(obj->serial_ext);
							msgbf10->WriteFlipped(obj->rev());
						}
						msgbf10.Send(client2);
					}
				}
			}
		}
	}
}


void SendAOSTooltip(Client* client, UObject* obj, bool vendor_content)
{
    string desc;
    if(obj->isa(UObject::CLASS_CHARACTER))
    {
        Character* chr = (Character*)obj;
        desc = (chr->title_prefix.empty() ? " \t": chr->title_prefix + " \t") + chr->name() + 
			(chr->title_suffix.empty() ? "\t ": "\t " + chr->title_suffix);
        if (!chr->title_race.empty())
            desc += " (" +  chr->title_race + ")";
        if (!chr->title_guild.empty())
            desc += " [" +  chr->title_guild + "]";
    }
    else
		if (vendor_content)
		{
			Item* item = (Item*)obj;
			desc = item->merchant_description();
		}
		else
			desc = obj->description();

	PacketOut<PktOut_D6> msg;
	msg->offset+=2;
	msg->WriteFlipped(static_cast<u16>(1)); //u16 unk1
	msg->Write(obj->serial_ext);
	msg->offset+=2; // u8 unk2,unk3
	msg->WriteFlipped(obj->rev());
	if(obj->isa(UObject::CLASS_CHARACTER))
		msg->WriteFlipped(static_cast<u32>(1050045));   //1 text argument only
	else
		msg->WriteFlipped(static_cast<u32>(1042971));   //1 text argument only

	msg->WriteFlipped(static_cast<u16>(desc.size()*2));
	const char* string = desc.c_str();
	while (*string) //unicode
		msg->Write(static_cast<u16>(*string++));
	msg->offset+=4; // indicates end of property list
	u16 len=msg->offset;
	msg->offset=1;
	msg->WriteFlipped(len);
	msg.Send(client, len);
}

