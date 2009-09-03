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

Notes
=======

*/

#include "clib/stl_inc.h"

#include <assert.h>
#include <stddef.h>

#include "clib/clib.h"
#include "clib/endian.h"
#include "clib/logfile.h"
#include "clib/passert.h"
#include "clib/stlutil.h"
#include "clib/strutil.h"

#include "plib/mapcell.h"
#include "plib/realm.h"

#include "account.h"
#include "charactr.h"
#include "client.h"
#include "equipmnt.h"
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
#include "multi.h"
#include "uobject.h"
#include "uoclient.h"
#include "uofile.h"
#include "ustruct.h"
#include "uvars.h"
#include "uworld.h"

#include "bscript/impstr.h"

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
    PKTOUT_20 msg;
    msg.msgtype = PKTOUT_20_ID;
    msg.serial = chr->serial_ext;
    msg.graphic = chr->graphic_ext;
    msg.unk7 = 0;
    msg.color = chr->color_ext;
	msg.flag1 = chr->get_flag1();
    msg.x = ctBEu16( chr->x );
    msg.y = ctBEu16( chr->y );
    msg.unk15 = 0;
    msg.unk16 = 0;
    msg.dir = 0x80 | chr->facing; // is it always right to set this flag?
    msg.z = chr->z;
    transmit( client, &msg, sizeof msg );
}

// Character chr has moved.  Tell a client about it.
// FIXME: Use of this packet needs optimized. Other functions for
// sending to in range etc, call this function for each client.
// Even with the independant flags, we should be able to
// optimize this out to reduce build amounts
void send_move( Client *client, const Character *chr )
{
    PKTOUT_77 msg;

    msg.msgtype = PKTOUT_77_ID;
    msg.serial = chr->serial_ext;
    msg.graphic = chr->graphic_ext;
    msg.x = ctBEu16( chr->x );
    msg.y = ctBEu16( chr->y );
    msg.z = chr->z;
    msg.dir = (chr->dir & 0x80) | chr->facing; // NOTE, this only includes mask 0x07 of the last MOVE message 
    msg.skin = chr->color_ext;
	msg.flag1 = chr->get_flag1();
    msg.hilite = chr->hilite_color_idx( client->chr );

    transmit( client, &msg, sizeof msg );
}

void send_owncreate( Client *client, const Character *chr )
{
    int num_worn_bytes;
    unsigned short msglen;

    PKTOUT_78 owncreate;
    owncreate.msgtype = PKTOUT_78_ID;
    owncreate.serial = chr->serial_ext;
    owncreate.graphic = chr->graphic_ext;
    owncreate.x = ctBEu16(chr->x);
    owncreate.y = ctBEu16(chr->y);
    owncreate.z = chr->z;
    owncreate.facing = chr->facing;
    owncreate.skin = chr->color_ext;
	// MuadDib changed to reflect true status for 0x20 packet. 1/4/2007
	// Undo Chance
	owncreate.flag1 = chr->get_flag1();
    owncreate.hilite = chr->hilite_color_idx( client->chr ); 

    
    num_worn_bytes = 0;
    for( int layer = LAYER_EQUIP__LOWEST; layer <= LAYER_EQUIP__HIGHEST; ++layer )
    {
        const Item *item = chr->wornitem( layer );
        if (item == NULL) 
            continue;

        if (item->color)
        {
            PKTOUT_78_COLOR *add = (PKTOUT_78_COLOR *) &owncreate.wornitems[ num_worn_bytes ];
            add->serial = item->serial_ext;
            add->graphic = ctBEu16( 0x8000 | item->graphic );
            add->layer = static_cast<u8>(layer);
            add->color = item->color_ext;
            num_worn_bytes += sizeof *add;
        }
        else
        {
            PKTOUT_78_NOCOLOR *add = (PKTOUT_78_NOCOLOR *) &owncreate.wornitems[ num_worn_bytes ];
            add->serial = item->serial_ext;
            add->graphic = item->graphic_ext;
            add->layer = static_cast<u8>(layer);
            num_worn_bytes += sizeof *add;
        }
    }
    
    owncreate.wornitems[num_worn_bytes++] = 0x00;
    owncreate.wornitems[num_worn_bytes++] = 0x00;
    owncreate.wornitems[num_worn_bytes++] = 0x00;
    owncreate.wornitems[num_worn_bytes++] = 0x00;

    msglen = static_cast<unsigned short>(offsetof(PKTOUT_78, wornitems ) + num_worn_bytes);
    
    passert( msglen <= sizeof owncreate );
    
    owncreate.msglen = ctBEu16( msglen );

    transmit(client, &owncreate, msglen );

	if(client->UOExpansionFlag & AOS)
	{
		send_object_cache(client, dynamic_cast<const UObject*>(chr));
		// 07/11/09 Turley: moved to bottom first the client needs to know the item then we can send revision
		for( int layer = LAYER_EQUIP__LOWEST; layer <= LAYER_EQUIP__HIGHEST; ++layer )
		{
			const Item *item = chr->wornitem( layer );
			if (item == NULL) 
				continue;
			send_object_cache(client, dynamic_cast<const UObject*>(item));
		}
	}
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

    PKTOUT_1D msg;
    msg.msgtype = PKTOUT_1D_ID;
    msg.serial = chr->serial_ext;
    transmit( client, &msg, sizeof msg );
}


void send_remove_character_if_nearby( Client* client, const Character* chr )
{
    if (!client->ready)     /* if a client is just connecting, don't bother him. */
        return;

    if (!inrange( client->chr, chr))
        return;

    if (client->chr == chr)
        return;

    PKTOUT_1D msg;
    msg.msgtype = PKTOUT_1D_ID;
    msg.serial = chr->serial_ext;
    transmit( client, &msg, sizeof msg );
}
void send_remove_character_to_nearby( const Character* chr )
{
    ForEach( clients, send_remove_character_if_nearby, chr );
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
        PKTOUT_1D msg;
        msg.msgtype = PKTOUT_1D_ID;
        msg.serial = chr->serial_ext;
        transmit( client, &msg, sizeof msg );
    }
}
void send_remove_character_to_nearby_cantsee( const Character* chr )
{
    ForEach( clients, send_remove_character_if_nearby_cantsee, chr );
}

void send_remove_character_if_nearby_cansee( Client* client, const Character* chr )
{
    if (client->ready &&
        inrange( client->chr, chr) &&
        client->chr != chr &&
        client->chr->is_visible_to_me( chr ))
    {
        PKTOUT_1D msg;
        msg.msgtype = PKTOUT_1D_ID;
        msg.serial = chr->serial_ext;
        transmit( client, &msg, sizeof msg );
    }
}
void send_remove_character_to_nearby_cansee( const Character* chr )
{
    ForEach( clients, send_remove_character_if_nearby_cansee, chr );
}



void send_remove_object_if_inrange( Client *client, const Item *item )
{
    if (!client->ready)     /* if a client is just connecting, don't bother him. */
        return;

    if (!inrange( client->chr, item))
        return;

    PKTOUT_1D msg;
    msg.msgtype = PKTOUT_1D_ID;
    msg.serial = item->serial_ext;
    transmit( client, &msg, sizeof msg );
}

void send_remove_object_to_inrange( const UObject *centerObject )
{
    PKTOUT_1D msg;
    msg.msgtype = PKTOUT_1D_ID;
    msg.serial = centerObject->serial_ext;

	for( Clients::iterator itr = clients.begin(), end = clients.end(); itr != end; ++itr )
	{
        Client *client2 = *itr;
	    if (!client2->ready)
		    continue;
	    // FIXME need to check character's additional_legal_items.
	    // looks like inrange should be a Character member function.
		if (inrange(client2->chr, centerObject))
		{
			transmit( client2, &msg, sizeof msg );
		}
	}
}

void send_remove_object( Client *client, const Item *item )
{
    PKTOUT_1D msg;
    msg.msgtype = PKTOUT_1D_ID;
    msg.serial = item->serial_ext;
    transmit( client, &msg, sizeof msg );
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
    return inrangex( c1, c2, RANGE_SAY );
}
bool in_yell_range( const Character *c1, const Character *c2 )
{
    return inrangex( c1, c2, RANGE_YELL );
}
bool in_whisper_range( const Character *c1, const Character *c2 )
{
    return inrangex( c1, c2, RANGE_WHISPER );
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
	if ( client->is_greq_6017 )
	{
		PKTOUT_25_V2 slot_buffer;
		BuildMsg25(item, &slot_buffer);
		client->transmit(&slot_buffer, sizeof slot_buffer);
	}
	else
	{
		PKTOUT_25_V1 legacy_buffer;
		BuildMsg25(item, &legacy_buffer);
		client->transmit(&legacy_buffer, sizeof legacy_buffer);
	}
	if(client->UOExpansionFlag & AOS)
	{
	    send_object_cache(client, dynamic_cast<const UObject*>(item));
	}
}

void send_put_in_container_to_inrange( const Item *item )
{
	// FIXME there HAS to be a better, more efficient way than this, without
	// building these with pointer and NULL to check. Cuz that method requires
	// recast and delete. Ewwww.
	bool slot_built = false;
	bool legacy_built = false;
	PKTOUT_25_V1 legacy_buffer;
	PKTOUT_25_V2 slot_buffer;

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
			if ( client2->is_greq_6017 )
			{
				if (!slot_built)
					BuildMsg25(item, &slot_buffer);
				client2->transmit(&slot_buffer, sizeof slot_buffer);
			}
			else
			{
				if (!legacy_built)
					BuildMsg25(item, &legacy_buffer);
				client2->transmit(&legacy_buffer, sizeof legacy_buffer);
			}
			if(client2->UOExpansionFlag & AOS)
			{
				send_object_cache(client2, dynamic_cast<const UObject*>(item));
			}
        }
    }
}

void BuildMsg25 (const Item *item, PKTOUT_25_V1 *packet_struct)
{
	packet_struct->msgtype = PKTOUT_25_V1_ID;
	packet_struct->serial = item->serial_ext;
	packet_struct->graphic = item->graphic_ext;
	packet_struct->unk7 = 0;
	packet_struct->amount = ctBEu16( item->get_senditem_amount() );
	packet_struct->x = ctBEu16( item->x );
	packet_struct->y = ctBEu16( item->y );
	packet_struct->container_serial = item->container->serial_ext;
	packet_struct->color = item->color_ext;
}

void BuildMsg25 (const Item *item, PKTOUT_25_V2 *packet_struct)
{
	packet_struct->msgtype = PKTOUT_25_V2_ID;
	packet_struct->serial = item->serial_ext;
	packet_struct->graphic = item->graphic_ext;
	packet_struct->unk7 = 0;
	packet_struct->amount = ctBEu16( item->get_senditem_amount() );
	packet_struct->x = ctBEu16( item->x );
	packet_struct->y = ctBEu16( item->y );
	packet_struct->slotindex = static_cast<u8>(item->slot_index());
	packet_struct->container_serial = item->container->serial_ext;
	packet_struct->color = item->color_ext;
}

// FIXME it would be better to compose this message once and
// send to multiple clients.
// Uh, WTF.  Looks like we have to send a full "container contents"
// message, just to get the clothes on the corpse - without an
// 'open gump' message of course
void send_corpse_items( Client *client, const Item *item )
{
    const UContainer *cont = static_cast<const UContainer *>(item);

    PKTOUT_89 msg;
    unsigned short msglen;
    msg.msgtype = PKTOUT_89_ID;
    msg.serial = item->serial_ext;
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

		msg.layers[ n_layers_found ].layer = item2->layer;
        msg.layers[ n_layers_found ].serial = item2->serial_ext;
        n_layers_found++;
    }
    passert_always( n_layers_found <= NUM_LAYERS );
    msg.layers[ n_layers_found ].layer = 0;
    msglen = static_cast<unsigned short>(
		     offsetof( PKTOUT_89, layers[0] ) +
             n_layers_found * sizeof msg.layers[0] +
             1);         // terminating 'layer 0' byte
    msg.msglen = ctBEu16( msglen );
    transmit( client, &msg, msglen );

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

    if (item->facing == 0)
    {
        PKTOUT_1A_A msg;
        // transmit item info
        msg.msgtype = PKTOUT_1A_A_ID;
        msg.msglen = ctBEu16( sizeof msg );
           // If the 0x80000000 is left out, the item won't show up. 
        msg.serial = ctBEu32( 0x80000000 | item->serial );
        msg.graphic = item->graphic_ext;
        msg.amount = ctBEu16( item->get_senditem_amount() );
        msg.x = ctBEu16( item->x );
        msg.y = ctBEu16( 0xC000 | item->y ); // dyeable and moveable?
        msg.z = item->z;
        msg.color = item->color_ext;
        msg.flags = flags;
        transmit( client, &msg, sizeof msg );
    }
    else
    {
        PKTOUT_1A_B msg;
        // transmit item info
        msg.msgtype = PKTOUT_1A_B_ID;
        msg.msglen = ctBEu16( sizeof msg );
           // If the 0x80000000 is left out, the item won't show up. 
        msg.serial = ctBEu32( 0x80000000 | item->serial );
        msg.graphic = item->graphic_ext;
        msg.amount = ctBEu16( item->get_senditem_amount() );
        msg.x = ctBEu16( 0x8000 | item->x );
        msg.y = ctBEu16( 0xC000 | item->y ); // dyeable and moveable?
        msg.facing = item->facing;
        msg.z = item->z;
        msg.color = item->color_ext;
        msg.flags = flags;
        transmit( client, &msg, sizeof msg );
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
        PKTOUT_4F msg;
        msg.msgtype = PKTOUT_4F_ID;
        msg.lightlevel = static_cast<u8>(lightlevel);
        transmit( client, &msg, sizeof msg );
    }
}

void send_weather( Client* client, u8 type, u8 severity, u8 aux )
{
    PKTOUT_65 msg;
    msg.msgtype = PKTOUT_65_ID;
    msg.type = type;
    msg.severity = severity;
    msg.aux = aux;
    transmit( client, &msg, sizeof msg );
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
    PKTOUT_27 msg;
    msg.msgtype = PKTOUT_27_ID;
    msg.reason = reason;
    transmit( client, &msg, sizeof msg );
}

void send_wornitem( Client *client, const Character *chr, const Item *item )
{
    PKTOUT_2E msg;
    msg.msgtype = PKTOUT_2E_ID;
    msg.serial = item->serial_ext;
    msg.graphic = item->graphic_ext;
    msg.unk7 = 0;
    msg.layer = item->layer;
    msg.worn_by = chr->serial_ext;
    msg.color = item->color_ext;
    transmit( client, &msg, sizeof msg );

	if(client->UOExpansionFlag & AOS)
	{
		send_object_cache(client, dynamic_cast<const UObject*>(item));
	}
}

void send_wornitem_to_inrange( const Character *chr, const Item *item )
{
    PKTOUT_2E msg;
    msg.msgtype = PKTOUT_2E_ID;
    msg.serial = item->serial_ext;
    msg.graphic = item->graphic_ext;
    msg.unk7 = 0;
    msg.layer = item->layer;
    msg.worn_by = chr->serial_ext;
    msg.color = item->color_ext;
	transmit_to_inrange( item, &msg, sizeof msg, false, false );
	send_object_cache_to_inrange( dynamic_cast<const UObject*>(item) );
}

// This used when item already worn and graphic/color changed. Deletes the item
// at client and then sends the new information.
void update_wornitem_to_inrange( const Character *chr, const Item *item )
{
	if(chr != NULL)
	{
		PKTOUT_1D delmsg;
		delmsg.msgtype = PKTOUT_1D_ID;
	    delmsg.serial = item->serial_ext;
		transmit_to_inrange( item, &delmsg, sizeof delmsg, false, false );

		PKTOUT_2E msg;
		msg.msgtype = PKTOUT_2E_ID;
	    msg.serial = item->serial_ext;
		msg.graphic = item->graphic_ext;
	    msg.unk7 = 0;
		msg.layer = item->layer;
	    msg.worn_by = chr->serial_ext;
		msg.color = item->color_ext;
		transmit_to_inrange( item, &msg, sizeof msg, false, false );

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
		if (worn_item->layer != LAYER_HAIR && worn_item->layer != LAYER_BACKPACK_WTF && worn_item->layer != LAYER_BEARD
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
    PKTOUT_54 msg;
    
    msg.msgtype = PKTOUT_54_ID;
    msg.flags = PKTOUT_54::FLAG_SINGLEPLAY;
    msg.effect = ctBEu16( effect-1 );
    msg.volume = ctBEu16( 0 );
    msg.x = ctBEu16( center->x );
    msg.y = ctBEu16( center->y );
    msg.z = 0;
    // FIXME hearing range check perhaps?
    transmit_to_inrange( center, &msg, sizeof msg, false, false );
}

void play_sound_effect_xyz( u16 cx,u16 cy,s8 cz, u16 effect, Realm* realm )
{
    PKTOUT_54 msg;
    
    msg.msgtype = PKTOUT_54_ID;
    msg.flags = PKTOUT_54::FLAG_SINGLEPLAY;
    msg.effect = ctBEu16( effect-1 );
    msg.volume = ctBEu16( 0 );
    msg.x = ctBEu16( cx );
    msg.y = ctBEu16( cy );
    msg.z = cz;

    for( Clients::iterator itr = clients.begin(), end = clients.end(); itr != end; ++itr )
    {
        Client* client = *itr;

        if (!client->ready)
            continue;
		if( client->chr->realm != realm )
			continue;
        if (inrange( client->chr, cx, cy ))
        {
            transmit( client, &msg, sizeof msg );
        }
    }
}

void play_sound_effect_private( const UObject *center, u16 effect, Character* forchr )
{
    if (forchr->client)
    {
        PKTOUT_54 msg;
    
        msg.msgtype = PKTOUT_54_ID;
        msg.flags = PKTOUT_54::FLAG_SINGLEPLAY;
        msg.effect = ctBEu16( effect-1 );
        msg.volume = ctBEu16( 0 );
        msg.x = ctBEu16( center->x );
        msg.y = ctBEu16( center->y );
		msg.z = 0;

        forchr->client->transmit( &msg, sizeof msg );
    }
}

void play_moving_effect( const UObject *src, const UObject *dst,
                         u16 effect,
                         u8 speed,
                         u8 loop,
                         u8 explode)
{
    PKTOUT_70 msg;
    msg.msgtype = PKTOUT_70_ID;
    msg.effect_type = EFFECT_TYPE_MOVING;
    msg.source_serial = src->serial_ext;
    msg.target_serial = dst->serial_ext;
    msg.effect = ctBEu16( effect );
    msg.srcx = ctBEu16( src->x );
    msg.srcy = ctBEu16( src->y );
    msg.srcz = src->z+src->height;
    msg.dstx = ctBEu16( dst->x );
    msg.dsty = ctBEu16( dst->y );
    msg.dstz = dst->z+src->height;
    msg.speed = speed;
    msg.loop = loop;
    msg.unk24 = msg.unk25 = msg.unk26 = 0;
    msg.explode = explode;
 
    for( Clients::iterator itr = clients.begin(), end = clients.end(); itr != end; ++itr )
    {
        Client* client = *itr;

        if (!client->ready)
            continue;
        //  -- in range of either source or destination.
        if (inrange( client->chr, src ) ||
            inrange( client->chr, dst ))
        {
            transmit( client, &msg, sizeof msg );
        }
    }
}

void play_moving_effect2( u16 xs, u16 ys, s8 zs,
                          u16 xd, u16 yd, s8 zd,
                          u16 effect,
                          u8 speed,
                          u8 loop,
                          u8 explode,
						  Realm* realm )
{
    PKTOUT_70 msg;
    msg.msgtype = PKTOUT_70_ID;
    msg.effect_type = EFFECT_TYPE_MOVING;
    msg.effect = ctBEu16( effect );
    msg.srcx = ctBEu16( xs );
    msg.srcy = ctBEu16( ys );
    msg.srcz = zs;
    msg.dstx = ctBEu16( xd );
    msg.dsty = ctBEu16( yd );
    msg.dstz = zd;
    msg.speed = speed;
    msg.loop = loop;
    msg.unk24 = msg.unk25 = msg.unk26 = 0;
    msg.explode = explode;
 
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
            transmit( client, &msg, sizeof msg );
        }
    }
}


void play_lightning_bolt_effect( const UObject* center )
{
    static PKTOUT_70 msg; // local-static
    msg.msgtype = PKTOUT_70_ID;
    msg.effect_type = EFFECT_TYPE_LIGHTNING_BOLT;
    msg.source_serial = center->serial_ext;
    msg.srcx = ctBEu16(center->x);
    msg.srcy = ctBEu16(center->y);
    msg.srcz = center->z;
    transmit_to_inrange( center, &msg, sizeof msg, false, false );
}
 
void play_object_centered_effect( const UObject* center,
                                  u16 effect,
                                  u8 speed,
                                  u8 loop )
{
    static PKTOUT_70 msg; // local-static
    msg.msgtype = PKTOUT_70_ID;
    msg.effect_type = EFFECT_TYPE_FOLLOW_OBJ;
    msg.source_serial = center->serial_ext;
    msg.effect = ctBEu16( effect );
    msg.srcx = ctBEu16(center->x);
    msg.srcy = ctBEu16(center->y);
    msg.srcz = center->z;
    msg.speed = speed;
    msg.loop = loop;
    //msg.unk24 = 1;
    //msg.unk26 = 1;
    transmit_to_inrange( center, &msg, sizeof msg, false, false );
}

void play_stationary_effect( u16 x, u16 y, u8 z, u16 effect, u8 speed, u8 loop, u8 explode, Realm* realm )
{
    static PKTOUT_70 msg; // local-static
    msg.msgtype = PKTOUT_70_ID;
    msg.effect_type = EFFECT_TYPE_STATIONARY_XYZ;
    msg.effect = ctBEu16( effect );
    msg.srcx = ctBEu16(x);
    msg.srcy = ctBEu16(y);
    msg.srcz = z;
    msg.speed = speed;
    msg.loop = loop;
    msg.unk26 = 1; // this is right for teleport, anyway
    msg.explode = explode;

    for( Clients::iterator itr = clients.begin(), end = clients.end(); itr != end; ++itr )
    {
        Client* client = *itr;

        if (!client->ready)
            continue;
		if( client->chr->realm != realm )
			continue;
        if (inrange( client->chr->x, client->chr->y, x, y ))
            client->transmit( &msg, sizeof msg );
    }
}

void play_stationary_effect_ex( u16 x, u16 y, u8 z, Realm* realm, u16 effect, u8 speed, u8 duration, u32 hue, 
							    u32 render, u16 effect3d )
{
	static PKTOUT_C7 msg;
	partical_effect(&msg,PKTOUT_C0::EFFECT_FIXEDXYZ,
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
            client->transmit( &msg, sizeof msg );
    }
}

void play_object_centered_effect_ex( const UObject* center, u16 effect, u8 speed, u8 duration, u32 hue, 
							         u32 render, u8 layer, u16 effect3d )
{
	static PKTOUT_C7 msg;
	partical_effect(&msg,PKTOUT_C0::EFFECT_FIXEDFROM,
		            center->serial_ext,center->serial_ext,
					center->x,center->y,center->z,
					center->x,center->y,center->z,
					effect,speed,duration,1,0,
					hue,render, effect3d, 1, 0,
					center->serial_ext,layer);

    transmit_to_inrange( center, &msg, sizeof msg, false, false );
}

void play_moving_effect_ex( const UObject *src, const UObject *dst,
                            u16 effect, u8 speed, u8 duration, u32 hue, 
							u32 render, u8 direction, u8 explode, 
							u16 effect3d, u16 effect3dexplode, u16 effect3dsound)
{
	static PKTOUT_C7 msg;
	partical_effect(&msg,PKTOUT_C0::EFFECT_MOVING,
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
            transmit( client, &msg, sizeof msg );
        }
    }
}

void play_moving_effect2_ex( u16 xs, u16 ys, s8 zs,
                             u16 xd, u16 yd, s8 zd, Realm* realm,
                             u16 effect, u8 speed, u8 duration, u32 hue, 
							 u32 render, u8 direction, u8 explode, 
							 u16 effect3d, u16 effect3dexplode, u16 effect3dsound)
{
	static PKTOUT_C7 msg;
	partical_effect(&msg,PKTOUT_C0::EFFECT_MOVING,
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
            transmit( client, &msg, sizeof msg );
        }
    }
}

// Central function to build 0xC7 packet
void partical_effect(struct PKTOUT_C7* msg,u8 type, u32 srcserial, u32 dstserial,
					 u16 srcx, u16 srcy, u8 srcz,
					 u16 dstx, u16 dsty, u8 dstz,
					 u16 effect, u8 speed, u8 duration, u8 direction,
					 u8 explode, u32 hue, u32 render, 
					 u16 effect3d, u16 effect3dexplode, u16 effect3dsound,
					 u32 itemid, u8 layer)
{
	msg->header.msgtype         = PKTOUT_C7_ID;
	msg->header.type            = type; 
	msg->header.source_serial   = srcserial;
	msg->header.target_serial   = dstserial;
	msg->header.x_source        = ctBEu16(srcx);
	msg->header.y_source        = ctBEu16(srcy);
	msg->header.z_source        = srcz;
	msg->header.x_target        = ctBEu16(dstx);
	msg->header.y_target        = ctBEu16(dsty);
	msg->header.z_target        = dstz;
	msg->header.itemid          = ctBEu16( effect );
	msg->header.speed           = speed;
	msg->header.duration        = duration;
	msg->header.unk             = 0;
	msg->header.fixed_direction = direction;
	msg->header.explodes        = explode;
	msg->header.hue             = ctBEu32(hue);
	msg->header.render_mode     = ctBEu32(render);
	msg->effect_num             = ctBEu16(effect3d);
	msg->explode_effect_num     = ctBEu16(effect3dexplode);
	msg->explode_sound_num      = ctBEu16(effect3dsound);
	msg->itemid                 = itemid;
	msg->layer                  = layer;
	msg->unk_effect             = 0;
}

// System message -- message in lower left corner
void send_sysmessage(Client *client, const char *text, unsigned short font, unsigned short color )
{
    PKTOUT_1C msg;
    unsigned textlen, msglen;
    
    textlen = strlen(text) + 1;
    if (textlen > sizeof msg.text)
        textlen = sizeof msg.text;
    msglen = offsetof( PKTOUT_1C, text ) + textlen;
    
    if (msglen <= sizeof msg)
    {
        msg.msgtype = PKTOUT_1C_ID;
        msg.msglen = ctBEu16( msglen );
        msg.source_serial = 0x01010101;
        msg.source_graphic = 0x0101;
        msg.type = TEXTTYPE_NORMAL;
        msg.color = ctBEu16( color );
        msg.font = ctBEu16(font);
        memset( msg.speaker_name, '\0', sizeof msg.speaker_name );
        strcpy( msg.speaker_name, "System" );
        memcpy( msg.text, text, textlen-1 );
        msg.text[ textlen-1 ] = '\0';
        transmit( client, &msg, msglen );
    } 
    else     // FIXME need to handle this better
    {
        Log( "send_sysmessage: '%s' is too long\n", text );
    }
}

// Unicode System message -- message in lower left corner
void send_sysmessage(Client *client,
					 const u16 *wtext, const char lang[4],
					 unsigned short font, unsigned short color )
{
	static PKTOUT_AE msg;
	unsigned textlen = 0, msglen;
	//textlen = wcslen((const wchar_t*)wtext) + 1;
	while( wtext[textlen] != L'\0' )
		++textlen;
	textlen += 1;
	if (textlen > (sizeof msg.wtext / sizeof(msg.wtext[0])))
		textlen = (sizeof msg.wtext / sizeof(msg.wtext[0]));
	msglen = offsetof( PKTOUT_AE, wtext ) + textlen*sizeof(msg.wtext[0]);

	if (msglen <= sizeof msg)
	{
		msg.msgtype         = PKTOUT_AE_ID;
		msg.msglen          = ctBEu16( msglen );
		msg.source_serial   = 0x01010101;
		msg.source_graphic  = 0x0101;
		msg.type    = TEXTTYPE_NORMAL;
		msg.color   = ctBEu16( color );
		msg.font    = ctBEu16( font );
		memcpy( &msg.lang, lang, sizeof msg.lang );
		strcpy( msg.speaker_name, "System" );
		for(unsigned i=0; i < textlen; i++)
			msg.wtext[i] = ctBEu16(wtext[i]);
		msg.wtext[textlen] = (u16)0L;
		transmit( client, &msg, msglen );
	}
    else     // FIXME need to handle this better
    {
        Log( "send_sysmessage: unicode text is too long\n");
    }
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
    PKTOUT_1C msg;
    unsigned textlen, msglen;

    textlen = str.length() + 1;
    if (textlen > sizeof msg.text)
        textlen = sizeof msg.text;
    msglen = offsetof( PKTOUT_1C, text ) + textlen;

    passert( msglen <= sizeof msg );
    
    msg.msgtype = PKTOUT_1C_ID;
    msg.msglen = ctBEu16( msglen );
    msg.source_serial = chr->serial_ext;
    msg.source_graphic = 0x0101;
    msg.type = TEXTTYPE_YOU_SEE;
    msg.color = ctBEu16( chr->name_color( client->chr )); // 0x03B2
    msg.font = ctBEu16(3);
    memset( msg.speaker_name, '\0', sizeof msg.speaker_name );
    strzcpy( msg.speaker_name, str.c_str(), sizeof msg.speaker_name );
    memcpy( msg.text, str.c_str(), textlen-1 );
    msg.text[ textlen-1 ] = '\0';
    transmit( client, &msg, msglen );
}

bool say_above(const UObject* obj, 
               const char *text,
               unsigned short font,
               unsigned short color,
			   unsigned long journal_print)
{
    static PKTOUT_1C msg;
    unsigned textlen, msglen;

    textlen = strlen(text) + 1;
    if (textlen > sizeof msg.text)
        textlen = sizeof msg.text;
    msglen = offsetof( PKTOUT_1C, text ) + textlen;

    if (msglen > sizeof msg)
        return false;
    
    msg.msgtype = PKTOUT_1C_ID;
    msg.msglen = ctBEu16( msglen );
    msg.source_serial = obj->serial_ext;
    msg.source_graphic = obj->graphic_ext;
    msg.type = TEXTTYPE_NORMAL;
    msg.color = ctBEu16( color );
    msg.font = ctBEu16( font );
    memset( msg.speaker_name, '\0', sizeof msg.speaker_name );
	switch(journal_print)
	{
		case JOURNAL_PRINT_YOU_SEE:
			strzcpy( msg.speaker_name, "You see", sizeof msg.speaker_name );
			break;

		case JOURNAL_PRINT_NAME:
		default:
			strzcpy( msg.speaker_name, obj->description().c_str(), sizeof msg.speaker_name );
			break;
	}
    memcpy( msg.text, text, textlen );
    
    // todo: only send to those that I'm visible to.
    transmit_to_inrange( obj, &msg, msglen, false, false );
    return true;
}

bool say_above(const UObject* obj, 
               const u16 *wtext, 
			   const char lang[4],
               unsigned short font,
               unsigned short color,
			   unsigned long journal_print)
{
    static PKTOUT_AE msg;
    unsigned textlen = 0, msglen;

	//textlen = wcslen((const wchar_t*)wtext) + 1;
	while( wtext[textlen] != L'\0' )
		++textlen;
	textlen += 1; // include NULL terminator
	if (textlen > (sizeof msg.wtext / sizeof(msg.wtext[0])))
		textlen = (sizeof msg.wtext / sizeof(msg.wtext[0]));
    msglen = offsetof( PKTOUT_AE, wtext ) + textlen*sizeof(msg.wtext[0]);

    if (msglen > sizeof msg)
        return false;
    
    msg.msgtype = PKTOUT_AE_ID;
    msg.msglen = ctBEu16( msglen );
    msg.source_serial = obj->serial_ext;
    msg.source_graphic = obj->graphic_ext;
    msg.type = TEXTTYPE_NORMAL;
    msg.color = ctBEu16( color );
    msg.font = ctBEu16( font );
	memcpy( &msg.lang, lang, sizeof msg.lang );
    memset( msg.speaker_name, '\0', sizeof msg.speaker_name );
    switch(journal_print)
	{
		case JOURNAL_PRINT_YOU_SEE:
			strzcpy( msg.speaker_name, "You see", sizeof msg.speaker_name );
			break;

		case JOURNAL_PRINT_NAME:
		default:
			strzcpy( msg.speaker_name, obj->description().c_str(), sizeof msg.speaker_name );
			break;
	}
	for(unsigned i=0; i < textlen; i++)
		msg.wtext[i] = ctBEu16(wtext[i]);
	msg.wtext[textlen] = (u16)0L;

    // todo: only send to those that I'm visible to.
    transmit_to_inrange( obj, &msg, msglen, false, false );
    return true;
}

bool private_say_above( Character* chr, 
                        const UObject* obj, 
                        const char *text,
                        unsigned short font,
                        unsigned short color,
						unsigned long journal_print)
{
    static PKTOUT_1C msg;
    unsigned textlen, msglen;

    textlen = strlen(text) + 1;
    if (textlen > sizeof msg.text)
        textlen = sizeof msg.text;
    msglen = offsetof( PKTOUT_1C, text ) + textlen;

    if (msglen > sizeof msg || chr->client == NULL)
        return false;
    
    msg.msgtype = PKTOUT_1C_ID;
    msg.msglen = ctBEu16( msglen );
    msg.source_serial = obj->serial_ext;
    msg.source_graphic = obj->graphic_ext;
    msg.type = TEXTTYPE_NORMAL;
    msg.color = ctBEu16( color );
    msg.font =ctBEu16( font );
    memset( msg.speaker_name, '\0', sizeof msg.speaker_name );
	switch(journal_print)
	{
		case JOURNAL_PRINT_YOU_SEE:
			strzcpy( msg.speaker_name, "You see", sizeof msg.speaker_name );
			break;

		case JOURNAL_PRINT_NAME:
		default:
			strzcpy( msg.speaker_name, obj->description().c_str(), sizeof msg.speaker_name );
			break;
	}
    memcpy( msg.text, text, textlen );
    chr->client->transmit( &msg, msglen );
    return true;
}

bool private_say_above( Character* chr, 
                        const UObject* obj, 
                        const u16 *wtext, 
						const char lang[4],
                        unsigned short font,
                        unsigned short color,
						unsigned long journal_print )
{
    static PKTOUT_AE msg;
    unsigned textlen = 0, msglen;

    //textlen = wcslen((const wchar_t*)wtext) + 1;
    while( wtext[textlen] != L'\0' )
    	++textlen;
    textlen += 1; //include null terminator
	if (textlen > (sizeof msg.wtext / sizeof(msg.wtext[0])))
		textlen = (sizeof msg.wtext / sizeof(msg.wtext[0]));
    msglen = offsetof( PKTOUT_AE, wtext ) + textlen*sizeof(msg.wtext[0]);

    if (msglen > sizeof msg || chr->client == NULL)
        return false;
    
    msg.msgtype = PKTOUT_AE_ID;
    msg.msglen = ctBEu16( msglen );
    msg.source_serial = obj->serial_ext;
    msg.source_graphic = obj->graphic_ext;
    msg.type = TEXTTYPE_NORMAL;
    msg.color = ctBEu16( color );
    msg.font = ctBEu16( font );
	memcpy( &msg.lang, lang, sizeof msg.lang );
    memset( msg.speaker_name, '\0', sizeof msg.speaker_name );
    switch(journal_print)
	{
		case JOURNAL_PRINT_YOU_SEE:
			strzcpy( msg.speaker_name, "You see", sizeof msg.speaker_name );
			break;

		case JOURNAL_PRINT_NAME:
		default:
			strzcpy( msg.speaker_name, obj->description().c_str(), sizeof msg.speaker_name );
			break;
	}
	for(unsigned i=0; i < textlen; i++)
		msg.wtext[i] = ctBEu16(wtext[i]);
	msg.wtext[textlen] = (u16)0L;

    chr->client->transmit( &msg, msglen );
    return true;
}

bool private_say_above_ex( Character* chr, 
                           const UObject* obj, 
                           const char *text,
                           unsigned short color )
{
    static PKTOUT_1C msg;
    unsigned textlen, msglen;

    textlen = strlen(text) + 1;
    if (textlen > sizeof msg.text)
        textlen = sizeof msg.text;
    msglen = offsetof( PKTOUT_1C, text ) + textlen;

    if (msglen > sizeof msg || chr->client == NULL)
        return false;
    
    msg.msgtype = PKTOUT_1C_ID;
    msg.msglen = ctBEu16( msglen );
    msg.source_serial = obj->serial_ext;
    msg.source_graphic = obj->graphic_ext;
    msg.type = TEXTTYPE_NORMAL;
    msg.color = ctBEu16( color );
    msg.font = ctBEu16( 3 );
    memset( msg.speaker_name, '\0', sizeof msg.speaker_name );
    strzcpy( msg.speaker_name, obj->description().c_str(), sizeof msg.speaker_name );
    memcpy( msg.text, text, textlen );
    chr->client->transmit( &msg, msglen );
    return true;
}

void send_objdesc( Client *client, Item *item )
{
    PKTOUT_1C msg;
    unsigned textlen, msglen;

    std::string desc = item->description();
    if (desc.length() >= sizeof msg.text)
        desc.resize( sizeof msg.text -1 );

    strcpy( msg.text, desc.c_str() );
    textlen = desc.length() + 1;

    msglen = offsetof( PKTOUT_1C, text ) + textlen;
    passert( msglen < sizeof msg );
    msg.msgtype = PKTOUT_1C_ID;
    msg.msglen = ctBEu16( msglen );
    msg.source_serial = item->serial_ext;
    msg.source_graphic = item->graphic_ext;
    msg.type = TEXTTYPE_YOU_SEE;
    msg.color = ctBEu16( 0x03B2 );
    msg.font = ctBEu16( 3 );

    memset( msg.speaker_name, '\0', sizeof msg.speaker_name );
    strcpy( msg.speaker_name, "System" );

    transmit( client, &msg, msglen );
}

void send_stamina_level( Client *client )
{
    Character *chr = client->chr;

    PKTOUT_A3 msg;
    msg.msgtype = PKTOUT_A3_ID;
    msg.serial = chr->serial_ext;
    if (uoclient_general.stamina.any)
    {
        long v = chr->vital(uoclient_general.stamina.id).current_ones();
        if (v > 0xFFFF)
            v = 0xFFFF;
        msg.stamina = ctBEu16( static_cast<u16>(v) );

        v = chr->vital( uoclient_general.stamina.id ).maximum_ones();
        if (v > 0xFFFF)
            v = 0xFFFF;

	    msg.max_stamina = ctBEu16( static_cast<u16>(v) );
    }
    else
    {
        msg.stamina = 0;
	    msg.max_stamina = 0;
    }
    transmit( client, &msg, sizeof msg );
}
void send_mana_level( Client *client )
{
    Character *chr = client->chr;

	PKTOUT_A2 msg;
    msg.msgtype = PKTOUT_A2_ID;
    msg.serial = chr->serial_ext;
    if (uoclient_general.mana.any)
    {
        long v = chr->vital(uoclient_general.mana.id).current_ones();
        if (v > 0xFFFF)
            v = 0xFFFF;
        msg.mana = ctBEu16( static_cast<u16>(v) );

        v = chr->vital(uoclient_general.mana.id).maximum_ones();
        if (v > 0xFFFF)
            v = 0xFFFF;
	    msg.max_mana = ctBEu16( static_cast<u16>(v) );
    }
    else
    {
        msg.mana = 0;
	    msg.max_mana = 0;
    }
    transmit( client, &msg, sizeof msg );
}

void send_death_message( Character *chr_died, Item *corpse )
{
    PKTOUT_AF msg;
    msg.msgtype = PKTOUT_AF_ID;
    msg.player_id = chr_died->serial_ext;
    msg.corpse_id = corpse->serial_ext;
    msg.unk4_zero = 0x00000000;

    for( Clients::iterator itr = clients.begin(), end = clients.end(); itr != end; ++itr )
    {
        Client *client = *itr;
        if (!client->ready)
            continue;
        
        if (client->chr == chr_died)
            continue;
        
        if (inrange( client->chr, corpse ))
            transmit( client, &msg, sizeof msg );
    }

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
		if (is_6017 && !client->is_greq_6017)
		{
			continue;
		}
		if (is_UOKR && !client->isUOKR)
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
		if (is_6017 && !client->is_greq_6017)
		{
			continue;
		}
		if (is_UOKR && !client->isUOKR)
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
	Realm* realm = (Realm*)arg;
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

    PKTOUT_1D msg;
    msg.msgtype = PKTOUT_1D_ID;
    msg.serial = item->serial_ext;

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
                transmit( client, &msg, sizeof msg );
            }
        }
    }

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

    PKTOUT_1D msg;
    msg.msgtype = PKTOUT_1D_ID;
    msg.serial = item->serial_ext;

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
                transmit( client, &msg, sizeof msg );
            }
        }
    }
}

void move_boat_item( Item* item, unsigned short newx, unsigned short newy, signed char newz, Realm* oldrealm )
{
    u16 oldx = item->x;
    u16 oldy = item->y;

    item->x = newx;
    item->y = newy;
    item->z = newz;

    MoveItemWorldPosition( oldx, oldy, item, oldrealm );

    PKTOUT_1A_C msg;
    msg.msgtype = PKTOUT_1A_C_ID;
    msg.msglen = ctBEu16( sizeof msg );
    msg.serial = item->serial_ext;
    msg.graphic = item->graphic_ext;
    msg.x = ctBEu16( item->x );
    msg.y = ctBEu16( item->y ); 
    msg.z = item->z;

    PKTOUT_1D rmv_msg;
    rmv_msg.msgtype = PKTOUT_1D_ID;
    rmv_msg.serial = item->serial_ext;

    for( Clients::iterator itr = clients.begin(), end = clients.end(); itr != end; ++itr )
    {
        Client *client = *itr;
        if (!client->ready)
            continue;

        if (inrange( client->chr, item ))
        {
            transmit( client, &msg, sizeof msg );
        }
        else // not in range.  If old loc was in range, send a delete.
        {
            if (inrange( client->chr->x, client->chr->y,
                         oldx, oldy ))
            {
                transmit( client, &rmv_msg, sizeof rmv_msg );
            }
        }
    }

}

#include "multi.h"
void send_multi( Client* client, UMulti* multi )
{
    static PKTOUT_1A_C msg;
    // transmit item info
    msg.msgtype = PKTOUT_1A_C_ID;
    msg.msglen = ctBEu16( sizeof msg );
    msg.serial = multi->serial_ext;
    msg.graphic = multi->graphic_ext;
    msg.x = ctBEu16( multi->x );
    msg.y = ctBEu16( multi->y );
    msg.z = multi->z;

    client->transmit( &msg, sizeof msg );
}

#include "zone.h"
#include "miscrgn.h"
#include "cgdata.h"
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

#include "cgdata.h"
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
string format_description( unsigned long polflags, const string& descdef, unsigned short amount )
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
		else // phase can only be 3
		{
			desc += ch;
		}
		
		++src;
	}
	
    if (!singular && !plural_handled)
		desc += 's';

    return desc;
}

void send_midi( Client* client, u16 midi )
{
    PKTOUT_6D msg;
    msg.msgtype = PKTOUT_6D_ID;
    msg.midi = ctBEu16( midi );
    // cout << "Setting midi for " << client->chr->name() << " to " << midi << endl;
    client->transmit( &msg, sizeof msg );
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
			PKTOUT_1D delmsg;
			delmsg.msgtype = PKTOUT_1D_ID;
			delmsg.serial = item->serial_ext;
			transmit_to_inrange( item, &delmsg, sizeof delmsg, false, false );
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
	PKTOUT_55 msg;
	msg.msgtype = PKTOUT_55_ID;
	c->transmit(&msg, sizeof msg);
}

void send_feature_enable(Client* client)
{
	u16 clientflag = 0;
	string uo_expansion = client->acct->uo_expansion();
	if(uo_expansion.find("KR") != string::npos)
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

	if (client->is_greq_60142)
	{
		PKTOUT_B9_V2 msg;
		msg.msgtype = PKTOUT_B9_V2_ID;
		msg.enable = ctBEu32((u32)clientflag);
		client->transmit(&msg, sizeof msg);
	}
	else
	{
		PKTOUT_B9 msg;
		msg.msgtype = PKTOUT_B9_ID;
		msg.enable = ctBEu16(clientflag);
		client->transmit(&msg, sizeof msg);
	}
}

void send_realm_change( Client* client, Realm* realm )
{
	PKTBI_BF msg;
	u16 msglen = offsetof( PKTBI_BF, cursorhue ) + 1;
	msg.msgtype = PKTBI_BF_ID;
    msg.msglen =  ctBEu16(msglen);
    msg.subcmd = ctBEu16(PKTBI_BF::TYPE_CURSOR_HUE);
	msg.cursorhue = static_cast<u8>(realm->getUOMapID());
	client->transmit(&msg, msglen);
}

void send_map_difs( Client* client )
{
	PKTBI_BF msg;
	msg.msgtype = PKTBI_BF_ID;
	msg.msglen = ctBEu16(9 + (Realms->size()*8));
	msg.subcmd = ctBEu16(PKTBI_BF::TYPE_ENABLE_MAP_DIFFS);
	msg.mapdiffs.num_maps = ctBEu32(baserealm_count);
	for(unsigned int i=0; i<baserealm_count; i++)
	{
		msg.mapdiffs.elems[i].num_map_patches = ctBEu32(Realms->at(i)->getNumMapPatches());
		msg.mapdiffs.elems[i].num_static_patches = ctBEu32(Realms->at(i)->getNumStaticPatches());
	}
	client->transmit( &msg, cfBEu16(msg.msglen));
}

// FIXME : Works, except for Login. Added length check as to not mess with 1.x clients
//         during login. Added send_season_info to handle_client_version to make up.
void send_season_info( Client* client )
{
	if (client->getversiondetail().major>=1)
    {
		PKTOUT_BC msg;
        msg.msgtype = PKTOUT_BC_ID;
		msg.season = static_cast<u8>(client->chr->realm->season());
		msg.playsound = PKTOUT_BC::PLAYSOUND_YES;

		client->transmit( &msg, sizeof msg );
	}
}

//assumes new realm has been set on client->chr
void send_new_subserver( Client* client )
{
	PKTOUT_76 msg;
	msg.msgtype = PKTOUT_76_ID;
	msg.xloc = ctBEu16(client->chr->x);
	msg.yloc = ctBEu16(client->chr->y);
	msg.zloc = ctBEu16(client->chr->z);
	msg.unk0 = 0;
	msg.x1 = 0;
	msg.y1 = 0;
	msg.x2 = ctBEu16(client->chr->realm->width());
	msg.y2 = ctBEu16(client->chr->realm->height());

	client->transmit( &msg, sizeof msg );
}

void send_fight_occuring( Client* client, Character* opponent )
{
	PKTOUT_2F msg;
	msg.msgtype = PKTOUT_2F_ID;
	msg.zero1 = 0;
	msg.attacker_serial = client->chr->serial_ext;
	msg.defender_serial = opponent->serial_ext;

	client->transmit( &msg, sizeof msg );
}


