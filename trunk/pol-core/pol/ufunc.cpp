/*
History
=======
2005/03/05 Shinigami: format_description -> ServSpecOpt UseAAnTileFlags to enable/disable "a"/"an" via Tiles.cfg in formatted Item Names
2005/04/03 Shinigami: send_feature_enable - added UOExpansionFlag for Samurai Empire
2005/08/29 Shinigami: ServSpecOpt UseAAnTileFlags renamed to UseTileFlagPrefix
2005/09/17 Shinigami: send_nametext - smaller bugfix in passert-check
2006/05/07 Shinigami: SendAOSTooltip - will now send merchant_description() if needed
2006/05/16 Shinigami: send_feature_enable - added UOExpansionFlag for Mondain's Legacy
2007/04/08 MuadDib:   Updated send_worn_item_to_inrange to create the message only
                      once and use the Transmit_to_inrange instead. Then uses 
                      send_object_cache_to_inrange. Required a tooltips.* update.
2008/07/08 Turley:    get_flag1() changed to show WarMode of other player again
                      get_flag1_aos() removed
2009/07/23 MuadDib:   updates for new Enum::Packet Out ID
2009/07/31 Turley:    added send_fight_occuring() for packet 0x2F
2009/08/01 MuadDib:   Removed send_tech_stuff(), unused and obsolete.
2009/08/09 MuadDib:   UpdateCharacterWeight() Rewritten to send stat msg intead of refreshar().
                      Refactor of Packet 0x25 for naming convention
2009/08/14 Turley:    PKTOUT_B9_V2 removed unk u16 and changed flag to u32
2009/09/03 MuadDib:   Relocation of account related cpp/h
                      Relocation of multi related cpp/h
2009/09/13 MuadDib:   Optimized send_remove_character_to_nearby_cansee, send_remove_character_to_nearby_cantsee, send_remove_character_to_nearby
                      to build packet and handle iter internally. Packet built just once this way, and sent to those who need it.
2009/09/06 Turley:    Changed Version checks to bitfield client->ClientType
2009/09/22 MuadDib:   Adding resending of light level if override not in effect, to sending of season packet. Fixes EA client bug.
2009/09/22 Turley:    Added DamagePacket support
2009/10/07 Turley:    Fixed DestroyItem while in hand
2009/10/12 Turley:    whisper/yell/say-range ssopt definition
2009/12/02 Turley:    0xf3 packet support - Tomi
                      face support
2009/12/03 Turley:    added 0x17 packet everywhere only send if poisoned, fixed get_flag1 (problem with poisoned & flying)

Notes
=======

*/

#include "../clib/stl_inc.h"

#include <assert.h>
#include <stddef.h>

#include "../clib/clib.h"
#include "../clib/endian.h"
#include "../clib/logfile.h"
#include "../clib/passert.h"
#include "../clib/pkthelper.h"
#include "../clib/stlutil.h"
#include "../clib/strutil.h"

#include "../plib/mapcell.h"
#include "../plib/realm.h"

#include "accounts/account.h"
#include "mobile/charactr.h"
#include "network/client.h"
#include "network/packets.h"
#include "item/equipmnt.h"
#include "fnsearch.h"
#include "layers.h"
#include "lightlvl.h"
#include "objtype.h"
#include "pktout.h"
#include "polcfg.h"
#include "polclass.h"
#include "realms.h"
#include "repsys.h"
#include "sockio.h"
#include "ssopt.h"
#include "statmsg.h"
#include "tmpvars.h"
#include "tooltips.h"
#include "uconst.h"
#include "ufunc.h"
#include "multi/multi.h"
#include "uobject.h"
#include "uoclient.h"
#include "uofile.h"
#include "ustruct.h"
#include "uvars.h"
#include "uworld.h"

#include "../bscript/impstr.h"

#include "objecthash.h"

u32 itemserialnumber = ITEMSERIAL_START;
u32 charserialnumber = CHARACTERSERIAL_START;

//Dave added 3/9/3
void SetCurrentItemSerialNumber(u32 serial)
{
	itemserialnumber = serial;
}

//Dave added 3/9/3
void SetCurrentCharSerialNumber(u32 serial)
{
	charserialnumber = serial;
}

//Dave added 3/8/3
u32 GetCurrentItemSerialNumber(void)
{
	return itemserialnumber;
}

//Dave added 3/8/3
u32 GetCurrentCharSerialNumber(void)
{
	return charserialnumber;
}

//Dave changed 3/8/3 to use objecthash
u32 GetNextSerialNumber(void)
{
	u32 nextserial = objecthash.GetNextUnusedCharSerial();
	charserialnumber = nextserial;
	return charserialnumber;
}

u32 UseCharSerialNumber( u32 serial )
{
    if (serial > charserialnumber)
        charserialnumber = serial+1;
    return serial;
}

//Dave changed 3/8/3
u32 UseItemSerialNumber( u32 serial )
{	
	if (serial > itemserialnumber)
        itemserialnumber = serial+1;
    return serial;
}

//Dave changed 3/8/3 to use objecthash
u32 GetNewItemSerialNumber( void )
{
	u32 nextserial = objecthash.GetNextUnusedItemSerial();
	itemserialnumber = nextserial;
	return itemserialnumber;
}

void send_goxyz( Client *client, const Character *chr )
{
	PktOut_20* msg = REQUESTPACKET(PktOut_20,PKTOUT_20_ID);
	msg->Write(chr->serial_ext);
	msg->Write(chr->graphic_ext);
	msg->offset++; //unk7
	msg->Write(chr->color_ext);
	msg->Write(chr->get_flag1(client));
	msg->WriteFlipped(chr->x);
	msg->WriteFlipped(chr->y);
	msg->offset+=2; //unk15,16
	msg->Write(static_cast<u8>(0x80 | chr->facing)); // is it always right to set this flag?
	msg->Write(chr->z);
    transmit( client, &msg->buffer, msg->offset );
	READDPACKET(msg);

    if ((client->ClientType & CLIENTTYPE_UOKR) && (chr->poisoned)) //if poisoned send 0x17 for newer clients
        send_poisonhealthbar( client, chr );
}

// Character chr has moved.  Tell a client about it.
// FIXME: Use of this packet needs optimized. Other functions for
// sending to in range etc, call this function for each client.
// Even with the independant flags, we should be able to
// optimize this out to reduce build amounts
void send_move( Client *client, const Character *chr )
{
	PktOut_77* msg = REQUESTPACKET(PktOut_77,PKTOUT_77_ID);
	msg->Write(chr->serial_ext);
	msg->Write(chr->graphic_ext);
	msg->WriteFlipped(chr->x);
	msg->WriteFlipped(chr->y);
	msg->Write(chr->z);
	msg->Write(static_cast<u8>((chr->dir & 0x80) | chr->facing));// NOTE, this only includes mask 0x07 of the last MOVE message 
	msg->Write(chr->color_ext);
	msg->Write(chr->get_flag1(client));
	msg->Write(chr->hilite_color_idx( client->chr ));

    transmit( client, &msg->buffer, msg->offset );
	READDPACKET(msg);

    if ((client->ClientType & CLIENTTYPE_UOKR) && (chr->poisoned)) //if poisoned send 0x17 for newer clients
        send_poisonhealthbar( client, chr );
}

void send_poisonhealthbar( Client *client, const Character *chr )
{
	PktOut_17* msg = REQUESTPACKET(PktOut_17,PKTOUT_17_ID);
	msg->WriteFlipped(static_cast<u16>(sizeof msg->buffer));
	msg->Write(chr->serial_ext);
	msg->Write(static_cast<u16>(1)); //unk
	msg->Write(static_cast<u16>(1)); // status_type
	msg->Write(static_cast<u8>(( chr->poisoned ) ? 1 : 0)); //flag
	transmit( client, &msg->buffer, msg->offset );
	READDPACKET(msg);
}

void send_owncreate( Client *client, const Character *chr )
{
	PktOut_78* owncreate = REQUESTPACKET(PktOut_78,PKTOUT_78_ID);
	owncreate->offset+=2;
	owncreate->Write(chr->serial_ext);
	owncreate->Write(chr->graphic_ext);
	owncreate->WriteFlipped(chr->x);
	owncreate->WriteFlipped(chr->y);
	owncreate->Write(chr->z);
	owncreate->Write(chr->facing);
	owncreate->Write(chr->color_ext);
	owncreate->Write(chr->get_flag1(client));
	owncreate->Write(chr->hilite_color_idx( client->chr ));
    
    for( int layer = LAYER_EQUIP__LOWEST; layer <= LAYER_EQUIP__HIGHEST; ++layer )
    {
        const Item *item = chr->wornitem( layer );
        if (item == NULL) 
            continue;

        // Dont send faces if older client or ssopt
        if ((layer==LAYER_FACE) && ((ssopt.support_faces==0) || (~client->ClientType & CLIENTTYPE_UOKR)))
            continue;

        if (item->color)
        {
			owncreate->Write(item->serial_ext);
			owncreate->WriteFlipped(static_cast<u16>(0x8000 | item->graphic));
			owncreate->Write(static_cast<u8>(layer));
            owncreate->Write(item->color_ext);
        }
        else
        {
			owncreate->Write(item->serial_ext);
			owncreate->Write(item->graphic_ext);
			owncreate->Write(static_cast<u8>(layer));
        }
    }
	owncreate->offset += 4; //items nullterm
	u16 len = owncreate->offset;
	owncreate->offset = 1;
	owncreate->WriteFlipped(len);

    transmit(client, &owncreate->buffer, len );
	READDPACKET(owncreate);

	if(client->UOExpansionFlag & AOS)
	{
		send_object_cache(client, dynamic_cast<const UObject*>(chr));
		// 07/11/09 Turley: moved to bottom first the client needs to know the item then we can send revision
		for( int layer = LAYER_EQUIP__LOWEST; layer <= LAYER_EQUIP__HIGHEST; ++layer )
		{
			const Item *item = chr->wornitem( layer );
			if (item == NULL) 
				continue;
            if (layer == LAYER_FACE)
                continue;
			send_object_cache(client, dynamic_cast<const UObject*>(item));
		}
	}

    if ((client->ClientType & CLIENTTYPE_UOKR) && (chr->poisoned)) //if poisoned send 0x17 for newer clients
        send_poisonhealthbar( client, chr );
}


void send_move_if_inrange( Client *client, const Character *chr )
{
    if (client->ready &&
        client->chr &&
        client->chr != chr &&
        inrange( client->chr, chr ))
    {
        send_move( client, chr );
    }
}

void send_remove_character( Client *client, const Character *chr )
{
    if (!client->ready)     /* if a client is just connecting, don't bother him. */
        return;

        /* Don't remove myself */
    if (client->chr == chr)
        return;

	PktOut_1D* msgremove = REQUESTPACKET(PktOut_1D,PKTOUT_1D_ID);
	msgremove->Write(chr->serial_ext);
    transmit( client, &msgremove->buffer, msgremove->offset );
	READDPACKET(msgremove);
}


void send_remove_character_if_nearby( Client* client, const Character* chr )
{
    if (!client->ready)     /* if a client is just connecting, don't bother him. */
        return;

    if (!inrange( client->chr, chr))
        return;

    if (client->chr == chr)
        return;

	PktOut_1D* msgremove = REQUESTPACKET(PktOut_1D,PKTOUT_1D_ID);
	msgremove->Write(chr->serial_ext);
	transmit( client, &msgremove->buffer, msgremove->offset );
	READDPACKET(msgremove);
}
void send_remove_character_to_nearby( const Character* chr )
{
	PktOut_1D* msgremove = REQUESTPACKET(PktOut_1D,PKTOUT_1D_ID);
	msgremove->Write(chr->serial_ext);

	for( Clients::iterator itr = clients.begin(), end = clients.end(); itr != end; ++itr )
	{
		Client *client = *itr;
		if (!client->ready)     /* if a client is just connecting, don't bother him. */
			continue;

		if (!inrange( client->chr, chr))
			continue;

		if (client->chr == chr)
			continue;

		transmit( client, &msgremove->buffer, msgremove->offset );
	}
	READDPACKET(msgremove);
}

void send_remove_character_if_nearby_cantsee( Client* client, const Character* chr )
{
    if (!client->ready)     /* if a client is just connecting, don't bother him. */
        return;

    if (!inrange( client->chr, chr))
        return;

    if (client->chr == chr)
        return;

    if (!client->chr->is_visible_to_me( chr ))
    {
		PktOut_1D* msgremove = REQUESTPACKET(PktOut_1D,PKTOUT_1D_ID);
		msgremove->Write(chr->serial_ext);
		transmit( client, &msgremove->buffer, msgremove->offset );
		READDPACKET(msgremove);
    }
}
void send_remove_character_to_nearby_cantsee( const Character* chr )
{
	PktOut_1D* msgremove = REQUESTPACKET(PktOut_1D,PKTOUT_1D_ID);
	msgremove->Write(chr->serial_ext);

	for( Clients::iterator itr = clients.begin(), end = clients.end(); itr != end; ++itr )
	{
		Client *client = *itr;
		if (!client->ready)     /* if a client is just connecting, don't bother him. */
			continue;

		if (!inrange( client->chr, chr))
			continue;

		if (client->chr == chr)
			continue;

		if (!client->chr->is_visible_to_me( chr ))
		{
			transmit( client, &msgremove->buffer, msgremove->offset );
		}
	}
	READDPACKET(msgremove);
}

// FIXME: This is depreciated unless we find a new place to use it.
void send_remove_character_if_nearby_cansee( Client* client, const Character* chr )
{
    if (client->ready &&
        inrange( client->chr, chr) &&
        client->chr != chr &&
        client->chr->is_visible_to_me( chr ))
    {
		PktOut_1D* msgremove = REQUESTPACKET(PktOut_1D,PKTOUT_1D_ID);
		msgremove->Write(chr->serial_ext);
		transmit( client, &msgremove->buffer, msgremove->offset );
		READDPACKET(msgremove);
    }
}

void send_remove_character_to_nearby_cansee( const Character* chr )
{
	PktOut_1D* msgremove = REQUESTPACKET(PktOut_1D,PKTOUT_1D_ID);
	msgremove->Write(chr->serial_ext);

	for( Clients::iterator itr = clients.begin(), end = clients.end(); itr != end; ++itr )
	{
		Client *client = *itr;
		if (client->ready &&
			inrange( client->chr, chr) &&
			client->chr != chr &&
			client->chr->is_visible_to_me( chr ))
		{
			transmit( client, &msgremove->buffer, msgremove->offset );
		}
	}
	READDPACKET(msgremove);
}



void send_remove_object_if_inrange( Client *client, const Item *item )
{
    if (!client->ready)     /* if a client is just connecting, don't bother him. */
        return;

    if (!inrange( client->chr, item))
        return;

	PktOut_1D* msgremove = REQUESTPACKET(PktOut_1D,PKTOUT_1D_ID);
	msgremove->Write(item->serial_ext);
	transmit( client, &msgremove->buffer, msgremove->offset );
	READDPACKET(msgremove);
}

void send_remove_object_to_inrange( const UObject *centerObject )
{
	PktOut_1D* msgremove = REQUESTPACKET(PktOut_1D,PKTOUT_1D_ID);
	msgremove->Write(centerObject->serial_ext);

	for( Clients::iterator itr = clients.begin(), end = clients.end(); itr != end; ++itr )
	{
        Client *client2 = *itr;
	    if (!client2->ready)
		    continue;
	    // FIXME need to check character's additional_legal_items.
	    // looks like inrange should be a Character member function.
		if (inrange(client2->chr, centerObject))
		{
			transmit( client2, &msgremove->buffer, msgremove->offset );
		}
	}
	READDPACKET(msgremove);
}

void send_remove_object( Client *client, const Item *item )
{
	PktOut_1D* msgremove = REQUESTPACKET(PktOut_1D,PKTOUT_1D_ID);
	msgremove->Write(item->serial_ext);
	transmit( client, &msgremove->buffer, msgremove->offset );
	READDPACKET(msgremove);
}

bool inrangex( const Character *c1, const Character *c2, int maxdist )
{
    return ( (c1->realm == c2->realm) &&
			 (abs( c1->x - c2->x ) <= maxdist) &&
             (abs( c1->y - c2->y ) <= maxdist) );
}

bool inrangex( const UObject *c1, unsigned short x, unsigned short y, int maxdist )
{
    return ( (abs( c1->x - x ) <= maxdist) &&
             (abs( c1->y - y ) <= maxdist) );
}

bool inrange( const UObject *c1, unsigned short x, unsigned short y )
{
    return ( (abs( c1->x - x ) <= RANGE_VISUAL) &&
             (abs( c1->y - y ) <= RANGE_VISUAL) );
}

bool inrange( const Character *c1, const Character *c2 )
{
    // note, these are unsigned.  abs converts to signed, so everything _should_ be okay.
    return ( (c1->realm == c2->realm) &&
			 (abs( c1->x - c2->x ) <= RANGE_VISUAL) &&
             (abs( c1->y - c2->y ) <= RANGE_VISUAL) );
}

bool inrange( const Character *c1, const UObject *obj )
{
    obj = obj->toplevel_owner();
    
    return ( (c1->realm == obj->realm) &&
			 (abs( c1->x - obj->x ) <= RANGE_VISUAL) &&
             (abs( c1->y - obj->y ) <= RANGE_VISUAL) );
}

bool multi_inrange( const Character *c1, const UMulti *obj )
{
    return ( (c1->realm == obj->realm) &&
			 (abs( c1->x - obj->x ) <= RANGE_VISUAL_LARGE_BUILDINGS) &&
             (abs( c1->y - obj->y ) <= RANGE_VISUAL_LARGE_BUILDINGS) );
}

unsigned short pol_distance( unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2 )
{
    int xd = abs( x1 - x2 );
    int yd = abs( y1 - y2 );
    if (xd > yd)
        return static_cast<unsigned short>(xd);
    else
        return static_cast<unsigned short>(yd);
}

unsigned short pol_distance( const Character* c1, const UObject* obj )
{
    obj = obj->toplevel_owner();

    int xd = abs( c1->x - obj->x );
    int yd = abs( c1->y - obj->y );
    if (xd > yd)
        return static_cast<unsigned short>(xd);
    else
        return static_cast<unsigned short>(yd);
}

bool in_say_range( const Character *c1, const Character *c2 )
{
    return inrangex( c1, c2, ssopt.speech_range );
}
bool in_yell_range( const Character *c1, const Character *c2 )
{
    return inrangex( c1, c2, ssopt.yell_range );
}
bool in_whisper_range( const Character *c1, const Character *c2 )
{
    return inrangex( c1, c2, ssopt.whisper_range );
}

bool inrange( unsigned short x1, unsigned short y1,
             unsigned short x2, unsigned short y2 )
{
    return ( (abs( x1-x2 ) <= RANGE_VISUAL ) &&
             (abs( y1-y2 ) <= RANGE_VISUAL ) );
}

bool multi_inrange( unsigned short x1, unsigned short y1,
             unsigned short x2, unsigned short y2 )
{
    return ( (abs( x1-x2 ) <= RANGE_VISUAL_LARGE_BUILDINGS ) &&
             (abs( y1-y2 ) <= RANGE_VISUAL_LARGE_BUILDINGS ) );
}

void send_put_in_container( Client* client, const Item* item )
{
	PktOut_25* msg = REQUESTPACKET(PktOut_25,PKTOUT_25_ID);
	msg->Write(item->serial_ext);
	msg->Write(item->graphic_ext);
	msg->offset++; //unk7 layer?
	msg->WriteFlipped(item->get_senditem_amount());
	msg->WriteFlipped(item->x);
	msg->WriteFlipped(item->y);
	if ( client->ClientType & CLIENTTYPE_6017 )
		msg->Write(item->slot_index());
	msg->Write(item->container->serial_ext);
	msg->Write(item->color_ext);
	transmit( client, &msg->buffer, msg->offset );
	READDPACKET(msg);

	if(client->UOExpansionFlag & AOS)
	    send_object_cache(client, dynamic_cast<const UObject*>(item));
}

void send_put_in_container_to_inrange( const Item *item )
{
	// FIXME there HAS to be a better, more efficient way than this, without
	// building these with pointer and NULL to check. Cuz that method requires
	// recast and delete. Ewwww.
	PktOut_25* legacy_buffer = REQUESTPACKET(PktOut_25,PKTOUT_25_ID);
	PktOut_25* slot_buffer = REQUESTPACKET(PktOut_25,PKTOUT_25_ID);

    for( Clients::iterator itr = clients.begin(), end = clients.end(); itr != end; ++itr )
    {
        Client *client2 = *itr;
        if (!client2->ready)
            continue;
        // FIXME need to check character's additional_legal_items.
        // looks like inrange should be a Character member function.
        if (client2->chr->mightsee( item->container ))
        {
            // FIXME if the container has an owner, and I'm not it, don't tell me?
			if ( client2->ClientType & CLIENTTYPE_6017 )
			{
				if (slot_buffer->offset==1)
                {
					slot_buffer->Write(item->serial_ext);
					slot_buffer->Write(item->graphic_ext);
					slot_buffer->offset++; //unk7 layer?
					slot_buffer->WriteFlipped(item->get_senditem_amount());
					slot_buffer->WriteFlipped(item->x);
					slot_buffer->WriteFlipped(item->y);
					slot_buffer->Write(item->slot_index());
					slot_buffer->Write(item->container->serial_ext);
					slot_buffer->Write(item->color_ext);
                }
				client2->transmit(&slot_buffer->buffer, slot_buffer->offset);
			}
			else
			{
				if (legacy_buffer->offset==1)
                {
					legacy_buffer->Write(item->serial_ext);
					legacy_buffer->Write(item->graphic_ext);
					legacy_buffer->offset++; //unk7 layer?
					legacy_buffer->WriteFlipped(item->get_senditem_amount());
					legacy_buffer->WriteFlipped(item->x);
					legacy_buffer->WriteFlipped(item->y);
					legacy_buffer->Write(item->container->serial_ext);
					legacy_buffer->Write(item->color_ext);
                }
				client2->transmit(&legacy_buffer->buffer, legacy_buffer->offset);
			}
			if(client2->UOExpansionFlag & AOS)
			{
				send_object_cache(client2, dynamic_cast<const UObject*>(item));
			}
        }
    }
	READDPACKET(legacy_buffer);
	READDPACKET(slot_buffer);
}

// FIXME it would be better to compose this message once and
// send to multiple clients.
// Uh, WTF.  Looks like we have to send a full "container contents"
// message, just to get the clothes on the corpse - without an
// 'open gump' message of course
void send_corpse_items( Client *client, const Item *item )
{
    const UContainer *cont = static_cast<const UContainer *>(item);

	PktOut_89* msg = REQUESTPACKET(PktOut_89,PKTOUT_89_ID);
	msg->offset+=2;
	msg->Write(item->serial_ext);

    int n_layers_found = 0;
    for( UContainer::const_iterator itr = cont->begin(); itr != cont->end(); ++itr )
    {
        const Item *item2 = GET_ITEM_PTR( itr );
		if (item2->layer == 0)
			continue;

        // FIXME it looks like it's possible for a corpse to have more than NUM_LAYERS items that specify a layer.
        // probably by dropping something onto a corpse
        if (n_layers_found >= NUM_LAYERS)
        {
            static u32 last_serial = 0;
            if (item->serial != last_serial)
            {
                Log2( "Too many items specify a layer on corpse 0x%lx\n", item->serial );
                last_serial = item->serial;
            }
            break;
        }
		msg->Write(item2->layer);
		msg->Write(item2->serial_ext);
        n_layers_found++;
    }
    passert_always( n_layers_found <= NUM_LAYERS );
	msg->offset+=1; // nullterm byte
	u16 len= msg->offset;
	msg->offset=1;
	msg->WriteFlipped(len);

    transmit( client, &msg->buffer, len );
    READDPACKET(msg);

    send_container_contents( client, *cont, true );
}

// Item::sendto( Client* ) ??
void send_item( Client *client, const Item *item )
{
    // printf( "Sending Item %08.08lX to Character %08.08lX\n", 
    //         item->serial, client->chr->serial );

    if (item->invisible() && !client->chr->can_seeinvisitems())
    {
        send_remove_object( client, item );
        return;
    }

    u8 flags = 0;
    if (client->chr->can_move( item ))
        flags |= ITEM_FLAG_FORCE_MOVABLE;
	if (client->ClientType & CLIENTTYPE_7000)
	{
		// Client >= 7.0.0.0 ( SA )
		PktOut_F3* msg = REQUESTPACKET(PktOut_F3,PKTOUT_F3_ID);
		msg->WriteFlipped(static_cast<u16>(0x1));
		msg->offset++; // datatype
		msg->Write(item->serial_ext);
		msg->Write(item->graphic_ext);
		msg->Write(item->facing);
		msg->WriteFlipped(item->get_senditem_amount());
		msg->WriteFlipped(item->get_senditem_amount());
		msg->WriteFlipped(item->x);
		msg->WriteFlipped(item->y);
		msg->Write(item->z);
		msg->Write(item->layer);
		msg->Write(item->color_ext);
		msg->Write(flags);
		transmit( client, &msg->buffer, msg->offset );
		READDPACKET(msg);
	}
	else
	{
		PktOut_1A* msg = REQUESTPACKET(PktOut_1A,PKTOUT_1A_ID);
		// transmit item info
		msg->offset+=2;
		// If the 0x80000000 is left out, the item won't show up. 
		msg->WriteFlipped(static_cast<u32>(0x80000000 | item->serial)); // bit 0x80000000 enables piles
		msg->Write(item->graphic_ext);
		msg->WriteFlipped(item->get_senditem_amount());
		if (item->facing == 0)
		{
			msg->WriteFlipped(item->x);
			// bits 0x80 and 0x40 are Dye and Move (dunno which is which)
			msg->WriteFlipped(static_cast<u16>(0xC000 | item->y)); // dyeable and moveable?
		}
		else
		{
			msg->WriteFlipped(static_cast<u16>(0x8000 |item->x));
			// bits 0x80 and 0x40 are Dye and Move (dunno which is which)
			msg->WriteFlipped(static_cast<u16>(0xC000 | item->y)); // dyeable and moveable?
			msg->Write(item->facing);
		}
		msg->Write(item->z);
		msg->Write(item->color_ext);
		msg->Write(flags);
		u16 len=msg->offset;
		msg->offset=1;
		msg->WriteFlipped(len);
		transmit( client, &msg->buffer, len );
		READDPACKET(msg);
	}
    
    // if the item is a corpse, transmit items contained by it
    if (item->objtype_ == UOBJ_CORPSE)
    {
        send_corpse_items( client, item );
    }

	if(client->UOExpansionFlag & AOS)
	{
		send_object_cache(client, dynamic_cast<const UObject*>(item));
		return;
	}
}

/* Tell all clients new information about an item */
// FIXME OPTIMIZE: OMG, EVERY TIME IT CALLS SEND_ITEM!
void send_item_to_inrange( const Item *item )
{
    // FIXME could use transmit_to_inrange, almost.
    // (Character-specific flags, like can_move(), make it so we can't)
	// However, could build main part of packet before for/loop, then
	// adjust per client. Would this be a better solution?
    for( Clients::iterator itr = clients.begin(), end = clients.end(); itr != end; ++itr )
    {
        Client *client = *itr;
        if (!client->ready)
            continue;
        if (inrange( client->chr, item ))
            send_item( client, item );
    }
}


void update_item_to_inrange( const Item* item )
{
    if (item->container != NULL)
    {
        if (IsCharacter( item->container->serial ))
        {
            // this may not be the right thing in all cases.
            // specifically, handle_dye used to not ever do send_wornitem.
            // FIXME way, way inefficient, but nontrivial.
            Character* chr = find_character( item->container->serial );
			if (chr)
			{
				update_wornitem_to_inrange( chr, item );
			}
            else
                Log( "Ack! update_item_to_inrange: character 0x%lx doesn't exist!\n", 
                     item->container->serial );
        }
        else
        {
            send_put_in_container_to_inrange( item );
        }
    }
    else
    {
        send_item_to_inrange( item ); 
    }
}

void send_light( Client *client, int lightlevel )
{
    if (VALID_LIGHTLEVEL( lightlevel ))
    {
		PktOut_4F* msg = REQUESTPACKET(PktOut_4F,PKTOUT_4F_ID);
		msg->Write(static_cast<u8>(lightlevel));
		transmit( client, &msg->buffer, msg->offset );
		READDPACKET(msg);
    }
}

void send_weather( Client* client, u8 type, u8 severity, u8 aux )
{
	PktOut_65* msg = REQUESTPACKET(PktOut_65,PKTOUT_65_ID);
	msg->Write(type);
	msg->Write(severity);
	msg->Write(aux);
    transmit( client, &msg->buffer, msg->offset );
	READDPACKET(msg);
}

/* send_char_data: called once for each client when a new character enters
   the world. */
void send_char_data( Client *client, Character *chr )
{
    if (!client->ready) 
        return;
    
    if (!client->chr->is_visible_to_me( chr ))
        return;

    if (inrange( client->chr, chr ))
    {
        send_owncreate( client, chr );
    }
}

/*
void send_char_data_to_ghosts( Client* client, Character* chr )
{
    if (client->ready &&                // must be logged into game
        inrange( client->chr, chr ) &&
        client->chr != chr &&
        client->chr->is_visible_to_me( chr ))
    {
        send_owncreate( client, chr );
    }
}

void send_char_data_to_nearby_ghosts( Character* chr )
{
    ForEach( clients, send_char_data_to_ghosts, chr );
}
*/

/* send_client_char_data: called once for each character when a client
   logs on.  If in range, tell the client about each character. */
void send_client_char_data( Character *chr, Client *client )
{
                // Don't tell a client about its own character.
    if (client->chr == chr)
        return; 

    if (!client->chr->is_visible_to_me( chr ))
        return;

    if (inrange( client->chr, chr ))
    {
        send_owncreate( client, chr );
    }
}

void send_item_move_failure( Client *client, u8 reason )
{
	PktOut_27* msg = REQUESTPACKET(PktOut_27,PKTOUT_27_ID);
	msg->Write(reason);
    transmit( client, &msg->buffer, msg->offset );
	READDPACKET(msg);
}

void send_wornitem( Client *client, const Character *chr, const Item *item )
{
	PktOut_2E* msg = REQUESTPACKET(PktOut_2E,PKTOUT_2E_ID);
	msg->Write(item->serial_ext);
	msg->Write(item->graphic_ext);
	msg->offset++; //unk7
	msg->Write(item->layer);
	msg->Write(chr->serial_ext);
	msg->Write(item->color_ext);
	transmit( client, &msg->buffer, msg->offset );
	READDPACKET(msg);

	if(client->UOExpansionFlag & AOS)
	{
		send_object_cache(client, dynamic_cast<const UObject*>(item));
	}
}

void send_wornitem_to_inrange( const Character *chr, const Item *item )
{
	PktOut_2E* msg = REQUESTPACKET(PktOut_2E,PKTOUT_2E_ID);
	msg->Write(item->serial_ext);
	msg->Write(item->graphic_ext);
	msg->offset++; //unk7
	msg->Write(item->layer);
	msg->Write(chr->serial_ext);
	msg->Write(item->color_ext);
	transmit_to_inrange( item, &msg->buffer, msg->offset, false, false );
	READDPACKET(msg);
	send_object_cache_to_inrange( dynamic_cast<const UObject*>(item) );
}

// This used when item already worn and graphic/color changed. Deletes the item
// at client and then sends the new information.
void update_wornitem_to_inrange( const Character *chr, const Item *item )
{
	if(chr != NULL)
	{
		PktOut_1D* msgremove = REQUESTPACKET(PktOut_1D,PKTOUT_1D_ID);
		msgremove->Write(item->serial_ext);
		transmit_to_inrange( item, &msgremove->buffer, msgremove->offset, false, false );
		READDPACKET(msgremove);

		PktOut_2E* msg = REQUESTPACKET(PktOut_2E,PKTOUT_2E_ID);
		msg->Write(item->serial_ext);
		msg->Write(item->graphic_ext);
		msg->offset++; //unk7
		msg->Write(item->layer);
		msg->Write(chr->serial_ext);
		msg->Write(item->color_ext);
		transmit_to_inrange( item, &msg->buffer, msg->offset, false, false );
		READDPACKET(msg);

		send_object_cache_to_inrange( dynamic_cast<const UObject*>(item) );
	}
}

// does 'item' have a parent with serial 'serial'?
bool is_a_parent( const Item *item, u32 serial )
{
    while (item->container != NULL)
    {
        // UNTESTED
        item = item->container;
        if (item->serial == serial)
            return true;
    }
    return false;
}



// search for a container that this character can legally act upon
// - remove items, insert items, etc.
UContainer *find_legal_container( const Character *chr, u32 serial )
{
    UContainer *cont;
    cont = chr->backpack();
    if (cont)
    {
        if (serial == cont->serial)
            return cont;
        // not the main backpack, look for subpacks.
        cont = cont->find_container( serial );
        if (cont) 
            return cont;
	}

	// 4/2007 - MuadDib
	// Wasn't in backpack, check wornitems
    cont = NULL;
	Item *worn_item = chr->find_wornitem( serial );
	if (worn_item != NULL && worn_item->script_isa(POLCLASS_CONTAINER) )
	{
		// Ignore these layers explicitly. Backpack especially since it was
		// already checked above.
		if (worn_item->layer != LAYER_HAIR && worn_item->layer != LAYER_FACE && worn_item->layer != LAYER_BEARD
			&& worn_item->layer != LAYER_BACKPACK && worn_item->layer != LAYER_MOUNT)
		{
			UContainer* worn_cont = static_cast<UContainer*>(worn_item);
			if (worn_cont != NULL) 
				return worn_cont;
		}
	}


    // not in the backpack, or in a subpack.  check global items and subpacks.
    // FIXME doesn't check range?
    unsigned short wxL, wyL, wxH, wyH;
    zone_convert_clip( chr->x - 8, chr->y - 8, chr->realm, wxL, wyL );
    zone_convert_clip( chr->x + 8, chr->y + 8, chr->realm, wxH, wyH );
    for( unsigned short wx = wxL; wx <= wxH; ++wx )
    {
        for( unsigned short wy = wyL; wy <= wyH; ++wy )
        {
            ZoneItems& witem = chr->realm->zone[wx][wy].items;
            for( ZoneItems::iterator itr = witem.begin(), end = witem.end(); itr != end; ++itr )
            {
                Item* item = *itr;

                if (item->isa(UObject::CLASS_CONTAINER))
                {
                    cont = (UContainer *) item;
                    if (serial == cont->serial)
                        return cont;
                    cont = cont->find_container( serial );
                    if (cont)
                        return cont;
                }
            }
        }
    }

    Item* item = chr->search_remote_containers( serial, NULL /* don't care if it's a remote container */ );
    if (item && item->isa( UObject::CLASS_CONTAINER))
        return static_cast<UContainer*>(item);
    else
        return NULL;
}

// assume if you pass additlegal or isRemoteContainer, you init to false
Item *find_legal_item( const Character *chr, u32 serial, bool* additlegal, bool* isRemoteContainer )
{
    UContainer *backpack = chr->backpack();
    if (backpack != NULL && backpack->serial == serial)
        return backpack;

    // check worn items
	// 04/2007 - MuadDib Added:
	// find_wornitem will now check inside containers listed in layers
	// for normal items now also. This will allow for quivers
	// in wornitems, handbags, pockets, whatever people want,
	// to find stuff as a legal item to the character. Treats it
	// just like the backpack, without making it specific like
	// a bankbox or backpack.
	Item *item = chr->find_wornitem( serial );
    if (item != NULL) 
        return item;

    if (backpack != NULL) 
    {
        item = backpack->find( serial );
        if (item != NULL) 
            return item;
    }
    
	// check items on the ground
    unsigned short wxL, wyL, wxH, wyH;
    zone_convert_clip( chr->x - 8, chr->y - 8, chr->realm, wxL, wyL );
    zone_convert_clip( chr->x + 8, chr->y + 8, chr->realm, wxH, wyH );
    for( unsigned short wx = wxL; wx <= wxH; ++wx )
    {
        for( unsigned short wy = wyL; wy <= wyH; ++wy )
        {
            ZoneItems& witem = chr->realm->zone[wx][wy].items;
            for( ZoneItems::iterator itr = witem.begin(), end = witem.end(); itr != end; ++itr )
            {
                Item* item = *itr;
                if (!inrange( chr, item ))
                    continue;
                if (item->serial == serial)
                {
                    passert_always( item->container == NULL );
                    return item;
                }
                if (item->isa(UObject::CLASS_CONTAINER))
                {
                    item = ((UContainer *) item)->find( serial );
                    if (item != NULL) 
                        return item;
                }
            }
        }
    }

    if (additlegal != NULL)
        *additlegal = true;
    return chr->search_remote_containers( serial, isRemoteContainer );
}

void play_sound_effect( const UObject *center, u16 effect )
{
	PktOut_54* msg = REQUESTPACKET(PktOut_54,PKTOUT_54_ID);
	msg->Write(static_cast<u8>(PKTOUT_54_FLAG_SINGLEPLAY));
	msg->WriteFlipped(static_cast<u16>(effect-1)); // SOUND_EFFECT_XX, see sfx.h
	msg->offset+=2; //volume
	//msg->WriteFlipped(static_cast<u16>(0));
	msg->WriteFlipped(center->x);
	msg->WriteFlipped(center->y);
	msg->offset+=2;
	//msg->WriteFlipped(z);
    // FIXME hearing range check perhaps?
    transmit_to_inrange( center, &msg->buffer, msg->offset, false, false );
}

void play_sound_effect_xyz( u16 cx,u16 cy,s8 cz, u16 effect, Realm* realm )
{
	PktOut_54* msg = REQUESTPACKET(PktOut_54,PKTOUT_54_ID);
	msg->Write(static_cast<u8>(PKTOUT_54_FLAG_SINGLEPLAY));
	msg->WriteFlipped(static_cast<u16>(effect-1)); // SOUND_EFFECT_XX, see sfx.h
	msg->offset+=2; //volume
	//msg->WriteFlipped(static_cast<u16>(0));
	msg->WriteFlipped(cx);
	msg->WriteFlipped(cy);
	msg->WriteFlipped(static_cast<s16>(cz));

    for( Clients::iterator itr = clients.begin(), end = clients.end(); itr != end; ++itr )
    {
        Client* client = *itr;

        if (!client->ready)
            continue;
		if( client->chr->realm != realm )
			continue;
        if (inrange( client->chr, cx, cy ))
        {
            transmit( client, &msg->buffer, msg->offset );
        }
    }
}

void play_sound_effect_private( const UObject *center, u16 effect, Character* forchr )
{
    if (forchr->client)
    {
		PktOut_54* msg = REQUESTPACKET(PktOut_54,PKTOUT_54_ID);
		msg->Write(static_cast<u8>(PKTOUT_54_FLAG_SINGLEPLAY));
		msg->WriteFlipped(static_cast<u16>(effect-1)); // SOUND_EFFECT_XX, see sfx.h
		msg->offset+=2; //volume
		//msg->WriteFlipped(static_cast<u16>(0));
		msg->WriteFlipped(center->x);
		msg->WriteFlipped(center->y);
		msg->offset+=2;
		//msg->WriteFlipped(z);

        forchr->client->transmit( &msg->buffer, msg->offset );
    }
}

void play_moving_effect( const UObject *src, const UObject *dst,
                         u16 effect,
                         u8 speed,
                         u8 loop,
                         u8 explode)
{
	PktOut_70* msg = REQUESTPACKET(PktOut_70,PKTOUT_70_ID);
	msg->Write(static_cast<u8>(EFFECT_TYPE_MOVING));
	msg->Write(src->serial_ext);
	msg->Write(dst->serial_ext);
	msg->WriteFlipped(effect);
	msg->WriteFlipped(src->x);
	msg->WriteFlipped(src->y);
	msg->Write(static_cast<s8>(src->z+src->height));
	msg->WriteFlipped(dst->x);
	msg->WriteFlipped(dst->y);
	msg->Write(static_cast<s8>(dst->z+dst->height));
	msg->Write(speed);
	msg->Write(loop);
	msg->offset+=3; //unk24,unk25,unk26
	msg->Write(explode);

	for( Clients::iterator itr = clients.begin(), end = clients.end(); itr != end; ++itr )
    {
        Client* client = *itr;

        if (!client->ready)
            continue;
        //  -- in range of either source or destination.
        if (inrange( client->chr, src ) ||
            inrange( client->chr, dst ))
        {
            transmit( client, &msg->buffer, msg->offset );
        }
    }
	READDPACKET(msg);
}

void play_moving_effect2( u16 xs, u16 ys, s8 zs,
                          u16 xd, u16 yd, s8 zd,
                          u16 effect,
                          u8 speed,
                          u8 loop,
                          u8 explode,
						  Realm* realm )
{
	PktOut_70* msg = REQUESTPACKET(PktOut_70,PKTOUT_70_ID);
	msg->Write(static_cast<u8>(EFFECT_TYPE_MOVING));
	msg->offset+=8; // src+dst serial
	msg->WriteFlipped(effect);
	msg->WriteFlipped(xs);
	msg->WriteFlipped(ys);
	msg->Write(zs);
	msg->WriteFlipped(xd);
	msg->WriteFlipped(yd);
	msg->Write(zd);
	msg->Write(speed);
	msg->Write(loop);
	msg->offset+=3; //unk24,unk25,unk26
	msg->Write(explode);

    for( Clients::iterator itr = clients.begin(), end = clients.end(); itr != end; ++itr )
    {
        Client* client = *itr;

        if (!client->ready)
            continue;
        //  -- in range of either source or destination.
		if( client->chr->realm != realm )
			continue;
        if (inrange( client->chr, xs, ys ) ||
            inrange( client->chr, xd, yd ))
        {
            transmit( client, &msg->buffer, msg->offset );
        }
    }
	READDPACKET(msg);
}


void play_lightning_bolt_effect( const UObject* center )
{
	PktOut_70* msg = REQUESTPACKET(PktOut_70,PKTOUT_70_ID);
	msg->Write(static_cast<u8>(EFFECT_TYPE_LIGHTNING_BOLT));
	msg->Write(center->serial_ext);
	msg->offset+=6; // dst serial + effect
	msg->WriteFlipped(center->x);
	msg->WriteFlipped(center->y);
	msg->Write(center->z);
	msg->offset+=11;
    transmit_to_inrange( center, &msg->buffer, msg->offset, false, false );
	READDPACKET(msg);
}
 
void play_object_centered_effect( const UObject* center,
                                  u16 effect,
                                  u8 speed,
                                  u8 loop )
{
	PktOut_70* msg = REQUESTPACKET(PktOut_70,PKTOUT_70_ID);
	msg->Write(static_cast<u8>(EFFECT_TYPE_FOLLOW_OBJ));
	msg->Write(center->serial_ext);
	msg->offset+=4; // dst serial
	msg->WriteFlipped(effect);
	msg->WriteFlipped(center->x);
	msg->WriteFlipped(center->y);
	msg->Write(center->z);
	msg->offset+=5; //dst x,y,z
	msg->Write(speed);
	msg->Write(loop);
	msg->offset+=5; //unk24,unk25,unk26,explode
    transmit_to_inrange( center, &msg->buffer, msg->offset, false, false );
	READDPACKET(msg);
}

void play_stationary_effect( u16 x, u16 y, u8 z, u16 effect, u8 speed, u8 loop, u8 explode, Realm* realm )
{
	PktOut_70* msg = REQUESTPACKET(PktOut_70,PKTOUT_70_ID);
	msg->Write(static_cast<u8>(EFFECT_TYPE_STATIONARY_XYZ));
	msg->offset+=8; // src,dst serial
	msg->WriteFlipped(effect);
	msg->WriteFlipped(x);
	msg->WriteFlipped(y);
	msg->Write(z);
	msg->offset+=5; //dst x,y,z
	msg->Write(speed);
	msg->Write(loop);
	msg->offset+=2; //unk24,unk25
	msg->Write(static_cast<u8>(1)); // this is right for teleport, anyway
	msg->Write(explode);

    for( Clients::iterator itr = clients.begin(), end = clients.end(); itr != end; ++itr )
    {
        Client* client = *itr;

        if (!client->ready)
            continue;
		if( client->chr->realm != realm )
			continue;
        if (inrange( client->chr->x, client->chr->y, x, y ))
            client->transmit( &msg->buffer, msg->offset );
    }
	READDPACKET(msg);
}

void play_stationary_effect_ex( u16 x, u16 y, u8 z, Realm* realm, u16 effect, u8 speed, u8 duration, u32 hue, 
							    u32 render, u16 effect3d )
{
	PktOut_C7* msg = REQUESTPACKET(PktOut_C7,PKTOUT_C7_ID);
	partical_effect(msg,PKTOUT_C0::EFFECT_FIXEDXYZ,
		            0,0,x,y,z,x,y,z,
					effect,speed,duration,1,0,
					hue,render, effect3d, 1, 0,
					0,0xFF);

    for( Clients::iterator itr = clients.begin(), end = clients.end(); itr != end; ++itr )
    {
        Client* client = *itr;

        if (!client->ready)
            continue;
		if( client->chr->realm != realm )
			continue;
        if (inrange( client->chr->x, client->chr->y, x, y ))
            client->transmit( &msg->buffer, msg->offset );
    }
	READDPACKET(msg);
}

void play_object_centered_effect_ex( const UObject* center, u16 effect, u8 speed, u8 duration, u32 hue, 
							         u32 render, u8 layer, u16 effect3d )
{
	PktOut_C7* msg = REQUESTPACKET(PktOut_C7,PKTOUT_C7_ID);
	partical_effect(msg,PKTOUT_C0::EFFECT_FIXEDFROM,
		            center->serial_ext,center->serial_ext,
					center->x,center->y,center->z,
					center->x,center->y,center->z,
					effect,speed,duration,1,0,
					hue,render, effect3d, 1, 0,
					center->serial_ext,layer);

    transmit_to_inrange( center, &msg->buffer, msg->offset, false, false );
	READDPACKET(msg);
}

void play_moving_effect_ex( const UObject *src, const UObject *dst,
                            u16 effect, u8 speed, u8 duration, u32 hue, 
							u32 render, u8 direction, u8 explode, 
							u16 effect3d, u16 effect3dexplode, u16 effect3dsound)
{
	PktOut_C7* msg = REQUESTPACKET(PktOut_C7,PKTOUT_C7_ID);
	partical_effect(msg,PKTOUT_C0::EFFECT_MOVING,
		            src->serial_ext,dst->serial_ext,
		            src->x,src->y,src->z+src->height,
					dst->x,dst->y,dst->z+dst->height,
		            effect,speed,duration, direction,explode,hue,render,
					effect3d,effect3dexplode,effect3dsound,
					0,0xFF);

	for( Clients::iterator itr = clients.begin(), end = clients.end(); itr != end; ++itr )
    {
        Client* client = *itr;

        if (!client->ready)
            continue;
        //  -- in range of either source or destination.
        if (inrange( client->chr, src ) ||
            inrange( client->chr, dst ))
        {
            transmit( client, &msg->buffer, msg->offset );
        }
    }
	READDPACKET(msg);
}

void play_moving_effect2_ex( u16 xs, u16 ys, s8 zs,
                             u16 xd, u16 yd, s8 zd, Realm* realm,
                             u16 effect, u8 speed, u8 duration, u32 hue, 
							 u32 render, u8 direction, u8 explode, 
							 u16 effect3d, u16 effect3dexplode, u16 effect3dsound)
{
	PktOut_C7* msg = REQUESTPACKET(PktOut_C7,PKTOUT_C7_ID);
	partical_effect(msg,PKTOUT_C0::EFFECT_MOVING,
		            0,0,xs,ys,zs,xd,yd,zd,
		            effect,speed,duration, direction,explode,hue,render,
					effect3d,effect3dexplode,effect3dsound,
					0,0xFF);

	for( Clients::iterator itr = clients.begin(), end = clients.end(); itr != end; ++itr )
    {
        Client* client = *itr;

        if (!client->ready)
            continue;
        //  -- in range of either source or destination.
		if( client->chr->realm != realm )
			continue;
        if (inrange( client->chr, xs, ys ) ||
            inrange( client->chr, xd, yd ))
        {
            transmit( client, &msg->buffer, msg->offset );
        }
    }
	READDPACKET(msg);
}

// Central function to build 0xC7 packet
void partical_effect(PktOut_C7* msg,u8 type, u32 srcserial, u32 dstserial,
					 u16 srcx, u16 srcy, u8 srcz,
					 u16 dstx, u16 dsty, u8 dstz,
					 u16 effect, u8 speed, u8 duration, u8 direction,
					 u8 explode, u32 hue, u32 render, 
					 u16 effect3d, u16 effect3dexplode, u16 effect3dsound,
					 u32 itemid, u8 layer)
{
	//C0 part
	msg->Write(type);
	msg->Write(srcserial);
	msg->Write(dstserial);
	msg->WriteFlipped(effect);
	msg->WriteFlipped(srcx);
	msg->WriteFlipped(srcy);
	msg->Write(srcz);
	msg->WriteFlipped(dstx);
	msg->WriteFlipped(dsty);
	msg->Write(dstz);
	msg->Write(speed);
	msg->Write(duration);
	msg->offset+=2; // u16 unk
	msg->Write(direction);
	msg->Write(explode);
	msg->WriteFlipped(hue);
	msg->WriteFlipped(render);
	// C7 part
	msg->WriteFlipped(effect3d);   //see particleffect subdir
	msg->WriteFlipped(effect3dexplode); //0 if no explosion
	msg->WriteFlipped(effect3dsound); //for moving effects, 0 otherwise
	msg->Write(itemid); //if target is item (type 2), 0 otherwise 
	msg->Write(layer); //(of the character, e.g left hand, right hand, … 0-5,7, 0xff: moving effect or target is no char) 
	msg->offset+=2; // u16 unk_effect
}

// System message -- message in lower left corner
void send_sysmessage(Client *client, const char *text, unsigned short font, unsigned short color )
{
	PktOut_1C* msg = REQUESTPACKET(PktOut_1C,PKTOUT_1C_ID);
	u16 textlen= static_cast<u16>(strlen(text) + 1);
	if (textlen > SPEECH_MAX_LEN+1)  // FIXME need to handle this better second msg?
		textlen = SPEECH_MAX_LEN+1;

	msg->offset+=2;
	msg->Write(static_cast<u32>(0x01010101));
	msg->Write(static_cast<u16>(0x0101));
	msg->Write(static_cast<u8>(TEXTTYPE_NORMAL));
	msg->WriteFlipped(color);
	msg->WriteFlipped(font);
	msg->Write("System",30);
	msg->Write(text,textlen);
	u16 len=msg->offset;
	msg->offset=1;
	msg->WriteFlipped(len);
	transmit( client, &msg->buffer, len );
	READDPACKET(msg);
}

// Unicode System message -- message in lower left corner
void send_sysmessage(Client *client,
					 const u16 *wtext, const char lang[4],
					 unsigned short font, unsigned short color )
{
	unsigned textlen = 0;
	//textlen = wcslen((const wchar_t*)wtext) + 1;
	while( wtext[textlen] != L'\0' )
		++textlen;
	if (textlen > (SPEECH_MAX_LEN))  // FIXME need to handle this better second msg?
		textlen = (SPEECH_MAX_LEN);

	PktOut_AE* msg = REQUESTPACKET(PktOut_AE,PKTOUT_AE_ID);
	msg->offset+=2;
	msg->Write(static_cast<u32>(0x01010101));
	msg->Write(static_cast<u16>(0x0101));
	msg->Write(static_cast<u8>(TEXTTYPE_NORMAL));
	msg->WriteFlipped(color);
	msg->WriteFlipped(font);
	msg->Write(lang,4);
	msg->Write("System",30);
	msg->WriteFlipped(&wtext[0],static_cast<u16>(textlen));
	u16 len=msg->offset;
	msg->offset=1;
	msg->WriteFlipped(len);
	transmit( client, &msg->buffer, len );
	READDPACKET(msg);
}

void send_sysmessage(Client *client, const std::string& text, unsigned short font, unsigned short color)
{
    send_sysmessage( client, text.c_str(), font, color );
}

void send_sysmessage(Client *client, const std::wstring& wtext, const char lang[4], unsigned short font, unsigned short color)
{
	using std::wstring;
	u16 uctext[SPEECH_MAX_LEN+1];
	for(size_t i = 0; i < wtext.length(); i++)
		uctext[i] = static_cast<u16>(wtext[i]);
	uctext[wtext.length()] = 0x00;
    send_sysmessage( client, uctext, lang, font, color );
}

void broadcast( const char *text, unsigned short font, unsigned short color )
{
    for( Clients::iterator itr = clients.begin(), end = clients.end(); itr != end; ++itr )
    {
        Client* client = *itr;
        if (!client->ready)
            continue;

        send_sysmessage( client, text, font, color );
    }
}

void broadcast( const u16 *wtext, const char lang[4],
			    unsigned short font, unsigned short color )
{
    for( Clients::iterator itr = clients.begin(), end = clients.end(); itr != end; ++itr )
    {
        Client* client = *itr;
        if (!client->ready)
            continue;

        send_sysmessage( client, wtext, lang, font, color );
    }
}

void send_nametext( Client *client, const Character *chr, const std::string& str )
{
	PktOut_1C* msg = REQUESTPACKET(PktOut_1C,PKTOUT_1C_ID);
	u16 textlen= static_cast<u16>(str.length() + 1);
	if (textlen > SPEECH_MAX_LEN+1)
		textlen = SPEECH_MAX_LEN+1;

	msg->offset+=2;
	msg->Write(chr->serial_ext);
	msg->Write(static_cast<u16>(0x0101));
	msg->Write(static_cast<u8>(TEXTTYPE_YOU_SEE));
	msg->WriteFlipped(chr->name_color( client->chr )); // 0x03B2
	msg->WriteFlipped(static_cast<u16>(3));
	msg->Write(str.c_str(),30);
	msg->Write(str.c_str(),textlen);
	u16 len=msg->offset;
	msg->offset=1;
	msg->WriteFlipped(len);
	transmit( client, &msg->buffer, len );
	READDPACKET(msg);
}

bool say_above(const UObject* obj, 
               const char *text,
               unsigned short font,
               unsigned short color,
			   unsigned int journal_print)
{
	PktOut_1C* msg = REQUESTPACKET(PktOut_1C,PKTOUT_1C_ID);
	u16 textlen= static_cast<u16>(strlen(text) + 1);
	if (textlen > SPEECH_MAX_LEN+1)  // FIXME need to handle this better second msg?
		textlen = SPEECH_MAX_LEN+1;

	msg->offset+=2;
	msg->Write(obj->serial_ext);
	msg->Write(obj->graphic_ext);
	msg->Write(static_cast<u8>(TEXTTYPE_NORMAL));
	msg->WriteFlipped(color);
	msg->WriteFlipped(font);
	switch(journal_print)
	{
		case JOURNAL_PRINT_YOU_SEE:
			msg->Write("You see", 30 );
			break;
		case JOURNAL_PRINT_NAME:
		default:
			msg->Write(obj->description().c_str(), 30 );
			break;
	}
	msg->Write(text,textlen);
	u16 len=msg->offset;
	msg->offset=1;
	msg->WriteFlipped(len);
	// todo: only send to those that I'm visible to.
	transmit_to_inrange( obj, &msg->buffer, len, false, false );
	READDPACKET(msg);
	return true;
}

bool say_above(const UObject* obj, 
               const u16 *wtext, 
			   const char lang[4],
               unsigned short font,
               unsigned short color,
			   unsigned int journal_print)
{
	unsigned textlen = 0;
	//textlen = wcslen((const wchar_t*)wtext) + 1;
	while( wtext[textlen] != L'\0' )
		++textlen;
	if (textlen > (SPEECH_MAX_LEN))  // FIXME need to handle this better second msg?
		textlen = (SPEECH_MAX_LEN);

	PktOut_AE* msg = REQUESTPACKET(PktOut_AE,PKTOUT_AE_ID);
	msg->offset+=2;
	msg->Write(obj->serial_ext);
	msg->Write(obj->graphic_ext);
	msg->Write(static_cast<u8>(TEXTTYPE_NORMAL));
	msg->WriteFlipped(color);
	msg->WriteFlipped(font);
	msg->Write(lang,4);
	switch(journal_print)
	{
		case JOURNAL_PRINT_YOU_SEE:
			msg->Write("You see", 30);
			break;
		case JOURNAL_PRINT_NAME:
		default:
			msg->Write(obj->description().c_str(), 30);
			break;
	}
	msg->WriteFlipped(&wtext[0],static_cast<u16>(textlen));
	u16 len=msg->offset;
	msg->offset=1;
	msg->WriteFlipped(len);
	// todo: only send to those that I'm visible to.
	transmit_to_inrange( obj, &msg->buffer, len, false, false );
	READDPACKET(msg);
    return true;
}

bool private_say_above( Character* chr, 
                        const UObject* obj, 
                        const char *text,
                        unsigned short font,
                        unsigned short color,
						unsigned int journal_print)
{
	if (chr->client == NULL)
		return false;
	PktOut_1C* msg = REQUESTPACKET(PktOut_1C,PKTOUT_1C_ID);
	u16 textlen= static_cast<u16>(strlen(text) + 1);
	if (textlen > SPEECH_MAX_LEN+1)  // FIXME need to handle this better second msg?
		textlen = SPEECH_MAX_LEN+1;

	msg->offset+=2;
	msg->Write(obj->serial_ext);
	msg->Write(obj->graphic_ext);
	msg->Write(static_cast<u8>(TEXTTYPE_NORMAL));
	msg->WriteFlipped(color);
	msg->WriteFlipped(font);
	switch(journal_print)
	{
		case JOURNAL_PRINT_YOU_SEE:
			msg->Write("You see", 30 );
			break;
		case JOURNAL_PRINT_NAME:
		default:
			msg->Write(obj->description().c_str(), 30 );
			break;
	}
	msg->Write(text,textlen);
	u16 len=msg->offset;
	msg->offset=1;
	msg->WriteFlipped(len);
	chr->client->transmit( &msg->buffer, len );
	READDPACKET(msg);
	return true;
}

bool private_say_above( Character* chr, 
                        const UObject* obj, 
                        const u16 *wtext, 
						const char lang[4],
                        unsigned short font,
                        unsigned short color,
						unsigned int journal_print )
{
	unsigned textlen = 0;
	//textlen = wcslen((const wchar_t*)wtext) + 1;
	while( wtext[textlen] != L'\0' )
		++textlen;
	if (textlen > (SPEECH_MAX_LEN))  // FIXME need to handle this better second msg?
		textlen = (SPEECH_MAX_LEN);
	if (chr->client == NULL)
		return false;

	PktOut_AE* msg = REQUESTPACKET(PktOut_AE,PKTOUT_AE_ID);
	msg->offset+=2;
	msg->Write(obj->serial_ext);
	msg->Write(obj->graphic_ext);
	msg->Write(static_cast<u8>(TEXTTYPE_NORMAL));
	msg->WriteFlipped(color);
	msg->WriteFlipped(font);
	msg->Write(lang,4);
	switch(journal_print)
	{
	case JOURNAL_PRINT_YOU_SEE:
		msg->Write("You see", 30);
		break;
	case JOURNAL_PRINT_NAME:
	default:
		msg->Write(obj->description().c_str(), 30);
		break;
	}
	msg->WriteFlipped(&wtext[0],static_cast<u16>(textlen));
	u16 len=msg->offset;
	msg->offset=1;
	msg->WriteFlipped(len);
	chr->client->transmit( &msg->buffer, len );
	READDPACKET(msg);
	return true;
}

bool private_say_above_ex( Character* chr, 
                           const UObject* obj, 
                           const char *text,
                           unsigned short color )
{
	if (chr->client == NULL)
		return false;
	PktOut_1C* msg = REQUESTPACKET(PktOut_1C,PKTOUT_1C_ID);
	u16 textlen= static_cast<u16>(strlen(text) + 1);
	if (textlen > SPEECH_MAX_LEN+1)  // FIXME need to handle this better second msg?
		textlen = SPEECH_MAX_LEN+1;

	msg->offset+=2;
	msg->Write(obj->serial_ext);
	msg->Write(obj->graphic_ext);
	msg->Write(static_cast<u8>(TEXTTYPE_NORMAL));
	msg->WriteFlipped(color);
	msg->WriteFlipped(static_cast<u8>(3));
	msg->Write(obj->description().c_str(), 30 );
	msg->Write(text,textlen);
	u16 len=msg->offset;
	msg->offset=1;
	msg->WriteFlipped(len);
	chr->client->transmit( &msg->buffer, len );
	READDPACKET(msg);
	return true;
}

void send_objdesc( Client *client, Item *item )
{
	PktOut_1C* msg = REQUESTPACKET(PktOut_1C,PKTOUT_1C_ID);
	u16 textlen= static_cast<u16>(item->description().length() + 1);
	if (textlen > SPEECH_MAX_LEN+1)  // FIXME need to handle this better second msg?
		textlen = SPEECH_MAX_LEN+1;
	msg->offset+=2;
	msg->Write(item->serial_ext);
	msg->Write(item->graphic_ext);
	msg->Write(static_cast<u8>(TEXTTYPE_YOU_SEE));
	msg->WriteFlipped(static_cast<u16>(0x03B2));
	msg->WriteFlipped(static_cast<u16>(3));
	msg->Write("System", 30 );
	msg->Write(item->description().c_str(),textlen);
	u16 len=msg->offset;
	msg->offset=1;
	msg->WriteFlipped(len);
	transmit( client, &msg->buffer, len );
	READDPACKET(msg);
}

void send_stamina_level( Client *client )
{
    Character *chr = client->chr;

	PktOut_A3* msg = REQUESTPACKET(PktOut_A3,PKTOUT_A3_ID);
	msg->Write(chr->serial_ext);

	if (uoclient_general.stamina.any)
	{
		int v = chr->vital(uoclient_general.mana.id).maximum_ones();
		if (v > 0xFFFF)
			v = 0xFFFF;
		msg->WriteFlipped(static_cast<u16>(v));

		v = chr->vital(uoclient_general.mana.id).current_ones();
		if (v > 0xFFFF)
			v = 0xFFFF;
		msg->WriteFlipped(static_cast<u16>(v));
	}
	else
	{
		msg->offset+=4;
	}
	transmit( client, &msg->buffer, msg->offset );
	READDPACKET(msg);
}

void send_mana_level( Client *client )
{
    Character *chr = client->chr;

	PktOut_A2* msg = REQUESTPACKET(PktOut_A2,PKTOUT_A2_ID);
	msg->Write(chr->serial_ext);

    if (uoclient_general.mana.any)
    {
		int v = chr->vital(uoclient_general.mana.id).maximum_ones();
		if (v > 0xFFFF)
			v = 0xFFFF;
		msg->WriteFlipped(static_cast<u16>(v));

		v = chr->vital(uoclient_general.mana.id).current_ones();
		if (v > 0xFFFF)
			v = 0xFFFF;
		msg->WriteFlipped(static_cast<u16>(v));
    }
    else
    {
		msg->offset+=4;
    }
    transmit( client, &msg->buffer, msg->offset );
	READDPACKET(msg);
}

void send_death_message( Character *chr_died, Item *corpse )
{
	PktOut_AF* msg = REQUESTPACKET(PktOut_AF,PKTOUT_AF_ID);
	msg->Write(chr_died->serial_ext);
	msg->Write(corpse->serial_ext);
	msg->offset+=4; // u32 unk4_zero

    for( Clients::iterator itr = clients.begin(), end = clients.end(); itr != end; ++itr )
    {
        Client *client = *itr;
        if (!client->ready)
            continue;
        
        if (client->chr == chr_died)
            continue;
        
        if (inrange( client->chr, corpse ))
            transmit( client, &msg->buffer, msg->offset );
    }
	READDPACKET(msg);
}

void transmit_to_inrange( const UObject* center, const void* msg, unsigned msglen, bool is_6017, bool is_UOKR)
{
    for( Clients::iterator itr = clients.begin(), end = clients.end(); itr != end; ++itr )
    {
        Client* client = *itr;

        if (!client->ready)
		{
            continue;
		}
		if (is_6017 && (!( client->ClientType & CLIENTTYPE_6017 )))
		{
			continue;
		}
		if (is_UOKR && (!( client->ClientType & CLIENTTYPE_UOKR )))
		{
			continue;
		}
		if (client->chr->realm != center->realm)
		{
			continue;
		}
		if (inrange( client->chr, center ))
		{
            client->transmit( msg, msglen );
		}
	}
}

void transmit_to_others_inrange( Character* center, const void* msg, unsigned msglen, bool is_6017, bool is_UOKR )
{
    for( Clients::iterator itr = clients.begin(), end = clients.end(); itr != end; ++itr )
    {
        Client* client = *itr;

        if (!client->ready)
		{
            continue;
		}
		if (is_6017 && (!( client->ClientType & CLIENTTYPE_6017 )))
		{
			continue;
		}
		if (is_UOKR && (!( client->ClientType & CLIENTTYPE_UOKR )))
		{
			continue;
		}
		if (client->chr == center)
		{
            continue;
		}
		if (client->chr->realm != center->realm)
		{
			continue;
		}
		if (inrange( client->chr, center ))
		{
			client->transmit( msg, msglen );
		}
	}
}

//DAVE made heavy changes to this 11/17 for speed.
Character* chr_from_wornitems( UContainer* wornitems )
{
	Character* owner = wornitems->get_chr_owner();
	if(owner != NULL)
		return owner;
	else
		return NULL; //fixed 3/8/3
}

void destroy_item( Item* item )
{
    if (item->serial == 0)
    {
        Log( "destroy %s: %s, orphan! (old serial: 0x%lx)\n",
                item->description().c_str(),
                item->classname(),
                cfBEu32(item->serial_ext) );
    }

    if (item->serial != 0)
    {
/*
    cout << "destroy " << item->description() << ": "
         << item->classname() << " " <<  item 
         << ", serial=" << hexint(item->serial) << endl;
*/
        item->set_dirty();

        ConstForEach( clients, send_remove_object_if_inrange, item );
        if (item->container == NULL) // on ground, easy.
        {
            if (!item->is_gotten()) // and not in hand
                remove_item_from_world( item );
        }
        else
        {
            item->extricate();
        }
        item->destroy();
    }
}

void setrealm(Item* item, void* arg)
{
	Realm* realm = static_cast<Realm*>(arg);
	item->realm = realm;
}

void world_delete( UObject* uobj )
{
    // std::cout << "world_delete(0x" << hex << uobj->serial << dec << ")" << endl;
    uobj->destroy();
/* moved to UObject::destroy()    
    uobj->serial = 0;
    uobj->serial_ext = 0;

    passert( uobj->count() >= 1 );

    ref_ptr<UObject> ref( uobj );

    uobj->release();
*/
}

void subtract_amount_from_item( Item* item, unsigned short amount )
{
    if (amount >= item->getamount())
    {
        destroy_item( item );
		return; //destroy_item will update character weight if item is carried.
    }
    else
    {
        item->subamount( amount );
        update_item_to_inrange( item );
    }
	//DAVE added this 11/17: if in a Character's pack, update weight.
	UpdateCharacterWeight(item);
}

#include "mdelta.h"

void move_item( Item* item, UFACING facing )
{
    u16 oldx = item->x;
    u16 oldy = item->y;

    item->x += move_delta[facing].xmove;
    item->y += move_delta[facing].ymove;

    item->restart_decay_timer();
    MoveItemWorldPosition( oldx, oldy, item, NULL );

	PktOut_1D* msgremove = REQUESTPACKET(PktOut_1D,PKTOUT_1D_ID);
	msgremove->Write(item->serial_ext);

    for( Clients::iterator itr = clients.begin(), end = clients.end(); itr != end; ++itr )
    {
        Client *client = *itr;
        if (!client->ready)
            continue;

        if (inrange( client->chr, item ))
        {
            send_item( client, item );
        }
        else // not in range.  If old loc was in range, send a delete.
        {
            if (inrange( client->chr->x, client->chr->y,
                         oldx, oldy ))
            {
				transmit( client, &msgremove->buffer, msgremove->offset );
            }
        }
    }
	READDPACKET(msgremove);

}

// FIXME: this is called from some places where the item didn't used
// to be on the ground - in a container, say.
// FIXME OPTIMIZE: Core is building the packet in send_item for every single client
// that needs to get it. There should be a better method for this. Such as, a function
// to run all the checks after building the packet here, then send as it needs to.
void move_item( Item* item, unsigned short newx, unsigned short newy, signed char newz, Realm* oldrealm )
{
    item->set_dirty();

	u16 oldx = item->x;
    u16 oldy = item->y;

    item->x = newx;
    item->y = newy;
    item->z = newz;

    item->restart_decay_timer();
    MoveItemWorldPosition( oldx, oldy, item, oldrealm );

	PktOut_1D* msgremove = REQUESTPACKET(PktOut_1D,PKTOUT_1D_ID);
	msgremove->Write(item->serial_ext);

    for( Clients::iterator itr = clients.begin(), end = clients.end(); itr != end; ++itr )
    {
        Client *client = *itr;
        if (!client->ready)
            continue;

        if (inrange( client->chr, item ))
        {
            send_item( client, item );
        }
        else // not in range.  If old loc was in range, send a delete.
        {
            if ( inrange( client->chr->x, client->chr->y, oldx, oldy )
				&& client->chr->realm == oldrealm )
            {
				transmit( client, &msgremove->buffer, msgremove->offset );
            }
        }
    }
	READDPACKET(msgremove);
}

void move_boat_item( Item* item, unsigned short newx, unsigned short newy, signed char newz, Realm* oldrealm )
{
    u16 oldx = item->x;
    u16 oldy = item->y;

    item->x = newx;
    item->y = newy;
    item->z = newz;

    MoveItemWorldPosition( oldx, oldy, item, oldrealm );

	PktOut_1A* msg = REQUESTPACKET(PktOut_1A,PKTOUT_1A_ID);
	msg->offset+=2;
	msg->Write(item->serial_ext);
	msg->Write(item->graphic_ext);
	msg->WriteFlipped(item->x);
	msg->WriteFlipped(item->y);
	msg->Write(item->z);
	u16 len1A=msg->offset;
	msg->offset=1;
	msg->WriteFlipped(len1A);

	// Client >= 7.0.0.0 ( SA )
	PktOut_F3* msg2 = REQUESTPACKET(PktOut_F3,PKTOUT_F3_ID);
	msg2->WriteFlipped(static_cast<u16>(0x1));
	msg2->offset++; // datatype
	msg2->Write(item->serial_ext);
	msg2->Write(item->graphic_ext);
	msg2->Write(item->facing);
	msg2->WriteFlipped(static_cast<u16>(0x1));
	msg2->WriteFlipped(static_cast<u16>(0x1));
	msg2->WriteFlipped(item->x);
	msg2->WriteFlipped(item->y);
	msg2->Write(item->z);
	msg2->offset++; //facing
	msg2->Write(item->color_ext);
	msg2->offset++; //flags

	PktOut_1D* msgremove = REQUESTPACKET(PktOut_1D,PKTOUT_1D_ID);
	msgremove->Write(item->serial_ext);

    for( Clients::iterator itr = clients.begin(), end = clients.end(); itr != end; ++itr )
    {
        Client *client = *itr;
        if (!client->ready)
            continue;

        if (inrange( client->chr, item ))
        {
			if (client->ClientType & CLIENTTYPE_7000)
				client->transmit( &msg2->buffer, msg2->offset );
			else
				client->transmit( &msg->buffer, len1A );
        }
        else // not in range.  If old loc was in range, send a delete.
        {
            if (inrange( client->chr->x, client->chr->y,
                         oldx, oldy ))
            {
				transmit( client, &msgremove->buffer, msgremove->offset );
            }
        }
    }
	READDPACKET(msg);
	READDPACKET(msg2);
	READDPACKET(msgremove);
}

#include "multi/multi.h"
void send_multi( Client* client, const UMulti* multi )
{
	if (client->ClientType & CLIENTTYPE_7000)
	{
		PktOut_F3* msg = REQUESTPACKET(PktOut_F3,PKTOUT_F3_ID);
		msg->WriteFlipped(static_cast<u16>(0x1));
		msg->Write(static_cast<u8>(0x02));
		msg->Write(multi->serial_ext);
		msg->WriteFlipped(multi->multidef().multiid);
		msg->offset++; //facing;
		msg->WriteFlipped(static_cast<u16>(0x1)); //amount
		msg->WriteFlipped(static_cast<u16>(0x1)); //amount2
		msg->WriteFlipped(multi->x);
		msg->WriteFlipped(multi->y);
		msg->Write(multi->z);
		msg->offset+=4; // u8 layer, u16 color, u8 flags
		client->transmit( &msg->buffer, msg->offset );
		READDPACKET(msg);
	}
	else
	{
		PktOut_1A* msg = REQUESTPACKET(PktOut_1A,PKTOUT_1A_ID);
		msg->offset+=2;
		msg->Write(multi->serial_ext);
		msg->Write(multi->graphic_ext);
		msg->WriteFlipped(multi->x);
		msg->WriteFlipped(multi->y);
		msg->Write(multi->z);
		u16 len=msg->offset;
		msg->offset=1;
		msg->WriteFlipped(len);
		client->transmit( &msg->buffer, len );
		READDPACKET(msg);
	}
}

void send_multi_to_inrange( const UMulti* multi )
{
    // FIXME could use transmit_to_inrange, almost.
    // (Character-specific flags, like can_move(), make it so we can't)
	// However, could build main part of packet before for/loop, then
	// adjust per client. Would this be a better solution?
    for( Clients::iterator itr = clients.begin(), end = clients.end(); itr != end; ++itr )
    {
        Client *client = *itr;
        if (!client->ready)
            continue;
        if (inrange( client->chr, multi ))
				send_multi( client, multi );
    }
}

#include "zone.h"
#include "miscrgn.h"
#include "network/cgdata.h"

void update_lightregion( Client* client, LightRegion* lightregion )
{
    if (!client->ready)
        return;

    client->chr->check_light_region_change();
}

void SetRegionLightLevel( LightRegion* lightregion, int lightlevel )
{
    lightregion->lightlevel = lightlevel;
    ForEach( clients, update_lightregion, lightregion );
}

void update_weatherregion( Client* client, WeatherRegion* weatherregion )
{
    if (!client->ready)
        return;

    if (client->gd->weather_region == weatherregion)
    {
       // client->gd->weather_region = NULL;  //dave commented this out 5/26/03, causing no processing to happen in following function, added force bool instead.
        client->chr->check_weather_region_change(true);
        client->chr->check_light_region_change();
    }
}

void SetRegionWeatherLevel( WeatherRegion* weatherregion,
                            unsigned type,
                            unsigned severity,
                            unsigned aux,
                            int lightoverride )
{
	weatherregion->weathertype = static_cast<unsigned char>(type);
    weatherregion->severity = static_cast<unsigned char>(severity);
    weatherregion->aux = static_cast<unsigned char>(aux);
    weatherregion->lightoverride = lightoverride;

    ForEach( clients, update_weatherregion, weatherregion );
}

void check_weather_change( Client* client )
{
    if (!client->ready)
        return;

    client->chr->check_weather_region_change();
    client->chr->check_light_region_change();
}

void update_all_weatherregions()
{
    ForEach( clients, check_weather_change );
}

/* there are four forms of 'name' in objinfo:
    name					 (normal)
	name%s					 (percent followed by plural-part, then null-term)
	name%s%					 (percent followed by plural-part, then percent, then more)
	wheat shea%ves/f%		 ( '%', plural part, '/', single part, '%', rest )
   Some examples:
		pil%es/e% of hides
		banana%s%
		feather%s
   Known bugs:
	1 gold coin displays as "gold coin".  There must be a bit somewhere 
	that I just don't understand yet.
*/
string format_description( unsigned int polflags, const string& descdef, unsigned short amount )
{
    string desc;

	if (amount != 1)
	{ 
        char s[15];
        sprintf( s, "%hu ", amount );
		desc = s;
	}
  else
    if (ssopt.use_tile_flag_prefix)
    {
      if (polflags & FLAG::DESC_PREPEND_AN)
      {
        desc = "an ";
      }
      else if (polflags & FLAG::DESC_PREPEND_A)
      {
        desc = "a ";
      }
    }
	
	// might want to consider strchr'ing here,  
	//   if not found, strcpy/return
	//   if found, memcpy up to strchr result, then continue as below.
	const char *src = descdef.c_str();
	int singular = (amount == 1);
	int plural_handled = 0;
	int phase = 0; /* 0= first part, 1=plural part, 2=singular part, 3=rest */
	int rest = 0;
	char ch;
	while ('\0' != (ch = *src))
	{
		if (phase == 0)
		{
			if (ch == '%')
			{
				plural_handled = 1;
				phase = 1;
			}
			else
			{
				desc += ch;
			}
		}
		else if (phase == 1)
		{
			if (ch == '%')
				phase = 3;
			else if (ch == '/')
				phase = 2;
			else if (!singular)
				desc += ch;
		}
		else if (phase == 2)
		{
			if (ch == '%')
				phase = 3;
			else if (singular)
				desc += ch;
		}
		else
		// if phase == 3 that means there are more words to come, 
		// lets loop through them to support singular/plural stuff in more than just the first word of the desc.
		{
			rest = 1;
			phase = 0;
			while (rest && '\0' != ch)
			{
				if (phase == 0)
				{
					if (ch == '%')
					{
						plural_handled = 1;
						phase = 1;
					}
					else
					{
						desc += ch;
						rest = 0;
					}
				}
				else if (phase == 1)
				{
					if (ch == '%')
						phase = 3;
					else if (ch == '/')
						phase = 2;
					else if (!singular)
						desc += ch;
						rest = 0;
				}
				else if (phase == 2)
				{
					if (ch == '%')
						phase = 3;
					else if (singular)
						desc += ch;
						rest = 0;
				}
				else // more words in the desc phase == 3, continue loop.
					rest = 1;
					phase = 0;
			}
		}
		++src;
	}
	
    if (!singular && !plural_handled)
		desc += 's';

    return desc;
}

void send_midi( Client* client, u16 midi )
{
	PktOut_6D* msg = REQUESTPACKET(PktOut_6D,PKTOUT_6D_ID);
	msg->WriteFlipped(midi);
	client->transmit( &msg->buffer, msg->offset );
	READDPACKET(msg);
    // cout << "Setting midi for " << client->chr->name() << " to " << midi << endl;
}

void register_with_supporting_multi( Item* item )
{
    if (item->container == NULL)
    {
        UMulti* multi = item->realm->find_supporting_multi( item->x, item->y, item->z );
        if (multi)
            multi->register_object( item );
    }
}


void send_create_mobile_if_nearby_cansee( Client* client, const Character* chr )
{
    if (client->ready &&                // must be logged into game
        inrange( client->chr, chr ) &&
        client->chr != chr &&
        client->chr->is_visible_to_me( chr ))
    {
        send_owncreate( client, chr );
    }
}

void send_move_mobile_if_nearby_cansee( Client* client, const Character* chr )
{
    if (client->ready &&                // must be logged into game
        inrange( client->chr, chr ) &&
        client->chr != chr &&
        client->chr->is_visible_to_me( chr ))
    {
        send_move( client, chr );
    }
}

void send_create_mobile_to_nearby_cansee( const Character* chr )
{
    ForEach( clients, send_create_mobile_if_nearby_cansee, chr );
}

void send_move_mobile_to_nearby_cansee( const Character* chr )
{
    ForEach( clients, send_move_mobile_if_nearby_cansee, chr );
}

Character* UpdateCharacterWeight(Item* item)
{
	Character* chr_owner = item->GetCharacterOwner();
	if(chr_owner != NULL && chr_owner->client != NULL)
	{
		send_full_statmsg( chr_owner->client, chr_owner );
		return chr_owner;
	}
	return NULL;
}

void UpdateCharacterOnDestroyItem(Item* item)
{
	Character* chr_owner = item->GetCharacterOwner();
	if(chr_owner != NULL)
	{
		if (item->layer && chr_owner->is_equipped(item))
		{
			PktOut_1D* msgremove = REQUESTPACKET(PktOut_1D,PKTOUT_1D_ID);
			msgremove->Write(item->serial_ext);
			transmit_to_inrange( item, &msgremove->buffer, msgremove->offset, false, false );
			READDPACKET(msgremove);
		}
	}
}

//Dave added this 12/1/02
bool clientHasCharacter(Client* c)
{
	return (c->chr != NULL);
}

void login_complete(Client* c)
{
	PktOut_55* msg = REQUESTPACKET(PktOut_55,PKTOUT_55_ID);
	c->transmit(&msg->buffer, msg->offset);
	READDPACKET(msg);
}

void send_feature_enable(Client* client)
{
	u32 clientflag = 0;
	string uo_expansion = client->acct->uo_expansion();
	if(uo_expansion.find("SA") != string::npos)
	{
		clientflag = 0x187DF;
		client->UOExpansionFlag = SA | KR | ML | SE | AOS; // SA needs KR- ML- SE- and AOS- features (and used checks) too
	}
	else if(uo_expansion.find("KR") != string::npos)
	{
		clientflag = 0x86DB;
		client->UOExpansionFlag = KR | ML | SE | AOS; // KR needs ML- SE- and AOS-features (and used checks) too
	}
	else if(uo_expansion.find("ML") != string::npos)
	{
		clientflag = 0x80DB;
		client->UOExpansionFlag = ML | SE | AOS; // ML needs SE- and AOS-features (and used checks) too
	}
	else if(uo_expansion.find("SE") != string::npos)
	{
		clientflag = 0x805B;
		client->UOExpansionFlag = SE | AOS; // SE needs AOS-features (and used checks) too
	}
	else if(uo_expansion.find("AOS") != string::npos)
	{
		clientflag = 0x801B;
		client->UOExpansionFlag = AOS;
	}
	else if(uo_expansion.find("LBR") != string::npos)
	{
		clientflag = 0x0002;
		client->UOExpansionFlag = LBR;
	}
	else if(uo_expansion.find("T2A") != string::npos)
	{
		clientflag = 0x0001;
		client->UOExpansionFlag = T2A;
	}
    
	// Change flag according to the number of CharacterSlots
	if (client->UOExpansionFlag & AOS) {
		if (config.character_slots == 7) {
			clientflag |= 0x1000; // 7th & 6th character flag (B9 Packet)
			clientflag &= ~0x0004; // Disable Third Dawn?
		}
		else if (config.character_slots == 6) {
			clientflag |= 0x0020; // 6th character flag (B9 Packet)
			clientflag &= ~0x0004;
		}
	}

    // Roleplay faces?
    if (client->UOExpansionFlag & KR)
    {
        if (ssopt.support_faces == 2)
            clientflag |= 0x2000;
    }

	PktOut_B9* msg = REQUESTPACKET(PktOut_B9,PKTOUT_B9_ID);
	if ( client->ClientType & CLIENTTYPE_60142 )
		msg->WriteFlipped(clientflag);
	else
		msg->WriteFlipped(static_cast<u16>(clientflag));
	client->transmit(&msg->buffer, msg->offset);
	READDPACKET(msg);
}

void send_realm_change( Client* client, Realm* realm )
{
	PktOut_BF_Sub8* msg = REQUESTSUBPACKET(PktOut_BF_Sub8,PKTBI_BF_ID,PKTBI_BF::TYPE_CURSOR_HUE);
	msg->WriteFlipped(static_cast<u16>(6));
	msg->offset+=2; //sub
	msg->Write(static_cast<u8>(realm->getUOMapID()));
	client->transmit(&msg->buffer, msg->offset);
	READDPACKET(msg);
}

void send_map_difs( Client* client )
{
	PktOut_BF_Sub18* msg = REQUESTSUBPACKET(PktOut_BF_Sub18,PKTBI_BF_ID,PKTBI_BF::TYPE_ENABLE_MAP_DIFFS);
	msg->offset+=4; //len+sub
	msg->WriteFlipped(baserealm_count);
	for(unsigned int i=0; i<baserealm_count; i++)
	{
		msg->WriteFlipped(Realms->at(i)->getNumStaticPatches());
		msg->WriteFlipped(Realms->at(i)->getNumMapPatches());
	}
	u16 len=msg->offset;
	msg->offset=1;
	msg->WriteFlipped(len);
	client->transmit( &msg->buffer, len);
	READDPACKET(msg);
}

// FIXME : Works, except for Login. Added length check as to not mess with 1.x clients
//         during login. Added send_season_info to handle_client_version to make up.
void send_season_info( Client* client )
{
	if (client->getversiondetail().major>=1)
    {
		PktOut_BC* msg = REQUESTPACKET(PktOut_BC,PKTOUT_BC_ID);
		msg->Write(static_cast<u8>(client->chr->realm->season()));
		msg->Write(static_cast<u8>(PKTOUT_BC::PLAYSOUND_YES));
		client->transmit( &msg->buffer, msg->offset );
		READDPACKET(msg);

		// Sending Season info resets light level in client, this fixes it during login
		if (client->gd->weather_region != NULL &&
			client->gd->weather_region->lightoverride != -1 &&
			client->chr->lightoverride == -1)
		{
			send_light( client, client->gd->weather_region->lightoverride );
		}
	}
}

//assumes new realm has been set on client->chr
void send_new_subserver( Client* client )
{
	PktOut_76* msg = REQUESTPACKET(PktOut_76,PKTOUT_76_ID);
	msg->WriteFlipped(client->chr->x);
	msg->WriteFlipped(client->chr->y);
	msg->WriteFlipped(static_cast<u16>(client->chr->z));
	msg->offset+=5; //unk0,x1,y2
	msg->WriteFlipped(client->chr->realm->width());
	msg->WriteFlipped(client->chr->realm->height());
	client->transmit( &msg->buffer, msg->offset );
	READDPACKET(msg);
}

void send_fight_occuring( Client* client, Character* opponent )
{
	PktOut_2F* msg = REQUESTPACKET(PktOut_2F,PKTOUT_2F_ID);
	msg->offset++; //zero1
	msg->Write(client->chr->serial_ext);
	msg->Write(opponent->serial_ext);
	client->transmit( &msg->buffer, msg->offset );
	READDPACKET(msg);
}

void send_damage( Character* attacker, Character* defender, u16 damage )
{
    if (attacker->client != NULL)
    {
        if (attacker->client->ClientType & CLIENTTYPE_4070)
            send_damage_new(attacker->client, defender, damage);
        else
            send_damage_old(attacker->client, defender, damage);
    }
    if ((defender->client != NULL) && (attacker != defender))
    {
        if (defender->client->ClientType & CLIENTTYPE_4070)
            send_damage_new(defender->client, defender, damage);
        else
            send_damage_old(defender->client, defender, damage);
    }
}

void send_damage_new(Client* client, Character* defender, u16 damage)
{
	PktOut_0B* msg = REQUESTPACKET(PktOut_0B,PKTOUT_0B_ID);
	msg->Write(defender->serial_ext);
	msg->WriteFlipped(damage);
    client->transmit( &msg->buffer, msg->offset );
	READDPACKET(msg);
}

void send_damage_old(Client* client, Character* defender, u16 damage)
{
	PktOut_BF_Sub22* msg = REQUESTSUBPACKET(PktOut_BF_Sub22,PKTBI_BF_ID,PKTBI_BF::TYPE_DAMAGE);
	msg->WriteFlipped(static_cast<u16>(11));
	msg->offset+=2; //sub
	msg->Write(static_cast<u8>(1));
	msg->Write(defender->serial_ext);
    if (damage > 0xFF)
        msg->Write(static_cast<u8>(0xFF));
    else
        msg->Write(static_cast<u8>(damage));
    client->transmit( &msg->buffer, msg->offset );
	READDPACKET(msg);
}

void sendCharProfile( Character* chr, Character* of_who, const char *title, const u16 *utext, const u16 *etext )
{
	PktOut_B8* msg = REQUESTPACKET(PktOut_B8,PKTBI_B8_OUT_ID);

	unsigned newulen = 0, newelen = 0, titlelen;

	while( utext[newulen] != L'\0' )
		++newulen;

	while( etext[newelen] != L'\0' )
		++newelen;

	titlelen = strlen(title);

	// Check Lengths

    if (titlelen > SPEECH_MAX_LEN)
        titlelen = SPEECH_MAX_LEN;
	if (newulen > SPEECH_MAX_LEN)
		newulen = SPEECH_MAX_LEN;
	if (newelen > SPEECH_MAX_LEN)
		newelen = SPEECH_MAX_LEN;

	// Build Packet
	msg->offset+=2;
	msg->Write(of_who->serial_ext);
	msg->Write(title,static_cast<u16>(titlelen+1));
	msg->WriteFlipped(utext,static_cast<u16>(newulen));
	msg->WriteFlipped(etext,static_cast<u16>(newelen));
	u16 len=msg->offset;
	msg->offset=1;
	msg->WriteFlipped(len);

	transmit( chr->client, &msg->buffer, len );
	READDPACKET(msg);
}
