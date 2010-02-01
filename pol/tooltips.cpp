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
                static PKTOUT_B7 msg;
                msg.msgtype = PKTOUT_B7_ID;
				u16 msglen = static_cast<u16>(offsetof( PKTOUT_B7, text ) +
                             id.tooltip.length() * 2 + 2);
                msg.msglen = ctBEu16( msglen );
                msg.serial = item->serial_ext;
                for( unsigned i = 0; i < id.tooltip.length(); ++i )
                {
                    msg.text[ i*2 ] = '\0';
                    msg.text[ i*2 + 1 ] = id.tooltip[ i ];
                }
                msg.text[ id.tooltip.length() * 2 ] = '\0';
                msg.text[ id.tooltip.length() * 2 + 1 ] = '\0';
                client->transmit( &msg, msglen );
            }
        }
    }
}
MESSAGE_HANDLER( PKTIN_B6, handle_request_tooltip );

//needed if A9 flag is sent with 0x20, single click no longer works. see about text# 1042971 for 0xD6
void send_object_cache(Client* client, const UObject* obj)
{
	if (ssopt.uo_feature_enable & PKTOUT_A9_START_FLAGS::FLAG_AOS_FEATURES)
	{
		if ((ssopt.force_new_objcache_packets) || (client->ClientType & CLIENTTYPE_5000))
		{
			unsigned char* uokr_buffer = BuildObjCache(obj, true);
			client->transmit(uokr_buffer, 9);
			delete [] uokr_buffer;
			uokr_buffer = NULL;
		}
		else
		{
			unsigned char* aos_buffer = BuildObjCache(obj, false);
			client->transmit(aos_buffer, 13);
			delete [] aos_buffer;
			aos_buffer = NULL;
		}
	}
}

void send_object_cache_to_inrange(const UObject* obj)
{
	if (ssopt.uo_feature_enable & PKTOUT_A9_START_FLAGS::FLAG_AOS_FEATURES)
	{
		// Since this is an InRange function, at least 1 person. So it isn't too far
		// fetched to build for AOS and UOKR both, since both could be used. At least
		// one will always be used, and this really makes a different in large groups.
		unsigned char* aos_buffer = NULL;
		unsigned char* uokr_buffer = NULL;
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
						if (uokr_buffer == NULL)
							uokr_buffer = BuildObjCache(obj, true);
						client2->transmit(uokr_buffer, 9);
					}
					else
					{
						if (aos_buffer == NULL)
							aos_buffer = BuildObjCache(obj, false);
						client2->transmit(aos_buffer, 13);
					}
				}
			}
		}
		if (uokr_buffer != NULL)
		{
			delete [] uokr_buffer;
			uokr_buffer = NULL;
		}
		if (aos_buffer != NULL)
		{
			delete [] aos_buffer;
			aos_buffer = NULL;
		}

	}
}

unsigned char* BuildObjCache (const UObject* obj, bool new_style)
{
	if (new_style)
	{
	    unsigned char* buffer = new unsigned char[9];
	    PKTOUT_DC* msg = reinterpret_cast<PKTOUT_DC*>(buffer);
		msg->msgtype = PKTOUT_DC_ID;
		msg->serial = obj->serial_ext;
		msg->revision = ctBEu32(obj->rev());
		return buffer;
	}
	else
	{
	    unsigned char* buffer = new unsigned char[13];
	    PKTBI_BF* msg = reinterpret_cast<PKTBI_BF*>(buffer);
		msg->msgtype = PKTBI_BF_ID;
		msg->msglen = ctBEu16(0xD);
		msg->subcmd = ctBEu16(PKTBI_BF::TYPE_OBJECT_CACHE);
		msg->objectcache.serial = obj->serial_ext;
		msg->objectcache.listid = ctBEu32(obj->rev());
		return buffer;
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

