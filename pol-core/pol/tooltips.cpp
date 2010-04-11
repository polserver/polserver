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
#include "item/itemdesc.h"
#include "msghandl.h"
#include "pktboth.h"
#include "pktin.h"
#include "ssopt.h"
#include "ufunc.h"
#include "uvars.h"

#include "tooltips.h"


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
				PktOut_B7* msg = REQUESTPACKET(PktOut_B7,PKTOUT_B7_ID);
				msg->offset+=2;
				msg->Write(item->serial_ext);
				for( unsigned i = 0; i < id.tooltip.length(); ++i )
				{
					msg->offset++;
					msg->Write(static_cast<u8>(id.tooltip[ i ]));
				}
				msg->offset+=2; //nullterm
				u16 len=msg->offset;
				msg->offset=1;
				msg->WriteFlipped(len);

                client->transmit( &msg->offset, len );
				READDPACKET(msg);
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
			PktOut_DC* msgdc = REQUESTPACKET(PktOut_DC,PKTOUT_DC_ID);
			msgdc->Write(obj->serial_ext);
			msgdc->WriteFlipped(obj->rev());
			client->transmit(&msgdc->buffer, msgdc->offset);
			READDPACKET(msgdc);
		}
		else
		{
			PktOut_BF_Sub10* msgbf10 = REQUESTSUBPACKET(PktOut_BF_Sub10,PKTBI_BF_ID,PKTBI_BF::TYPE_OBJECT_CACHE);
			msgbf10->WriteFlipped(static_cast<u16>(0xD));
			msgbf10->WriteFlipped(msgbf10->getSubID());
			msgbf10->Write(obj->serial_ext);
			msgbf10->WriteFlipped(obj->rev());
			client->transmit(&msgbf10->buffer, msgbf10->offset);
			READDPACKET(msgbf10);
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
		PktOut_DC* msgdc = REQUESTPACKET(PktOut_DC,PKTOUT_DC_ID);
		PktOut_BF_Sub10* msgbf10 = REQUESTSUBPACKET(PktOut_BF_Sub10,PKTBI_BF_ID,PKTBI_BF::TYPE_OBJECT_CACHE);

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
						client2->transmit(&msgdc->buffer, msgdc->offset);
						
					}
					else
					{
						if (msgbf10->offset==1)
						{
							msgbf10->WriteFlipped(static_cast<u16>(0xD));
							msgbf10->WriteFlipped(msgbf10->getSubID());
							msgbf10->Write(obj->serial_ext);
							msgbf10->WriteFlipped(obj->rev());
						}
						client2->transmit(&msgbf10->buffer, msgbf10->offset);
					}
				}
			}
		}
		READDPACKET(msgdc);
		READDPACKET(msgbf10);
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

    int len = desc.size()*2;
    string out(len,0);
    
    int packetsize = 25+len;

    for(unsigned int i=0,j=0; j<desc.size(); i++,j++)
    {
        out[i] = desc[j];
        i++;
        out[i] = 0;
    }

    unsigned char* buffer = new unsigned char[packetsize];
    PKTBI_D6_OUT* msg = reinterpret_cast<PKTBI_D6_OUT*>(buffer);
    msg->msgtype = PKTBI_D6_OUT_ID;
    msg->msglen = ctBEu16(packetsize);
    msg->unk1 = ctBEu16(1);
    msg->serial = obj->serial_ext;
    msg->unk2 = 0;
    msg->unk3 = 0;
    msg->listid = ctBEu32(obj->rev());

	if(obj->isa(UObject::CLASS_CHARACTER))
	{
		msg->data->localization_num = ctBEu32(1050045); //1 text argument only
	}
	else
		msg->data->localization_num = ctBEu32(1042971); //1 text argument only

	msg->data->textlen = ctBEu16(len);
	memcpy( reinterpret_cast<u8*>(&(msg->data->text)), out.c_str(), len );
    *(reinterpret_cast<u32*>(&buffer[packetsize-4])) = 0; //indicates end of property list

    client->transmit(buffer,packetsize);
	delete [] buffer;
	buffer = NULL;
}

