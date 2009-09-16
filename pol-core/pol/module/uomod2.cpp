/*
History
=======
2005/03/02 Shinigami: mf_MoveObjectToRealm - fixed item detection and added container handling
2005/06/11 Shinigami: added polcore().internal - just for internal Development (not documented)
2005/06/20 Shinigami: added memory log to polcore().internal (needs defined MEMORYLEAK)
2005/07/25 Shinigami: doubled Msg size in mf_SendGumpMenu to use larger Gumps
2005/10/16 Shinigami: added x- and y-offset to mf_SendGumpMenu
2005/11/26 Shinigami: changed "strcmp" into "stricmp" to suppress Script Errors
2006/05/07 Shinigami: mf_SendBuyWindow & mf_SendSellWindow - added Flags to send Item Description using AoS Tooltips
2006/05/24 Shinigami: added mf_SendCharacterRaceChanger - change Hair, Beard and Color
                      added character_race_changer_handler()
2006/05/30 Shinigami: Changed params of character_race_changer_handler()
                      Fixed Bug with detection of Gump-Cancel in uo::SendCharacterRaceChanger()
2006/09/23 Shinigami: Script_Cycles, Sleep_Cycles and Script_passes uses 64bit now
2007/04/28 Shinigami: polcore().internal information will be logged in excel-friendly format too (leak.log)
2009/07/23 MuadDib:   updates for new Enum::Packet Out ID
2009/08/06 MuadDib:   Removed PasswordOnlyHash support
2009/09/03 MuadDib:   Relocation of account related cpp/h
                      Relocation of multi related cpp/h
2009/09/10 Turley:    CompressedGump support (Grin)
2009/09/06 Turley:    Changed Version checks to bitfield client->ClientType

Notes
=======

*/

#include "../../clib/stl_inc.h"

/*
	UOEMOD2.CPP - a nice place for the Buy/Sell Interface Functions
*/

#include "uomod.h"

#include "../../bscript/berror.h"

#ifdef MEMORYLEAK
#	include "bscript/bobject.h"
#endif

#include "../../bscript/executor.h"
#include "../../bscript/impstr.h"

#include "../../clib/endian.h"
#include "../../clib/fdump.h"
#include "../../clib/logfile.h"

#ifdef MEMORYLEAK
#	include "clib/opnew.h"
#endif

#include "../../clib/strutil.h"
#include "../../clib/unicode.h"
#include "../../clib/MD5.h"
#include "../../clib/stlutil.h"

#include "../../plib/realm.h"

#include "../accounts/account.h"

#ifdef MEMORYLEAK
#	include "cfgrepos.h"
#endif

#include "../network/cgdata.h"
#include "../mobile/charactr.h"
#include "../network/client.h"
#include "../core.h"
#include "../exscrobj.h"
#include "../fnsearch.h"
#include "../multi/house.h"
#include "../network/iostats.h"
#include "../item/itemdesc.h"
#include "../npc.h"
#include "../objtype.h"
#include "osmod.h"
#include "../pktboth.h"
#include "../pktin.h"
#include "../pktout.h"
#include "../../plib/pkg.h"
#include "../polcfg.h"
#include "../polclass.h"
#include "../polstats.h"
#include "../poltimer.h"
#include "../../plib/polver.h"
#include "../profile.h"
#include "../realms.h"
#include "../scrsched.h"
#include "../scrstore.h"
#include "../sockio.h"
#include "../statmsg.h"
#include "../syshook.h"
#include "../tooltips.h"
#include "../ufunc.h"
#include "../uoexec.h"
#include "../uofile.h"
#include "../uoscrobj.h"
#include "../uvars.h"
#include "../uworld.h"
#include "../../../lib/zlib/zlib.h"

static char buffer[ 65535 ];

/*
0000: 74 02 70 40 29 ca d8 28  00 00 00 03 0b 53 65 77   t.p@)..( .....Sew
0010: 69 6e 67 20 6b 69 74 00  00 00 00 0d 09 53 63 69   ing kit. .....Sci
0020: 73 73 6f 72 73 00 00 00  00 09 0a 44 79 69 6e 67   ssors... ...Dying
0030: 20 74 75 62 00 00 00 00  09 05 44 79 65 73 00 00	tub.... ..Dyes..
0040: 00 00 1b 08 44 6f 75 62  6c 65 74 00 00 00 00 1a   ....Doub let.....
0050: 0c 53 68 6f 72 74 20 70  61 6e 74 73 00 00 00 00   .Short p ants....
0060: 37 0c 46 61 6e 63 79 20  73 68 69 72 74 00 00 00   7.Fancy  shirt...
0070: 00 26 0b 4c 6f 6e 67 20  70 61 6e 74 73 00 00 00   .&.Long  pants...
0080: 00 19 0c 46 61 6e 63 79  20 64 72 65 73 73 00 00   ...Fancy  dress..
0090: 00 00 39 0c 50 6c 61 69  6e 20 64 72 65 73 73 00   ..9.Plai n dress.
00a0: 00 00 00 21 06 53 6b 69  72 74 00 00 00 00 1f 05   ...!.Ski rt......
00b0: 4b 69 6c 74 00 00 00 00  1a 0b 48 61 6c 66 20 61   Kilt.... ..Half a
00c0: 70 72 6f 6e 00 00 00 00  49 05 52 6f 62 65 00 00   pron.... I.Robe..
00d0: 00 00 32 06 43 6c 6f 61  6b 00 00 00 00 32 06 43   ..2.Cloa k....2.C
00e0: 6c 6f 61 6b 00 00 00 00  1b 08 44 6f 75 62 6c 65   loak.... ..Double
00f0: 74 00 00 00 00 21 06 54  75 6e 69 63 00 00 00 00   t....!.T unic....
0100: 39 0c 4a 65 73 74 65 72  20 73 75 69 74 00 00 00   9.Jester  suit...
0110: 00 1f 0b 4a 65 73 74 65  72 20 68 61 74 00 00 00   ...Jeste r hat...
0120: 00 19 0b 46 6c 6f 70 70  79 20 68 61 74 00 00 00   ...Flopp y hat...
0130: 00 1a 0e 57 69 64 65 2d  62 72 69 6d 20 68 61 74   ...Wide- brim hat
0140: 00 00 00 00 1b 04 43 61  70 00 00 00 00 1a 0f 54   ......Ca p......T
0150: 61 6c 6c 20 73 74 72 61  77 20 68 61 74 00 00 00   all stra w hat...
0160: 00 19 0a 53 74 72 61 77  20 68 61 74 00 00 00 00   ...Straw  hat....
0170: 1e 0d 57 69 7a 61 72 64  27 73 20 68 61 74 00 00   ..Wizard 's hat..
0180: 00 00 1a 07 42 6f 6e 6e  65 74 00 00 00 00 1b 0e   ....Bonn et......
0190: 46 65 61 74 68 65 72 65  64 20 68 61 74 00 00 00   Feathere d hat...
01a0: 00 1a 0d 54 72 69 63 6f  72 6e 65 20 68 61 74 00   ...Trico rne hat.
01b0: 00 00 00 0e 08 42 61 6e  64 61 6e 61 00 00 00 00   .....Ban dana....
01c0: 0d 09 53 6b 75 6c 6c 63  61 70 00 00 00 00 02 0d   ..Skullc ap......
01d0: 46 6f 6c 64 65 64 20 63  6c 6f 74 68 00 00 00 00   Folded c loth....
01e0: 02 0d 46 6f 6c 64 65 64  20 63 6c 6f 74 68 00 00   ..Folded  cloth..
01f0: 00 00 02 0d 46 6f 6c 64  65 64 20 63 6c 6f 74 68   ....Fold ed cloth
0200: 00 00 00 00 02 0d 46 6f  6c 64 65 64 20 63 6c 6f   ......Fo lded clo
0210: 74 68 00 00 00 00 4a 0e  42 6f 6c 74 20 6f 66 20   th....J. Bolt of
0220: 63 6c 6f 74 68 00 00 00  00 4a 0e 42 6f 6c 74 20   cloth... .J.Bolt
0230: 6f 66 20 63 6c 6f 74 68  00 00 00 00 5b 0f 42 61   of cloth ....[.Ba
0240: 6c 65 20 6f 66 20 63 6f  74 74 6f 6e 00 00 00 00   le of co tton....
0250: 2d 0d 50 69 6c 65 20 6f  66 20 77 6f 6f 6c 00 00   -.Pile o f wool..
0260: 00 00 61 0c 46 6c 61 78  20 62 75 6e 64 6c 65 00   ..a.Flax  bundle.
*/
// msg 3C: be sure to not send the whole structure, just as many 'items' as you insert

static unsigned char bfr[ 65535 ];

bool send_vendorwindow_contents( Client* client, UContainer* for_sale, bool send_aos_tooltip )
{
		unsigned short msglen = 0;
		PKTOUT_74::HEADER* header = reinterpret_cast<PKTOUT_74::HEADER*>(bfr);

		header->msgtype = PKTOUT_74_ID;
		/* Length is Below */
		header->container = for_sale->serial_ext;
		header->num_items = 0;
		msglen = sizeof(*header);

		// FIXME: ick! apparently we need to iterate backwards... WTF?
		for( int i = for_sale->count()-1; i >= 0; --i )
		{
			PKTOUT_74::ELEMENT* element = reinterpret_cast<PKTOUT_74::ELEMENT*>(&bfr[msglen]);

			Item* item = (*for_sale)[i];
			// const ItemDesc& id = find_itemdesc( item->objtype_ );
			string desc = item->merchant_description();
			unsigned int addlen = sizeof(*element) + desc.size();
			if (msglen + addlen > sizeof bfr)
				return false;


			strcpy(element->description, desc.c_str() );
			element->desc_len=static_cast<u8>(desc.size())+1;  //Don't forget the NULL
			element->price=ctBEu32(item->sellprice());
			msglen += static_cast<unsigned short>(addlen); // element contains first byte of desc
			++header->num_items;

			if (send_aos_tooltip)
				SendAOSTooltip( client, item, true );
		}

		header->msglen = ctBEu16(msglen);

		transmit( client, bfr, msglen );
		return true;
}

BObjectImp* UOExecutorModule::mf_SendBuyWindow(/* character, container, vendor, items, flags */)
{
	Character *chr, *mrchnt;
	Item *item, *item2;
	NPC *merchant;
	long flags;
	UContainer *for_sale, *bought;
	unsigned char save_layer_one, save_layer_two;
	PKTOUT_24 open_window;

	if( getCharacterParam( exec, 0, chr ) &&
		getItemParam( exec, 1, item ) &&
		getCharacterParam( exec, 2, mrchnt ) &&
		getItemParam( exec, 3, item2 ) &&
		getParam( 4, flags ))
	{
		if (!chr->has_active_client())
		{
			return new BError( "No client connected to character" );
		}

		if( item->isa( UObject::CLASS_CONTAINER ) )
		{
			for_sale = static_cast<UContainer *>(item);
		}
		else
		{
			return new BError( "Parameter 1 invalid" );
		}

		if( mrchnt->isa( UObject::CLASS_NPC ) )
		{
			merchant = static_cast<NPC *>(mrchnt);
		}
		else
		{
			return new BError( "Parameter 2 invalid" );
		}

		if( item2->isa( UObject::CLASS_CONTAINER ) )
		{
			bought = static_cast<UContainer *>(item2);
		}
		else
		{
			return new BError( "Parameter 3 invalid" );
		}

//	  say_above(merchant, "How may I help you?" );

	}
	else
	{
		return new BError( "A parameter was invalid" );
	}


//try this
	save_layer_one = for_sale->layer;
	for_sale->layer = LAYER_VENDOR_FOR_SALE;
	send_wornitem( chr->client, merchant, for_sale );
	for_sale->layer = save_layer_one;
	for_sale->x = merchant->x;
	for_sale->y = merchant->y;
	for_sale->z = merchant->z;
	// chr->add_additional_legal_item( for_sale );

	save_layer_two = bought->layer;
	bought->layer = LAYER_VENDOR_PLAYER_ITEMS;
	send_wornitem( chr->client, merchant, bought );
	bought->layer = save_layer_two;
	bought->x = merchant->x;
	bought->y = merchant->y;
	bought->z = merchant->z;
	// chr->add_additional_legal_item( bought );

	bool send_aos_tooltip = flags & VENDOR_SEND_AOS_TOOLTIP ? true : false;

	chr->client->gd->vendor.set( merchant );
	chr->client->gd->vendor_for_sale.set( for_sale );

	chr->client->pause(); /* Prevent lag time between messages */

	// Send the 'for sale' container
	send_container_contents( chr->client, *for_sale);

	if (!send_vendorwindow_contents( chr->client, for_sale, send_aos_tooltip ))
	{
		chr->client->restart();
		return new BError( "Too much crap in vendor's inventory!" );
	}

	// Send the 'bought' container (?) (hmm, I think this is the player-sold items)
	send_container_contents( chr->client, *bought);

	if (!send_vendorwindow_contents( chr->client, bought, send_aos_tooltip ))
	{
		chr->client->restart();
		return new BError( "Too much crap in vendor's inventory!" );
	}

	//This looks good
	open_window.msgtype = PKTOUT_24_ID;
	open_window.serial = merchant->serial_ext;
	open_window.gump = ctBEu16(0x0030);  // FIXME: Serial of buy gump needs #define or enum?
	transmit( chr->client, &open_window, sizeof open_window );

	// Tell the client how much gold the character has, I guess
	send_full_statmsg(chr->client, chr);

	chr->client->restart();

	return new BLong(1);
}


void send_clear_vendorwindow( Client* client, Character* vendor )
{
	PKTIN_3B msg3b;
	msg3b.msgtype = PKTIN_3B_ID;
	msg3b.msglen = ctBEu16(offsetof( PKTIN_3B, items[0] ));
	msg3b.vendor_serial = vendor->serial_ext;
	msg3b.status = PKTIN_3B::STATUS_NOTHING_BOUGHT;
	client->transmit( &msg3b, offsetof( PKTIN_3B, items[0] ) );
}


//#include "msgfiltr.h"
#include "../msghandl.h"

unsigned long calculate_cost( Character* vendor, UContainer* for_sale, PKTIN_3B *msg )
{
	unsigned long amt = 0;

	int nitems = (cfBEu16( msg->msglen ) - offsetof( PKTIN_3B, items )) /
					 sizeof msg->items[0];

	for( int i = 0; i < nitems; ++i )
	{
		Item* item = for_sale->find( cfBEu32( msg->items[i].item_serial ) );
		if (item == NULL)
			continue;
		//const ItemDesc& id = find_itemdesc(item->objtype_);
		amt += cfBEu16(msg->items[i].number_bought) * item->sellprice();
	}
	return amt;
}

void buyhandler( Client* client, PKTIN_3B* msg)
{
	//Close the gump

	if (msg->status == PKTIN_3B::STATUS_NOTHING_BOUGHT)
		return;

	UContainer* backpack = client->chr->backpack();
	if (backpack == NULL)
		return;

	NPC* vendor = client->gd->vendor.get();

	if (vendor == NULL || vendor->orphan() ||
		vendor->serial_ext != msg->vendor_serial)
	{
		return;
	}
	client->gd->vendor.clear();

	UContainer* for_sale = client->gd->vendor_for_sale.get();
	if (for_sale == NULL || for_sale->orphan())
	{
		return;
	}
	client->gd->vendor_for_sale.clear();

	unsigned long total_cost = calculate_cost( vendor, for_sale, msg );
	if (total_cost > client->chr->gold_carried())
	{
		send_clear_vendorwindow( client, vendor );
		return;
	}

	unsigned long amount_spent = 0;

	// buy each item individually
	// note, we know the buyer can afford it all.
	// the question is, can it all fit in his backpack?
	int nitems = (cfBEu16( msg->msglen ) - offsetof( PKTIN_3B, items )) /
					 sizeof msg->items[0];

	for( int i = 0; i < nitems; ++i )
	{
		Item* fs_item = for_sale->find( cfBEu32( msg->items[i].item_serial ) );
		if (fs_item == NULL)
			continue;
		unsigned short numleft = cfBEu16(msg->items[i].number_bought );
		if (numleft > fs_item->getamount())
			numleft = fs_item->getamount();

		// const ItemDesc& id = find_itemdesc( fs_item->objtype_ );
		while (numleft)
		{
			unsigned short num;
			if ( fs_item->stackable() )
			{
				num = numleft;
			}
			else
			{
				num = 1;
			}
			Item* tobuy = NULL;
			if (fs_item->amount_to_remove_is_partial( num ))
			{
				tobuy = fs_item->remove_part_of_stack(num);
			}
			else
			{
				for_sale->remove( fs_item );
				tobuy = fs_item;
				fs_item = NULL;
			}

			// move the whole item
			// FIXME do stuff like adding to existing stacks.
			ItemRef itemref(tobuy); //dave 1/28/3 prevent item from being destroyed before function ends
			Item* existing_stack;
			if (tobuy->stackable() &&
				(existing_stack = backpack->find_addable_stack( tobuy )))
			{
				//dave 1-14-3 check backpack's insert scripts before moving.
				if(backpack->can_insert_increase_stack( client->chr, UContainer::MT_CORE_MOVED, existing_stack, tobuy->getamount(), tobuy ))
				{
					if(tobuy->orphan()) //dave added 1/28/3, item might be destroyed in RTC script
					{
						continue;
					}
				}
				else // put the item back just as if the pack had too many/too heavy items.
				{
					numleft = 0;
					if (fs_item)
						fs_item->add_to_self( tobuy );
					else
						// FIXME : Add Grid Index Default Location Checks here.
						// Remember, if index fails, move to the ground.
						for_sale->add( tobuy );
					continue;
				}
				numleft -= num;
				amount_spent += tobuy->sellprice() * num;
				u16 amtadded = tobuy->getamount();
				existing_stack->add_to_self(tobuy);
				update_item_to_inrange( existing_stack );

				backpack->on_insert_increase_stack( client->chr, UContainer::MT_CORE_MOVED, existing_stack, amtadded );
			}
			else if (backpack->can_add( *tobuy ))
			{
				numleft -= num;

				//dave 12-20 check backpack's insert scripts before moving.
				bool canInsert = backpack->can_insert_add_item( client->chr, UContainer::MT_CORE_MOVED, tobuy );
				if(tobuy->orphan()) //dave added 1/28/3, item might be destroyed in RTC script
				{
					continue;
				}

				if (!canInsert) // put the item back just as if the pack had too many/too heavy items.
				{
					numleft = 0;
					if (fs_item)
						fs_item->add_to_self( tobuy );
					else
						// FIXME : Add Grid Index Default Location Checks here.
						// Remember, if index fails, move to the ground.
						for_sale->add( tobuy );
					continue;
				}

				// FIXME : Add Grid Index Default Location Checks here.
				// Remember, if index fails, move to the ground.
				backpack->add_at_random_location( tobuy );
				update_item_to_inrange( tobuy );
				amount_spent += tobuy->sellprice() * num;

				backpack->on_insert_add_item( client->chr, UContainer::MT_CORE_MOVED, tobuy );
			}
			else
			{
				numleft = 0;
				if (fs_item)
					fs_item->add_to_self( tobuy );
				else
					// FIXME : Add Grid Index Default Location Checks here.
					// Remember, if index fails, move to the ground.
					for_sale->add( tobuy );
			}
		}
	}

	client->chr->spend_gold( amount_spent );

	SourcedEvent* sale_event = new SourcedEvent( EVID_MERCHANT_SOLD, client->chr );
	sale_event->addMember( "amount", new BLong(amount_spent) );
	vendor->send_event( sale_event );

	send_clear_vendorwindow( client, vendor );
}

MESSAGE_HANDLER_VARLEN(PKTIN_3B, buyhandler );




bool send_vendorsell( Client* client, NPC* merchant, UContainer* sellfrom, bool send_aos_tooltip )
{
	unsigned short msglen = 0;
	unsigned short num_items = 0;
	PKTOUT_9E::HEADER* header = reinterpret_cast<PKTOUT_9E::HEADER*>(bfr);

	header->msgtype=PKTOUT_9E_ID;
	/* Length is Below */
	header->vendor_serial = merchant->serial_ext;;
	msglen = sizeof(*header);

	UContainer* cont = sellfrom;
	while (cont != NULL)
	{
		for( UContainer::iterator itr = cont->begin(), end=cont->end(); itr != end; ++itr )
		{
			PKTOUT_9E::ELEMENT* element = reinterpret_cast<PKTOUT_9E::ELEMENT*>(&bfr[msglen]);

			Item* item = GET_ITEM_PTR( itr );
			if (item->isa( UObject::CLASS_CONTAINER ))
			{
				UContainer* cont2 = static_cast<UContainer*>(item);
				if (cont2->count())
					continue;
			}
			if (item->newbie())
				continue;
			unsigned long buyprice;
			if (!item->getbuyprice(buyprice))
				continue;
			string desc = item->merchant_description();
			unsigned addlen = sizeof(*element) + desc.size() - 1; // NO null terminator!
			if (msglen + addlen > sizeof bfr)
				return false;

			element->serial = item->serial_ext;
			element->graphic = item->graphic_ext;
			element->color = item->color_ext;
			element->amount = ctBEu16(item->getamount());
			element->price=ctBEu16( static_cast<u16>(buyprice) );
			strcpy(element->description, desc.c_str() );
			element->desc_len=ctBEu16(desc.size());  // NO null this time!
			msglen += static_cast<unsigned short>(addlen); // element contains first byte of desc
			++num_items;

			if (send_aos_tooltip)
				SendAOSTooltip( client, item, true );
		}

		cont = NULL;
	}

	header->msglen	= ctBEu16(msglen);
	header->num_items = ctBEu16(num_items);

	transmit( client, bfr, msglen );
	return true;
}

BObjectImp* UOExecutorModule::mf_SendSellWindow(/* character, vendor, i1, i2, i3, flags */)
{
	Character *chr, *mrchnt;
	NPC* merchant;
	Item* wi1a;
	Item* wi1b;
	Item* wi1c;
	long flags;
	UContainer* merchant_bought;

	if( !(getCharacterParam( exec, 0, chr ) &&
		  getCharacterParam( exec, 1, mrchnt ) &&
		  getItemParam( exec, 2, wi1a ) &&
		  getItemParam( exec, 3, wi1b ) &&
		  getItemParam( exec, 4, wi1c ) &&
		  getParam( 5, flags )) )
	{
		return new BError( "A parameter was invalid" );
	}

	if (!chr->has_active_client())
	{
		return new BError( "No client connected to character" );
	}

	if( mrchnt->isa( UObject::CLASS_NPC ) )
	{
		merchant = static_cast<NPC *>(mrchnt);
	}
	else
	{
		return new BError( "Parameter 1 invalid" );
	}
	if (wi1b->isa( UObject::CLASS_CONTAINER ))
	{
		merchant_bought = static_cast<UContainer*>(wi1b);
	}
	else
	{
		return new BError( "Parameter 2 must be a container" );
	}

	if (chr->backpack() == NULL)
	{
		return new BError( "Character has no backpack" );
	}

	unsigned char save_layer = wi1a->layer;
	wi1a->layer = LAYER_VENDOR_FOR_SALE;
	send_wornitem( chr->client, merchant, wi1a );
	wi1a->layer = save_layer;

	save_layer = wi1b->layer;
	wi1b->layer = LAYER_VENDOR_PLAYER_ITEMS;
	send_wornitem( chr->client, merchant, wi1b );
	wi1b->layer = save_layer;

	save_layer = wi1c->layer;
	wi1c->layer = LAYER_VENDOR_BUYABLE_ITEMS;
	send_wornitem( chr->client, merchant, wi1c );
	wi1c->layer = save_layer;

	bool send_aos_tooltip = flags & VENDOR_SEND_AOS_TOOLTIP ? true : false;

	send_vendorsell( chr->client, merchant, chr->backpack(), send_aos_tooltip );

	chr->client->gd->vendor.set( merchant );
	chr->client->gd->vendor_bought.set( merchant_bought );

	return new BLong(1);
}

extern BObjectImp* _create_item_in_container( UContainer* cont,
											  const ItemDesc* descriptor,
											  unsigned short amount,
											  bool force_stacking,
											  UOExecutorModule* uoemod );
// player selling to vendor
void sellhandler( Client* client, PKTIN_9F* msg)
{
	UContainer* backpack = client->chr->backpack();
	if (backpack == NULL)
		return;

	NPC* vendor = client->gd->vendor.get();
	client->gd->vendor.clear();
	if (vendor == NULL || vendor->orphan() ||
		vendor->serial_ext != msg->vendor_serial)
	{
		return;
	}
	UContainer* vendor_bought = client->gd->vendor_bought.get();
	client->gd->vendor_bought.clear();
	if (vendor_bought == NULL || vendor_bought->orphan())
	{
		return;
	}

	unsigned long cost = 0;
	int num_items = cfBEu16( msg->num_items );
	for( int i = 0; i < num_items; ++i )
	{
		u32 serial = cfBEu32( msg->items[i].serial );
		u16 amount = cfBEu16( msg->items[i].amount );

		unsigned long buyprice;

		Item* item = backpack->find_toplevel( serial );
		if (item == NULL)
			return;
		if (item->newbie())
			continue;
		if (item->inuse())
			continue;
		if (!item->getbuyprice( buyprice ))
			continue;
		if (amount > item->getamount())
			amount = item->getamount();
		Item* remainder_not_sold = NULL;
		if (item->amount_to_remove_is_partial( amount ))
			remainder_not_sold = item->slice_stacked_item( amount );

		if (vendor_bought->can_add( *item ))
		{
			u16 tx, ty;
			vendor_bought->get_random_location( &tx, &ty );
			backpack->remove( item );
			if (remainder_not_sold != NULL)
			{
				// FIXME : Add Grid Index Default Location Checks here.
				// Remember, if index fails, move to the ground.
				backpack->add( remainder_not_sold );
				update_item_to_inrange( remainder_not_sold );
				remainder_not_sold = NULL;
			}
			item->x = tx;
			item->y = ty;
			item->z = 0;
			// FIXME : Add Grid Index Default Location Checks here.
			// Remember, if index fails, move to the ground.
			vendor_bought->add( item );
			update_item_to_inrange( item );
			cost += buyprice * amount;
		}

		if (remainder_not_sold != NULL)
		{
			item->add_to_self( remainder_not_sold );
			update_item_to_inrange( item );
		}
	}

	//dave added 12-19. If no items are sold don't create any gold in the player's pack!
	if(cost > 0)
	{
		//dave added 12-21, create stacks of 60k gold instead of one huge, invalid stack.
		unsigned long temp_cost = cost;
		while(temp_cost > 60000)
		{
			BObject o( _create_item_in_container( backpack, &find_itemdesc(UOBJ_GOLD_COIN), static_cast<unsigned short>(60000), false, NULL ) );
			temp_cost -= 60000;
		}
		if(temp_cost > 0)
		{
			BObject o( _create_item_in_container( backpack, &find_itemdesc(UOBJ_GOLD_COIN), static_cast<unsigned short>(temp_cost), false, NULL ) );
		}
	}
	SourcedEvent* sale_event = new SourcedEvent( EVID_MERCHANT_BOUGHT, client->chr );
	sale_event->addMember( "amount", new BLong(cost) );
	vendor->send_event( sale_event );

	client->pause();

	send_full_statmsg( client, client->chr );

	send_clear_vendorwindow( client, vendor );

	client->restart();
}

MESSAGE_HANDLER_VARLEN(PKTIN_9F, sellhandler );

//
//  "GUMP" Functions
//
BObjectImp* UOExecutorModule::mf_SendGumpMenu( )
{
/*
 Client* client,
					 const char* layout,
					 const char* strings[]
*/
	long x, y, flags;
	Character* chr;
	ObjArray* layout_arr;
	ObjArray* data_arr;
	if (! (getCharacterParam( exec, 0, chr ) &&
		   exec.getObjArrayParam( 1, layout_arr ) &&
		   exec.getObjArrayParam( 2, data_arr ) &&
		   exec.getParam( 3, x ) &&
		   exec.getParam( 4, y ) &&
		   exec.getParam( 5, flags )))
	{
		return new BError( "Invalid parameter" );
	}
	if (!chr->has_active_client())
	{
		return new BError( "No client attached" );
	}
/*
	if (chr->client->gd->gump_uoemod != NULL)
	{
		return new BError( "Client already has an active gump" );
	}
*/
	if ((chr->client->ClientType & CLIENTTYPE_UOKR) || ((!(flags & SENDDIALOGMENU_FORCE_OLD)) && (chr->client->compareVersion(CLIENT_VER_50000))))
		return internal_SendCompressedGumpMenu(chr, layout_arr, data_arr, x, y);
	else
		return internal_SendUnCompressedGumpMenu(chr, layout_arr, data_arr, x, y);
}


BObjectImp* UOExecutorModule::internal_SendUnCompressedGumpMenu(Character* chr, ObjArray* layout_arr, ObjArray* data_arr, long x,long y)
{
	PKTOUT_B0::HEADER* hdr;
	PKTOUT_B0::LAYOUT* playout;
	unsigned playout_idx;
	PKTOUT_B0::DATA_HEADER* pdatahdr;
	unsigned pdatahdr_idx;
	PKTOUT_B0::DATA* pdata;
	int msglen = 0;

	hdr = reinterpret_cast<PKTOUT_B0::HEADER*>(&buffer[0]);

	hdr->msgtype = PKTOUT_B0_ID;
	hdr->serial = chr->serial_ext;
	hdr->dialogid = ctBEu32( this->uoexec.os_module->pid() );
	hdr->x = ctBEu32( x );
	hdr->y = ctBEu32( y );
	msglen = sizeof *hdr;

	playout_idx = sizeof *hdr;
	playout = reinterpret_cast<PKTOUT_B0::LAYOUT*>(&buffer[ msglen ]);
	msglen += offsetof( PKTOUT_B0::LAYOUT, text );
	char* addpt = &playout->text[0];
	int layoutlen = 0;
	for( unsigned i = 0; i < layout_arr->ref_arr.size(); ++i )
	{
		BObject* bo = layout_arr->ref_arr[i].get();
		if (bo == NULL)
			continue;
		BObjectImp* imp = bo->impptr();
		std::string s = imp->getStringRep();

		int addlen = 4 + s.length();
		layoutlen += addlen;
		msglen += addlen;
		if (msglen > static_cast<int>(sizeof buffer))
			return new BError( "Buffer length exceeded" );
		*addpt++ = '{';
		*addpt++ = ' ';
		memcpy( addpt, s.c_str(), s.length() );
		addpt += s.length();
		*addpt++ = ' ';
		*addpt++ = '}';
	}
	++msglen;
	if (msglen > static_cast<int>(sizeof buffer))
		return new BError( "Buffer length exceeded" );
	*addpt++ = '\0';
	++layoutlen;


	playout->len = ctBEu16( layoutlen );

	pdatahdr_idx = msglen;
	pdatahdr = reinterpret_cast<PKTOUT_B0::DATA_HEADER*>(&buffer[ msglen ]);
	int numlines = 0;
	msglen += sizeof *pdatahdr;
	if (msglen > static_cast<int>(sizeof buffer))
		return new BError( "Buffer length exceeded" );

	for( unsigned i = 0; i < data_arr->ref_arr.size(); ++i )
	{
		pdata = reinterpret_cast<PKTOUT_B0::DATA*>(&buffer[ msglen ]);
		BObject* bo = data_arr->ref_arr[i].get();
		if (bo == NULL)
			continue;
		BObjectImp* imp = bo->impptr();
		std::string s = imp->getStringRep();

		const char* string = s.c_str();
		++numlines;
		int textlen = s.length();

		msglen += sizeof *pdata - 1 + textlen * 2;
		if (msglen > static_cast<int>(sizeof buffer))
			return new BError( "Buffer length exceeded" );

		pdata->len = ctBEu16( textlen );
		char* t = &pdata->text[0];
		while (*string)
		{
			*t++ = '\0';
			*t++ = *string;
			++string;
		}
		// msglen += sizeof *pdata - 1 + t - &pdata->text[0];
	}

	pdatahdr->numlines = ctBEu16( numlines );
	hdr->msglen = ctBEu16( msglen );
	chr->client->transmit( buffer, msglen );
	chr->client->gd->add_gumpmod( this );
	//old_gump_uoemod = this;
	gump_chr = chr;
	uoexec.os_module->suspend();
	return new BLong(0);
}

BObjectImp* UOExecutorModule::internal_SendCompressedGumpMenu(Character* chr, ObjArray* layout_arr, ObjArray* data_arr, long x,long y)
{
	PKTOUT_DD::HEADER *header = (PKTOUT_DD::HEADER *)buffer;
	header->dialog_x = x;
	header->dialog_y = y;
	header->msgtype = PKTOUT_DD_ID;
	header->serial = chr->serial_ext;
	header->dialog_id = ctBEu32( this->uoexec.os_module->pid() );
	header->dialog_x = ctBEu32( header->dialog_x );
	header->dialog_y = ctBEu32( header->dialog_y );

	PKTOUT_DD::LAYOUT *playout = (PKTOUT_DD::LAYOUT *)((u32)header+sizeof(PKTOUT_DD::HEADER));
	unsigned char *tempbufpos = bfr;
	playout->layout_dlen = 0;

	for( unsigned i = 0; i < layout_arr->ref_arr.size(); ++i )
	{
		BObject* bo = layout_arr->ref_arr[i].get();
		if (bo == NULL)
			continue;
		BObjectImp* imp = bo->impptr();
		std::string s = imp->getStringRep();

		playout->layout_dlen += 4 + s.length();
		if (playout->layout_dlen > sizeof(bfr))
			return new BError( "Buffer length exceeded" );
		*tempbufpos++ = '{';
		*tempbufpos++ = ' ';
		memcpy( tempbufpos, s.c_str(), s.length() );
		tempbufpos += s.length();
		*tempbufpos++ = ' ';
		*tempbufpos++ = '}';
	}
	playout->layout_dlen++;
	if (playout->layout_dlen > sizeof(bfr))
		return new BError( "Buffer length exceeded" );
	*tempbufpos++ = '\0';


	playout->layout_clen = sizeof(bfr)*2;
	if (compress(&playout->layout_cdata, &playout->layout_clen, bfr, playout->layout_dlen))   
		return new BError( "Compression error" );
	playout->layout_dlen = ctBEu32( playout->layout_dlen );


	PKTOUT_DD::TEXT *ptext = (PKTOUT_DD::TEXT *)((u32)&playout->layout_cdata + playout->layout_clen);
	playout->layout_clen = ctBEu32( playout->layout_clen+4 );

	ptext->lines = 0;
	ptext->text_dlen = 0;
	u16 *tempwbufpos = (u16*)bfr;

	for( unsigned i = 0; i < data_arr->ref_arr.size(); ++i )
	{
		BObject* bo = data_arr->ref_arr[i].get();
		if (bo == NULL)
			continue;
		ptext->lines++;

		BObjectImp* imp = bo->impptr();
		std::string s = imp->getStringRep();
		const char* string = s.c_str();
		ptext->text_dlen += (s.length()+1)*2;

		if (ptext->text_dlen > sizeof(bfr))
			return new BError( "Buffer length exceeded" );

		*tempwbufpos++ = ctBEu16((u16)s.length());
		while (*string)
			*tempwbufpos++ = (u16)(*string++) << 8;
	}

	ptext->lines = ctBEu32(ptext->lines);

	ptext->text_clen = sizeof(bfr)*2;
	if (compress(&ptext->text_cdata, &ptext->text_clen, bfr, ptext->text_dlen))   
		return new BError( "Compression error" );

	ptext->text_dlen = ctBEu32(ptext->text_dlen);
	u16 msglen = (u16)((u32)&ptext->text_cdata - (u32)buffer + ptext->text_clen+1);
	header->msglen = ctBEu16( msglen );
	ptext->text_clen = ctBEu32(ptext->text_clen+4);

	chr->client->transmit( buffer, msglen );
	chr->client->gd->add_gumpmod( this );
	//old_gump_uoemod = this;
	gump_chr = chr;
	uoexec.os_module->suspend();
	return new BLong(0);
}

class BIntHash : public BObjectImp
{
public:
	BIntHash();
	BIntHash( const BIntHash& );
	void add( long key, BObjectImp* value );
	virtual BObjectRef get_member( const char* membername );
	virtual BObjectRef OperSubscript( const BObject& obj );
	virtual BObjectImp* copy() const;
	virtual std::string getStringRep() const;
	virtual unsigned long sizeEstimate() const;
private:
	typedef std::map<long,BObjectRef> Contents;
	Contents contents_;

	// not implemented:
	BIntHash& operator=( const BIntHash& );
};

BIntHash::BIntHash() :
	BObjectImp( OTUnknown )
{
}

BIntHash::BIntHash( const BIntHash& ih ) :
	BObjectImp( OTUnknown ),
	contents_( ih.contents_ )
{
}

BObjectImp* BIntHash::copy() const
{
	return new BIntHash( *this );
}

unsigned long BIntHash::sizeEstimate() const
{
	unsigned long size = sizeof(BIntHash);
	Contents::const_iterator itr, end;
	for( itr = contents_.begin(), end = contents_.end(); itr != end; ++itr )
	{
		size += sizeof(long) + (*itr).second.sizeEstimate();
	}
	return size;
}

string BIntHash::getStringRep() const
{
	return "<inthash>";
}

void BIntHash::add( long key, BObjectImp* value )
{
	// contents_.insert( Contents::value_type( key, value ) );
	contents_[key].set( new BObject(value) );
}

BObjectRef BIntHash::get_member( const char* membername )
{
	if (stricmp( membername, "keys") == 0)
	{
		ObjArray* arr = new ObjArray;
		BObject obj( arr );

		Contents::const_iterator itr, end;
		for( itr = contents_.begin(), end = contents_.end(); itr != end; ++itr )
		{
			long key = (*itr).first;
			arr->addElement( new BLong( key ) );
		}

		return BObjectRef( obj.impptr() );
	}
	else
	{
		return BObjectRef( new BError( "member not found" ) );
	}
}

BObjectRef BIntHash::OperSubscript( const BObject& obj )
{
	const BObjectImp& objimp = obj.impref();
	if (objimp.isa( OTLong ))
	{
		const BLong& lng = static_cast<const BLong&>(objimp);
		Contents::iterator itr = contents_.find( lng.value() );
		if (itr != contents_.end())
		{
			BObjectRef& oref = (*itr).second;
			return BObjectRef( oref.get()->impptr() );
		}
		else
		{
			return BObjectRef( new BError( "Key not found in inthash" ) );
		}
	}
	else
	{
		return BObjectRef( new BError( "Incorrect type used as subscript to inthash" ) );
	}
}

void clear_gumphandler( Client* client, UOExecutorModule* uoemod )
{
	uoemod->uoexec.os_module->revive();
	uoemod->gump_chr = NULL;
	client->gd->remove_gumpmod( uoemod );
}

BObjectImp* UOExecutorModule::mf_CloseGump(/* who, pid, response := 0 */)
{	
	Character *chr;
	unsigned int pid;
	BObjectImp* resp;

	if (! (getCharacterParam( exec, 0, chr ) &&
		   exec.getParam( 1, pid ) &&
		   (resp = exec.getParamImp( 2 ))))
	{
		return new BError( "Invalid parameter" );
	}

	if ( !chr->has_active_client() )
		return new BError( "No client attached" );

	Client* client = chr->client;

	UOExecutorModule* uoemod = client->gd->find_gumpmod( pid );
	if (uoemod == NULL)
	{
		return new BError( "Couldnt find script" );
	}

	PKTBI_BF msg;

	msg.msgtype = PKTBI_BF_ID;
	msg.subcmd = ctBEu16(PKTBI_BF::TYPE_CLOSE_GUMP);
	msg.closegump.buttonid = ctBEu32(0);
	msg.closegump.gumpid = ctBEu32(pid);
	msg.msglen = ctBEu16(13);

	client->transmit(&msg, 13);

	uoemod->uoexec.ValueStack.top().set( new BObject(resp) );
	clear_gumphandler( client, uoemod );

	return new BLong( 1 );
}

void gumpbutton_handler( Client* client, PKTIN_B1* msg )
{
	char* msgbuf = reinterpret_cast<char*>(msg);
	PKTIN_B1::HEADER* hdr = reinterpret_cast<PKTIN_B1::HEADER*>(msg);
	unsigned short msglen = cfBEu16( hdr->msglen );

	u32 pid = cfBEu32( hdr->dialogid );
	
	if ( pid == 0x1CD ) //virtue Pentagram doubleclicked
	{
		if ( hdr->serial == client->chr->serial_ext )
		{
			if ( cfBEu32(hdr->gumpid) == 1 )
			{
				PKTIN_B1::INTS_HEADER* intshdr_ = reinterpret_cast<PKTIN_B1::INTS_HEADER*>(hdr+1);
				if ( cfBEu32(intshdr_->count) == 1 )
				{
					PKTIN_B1::INT_ENTRY* intentries_ = reinterpret_cast<PKTIN_B1::INT_ENTRY*>(intshdr_+1);
					if ( intentries_->value == client->chr->serial_ext )
					{
						ref_ptr<EScriptProgram> prog = find_script( "misc/virtuebutton", 
                                                true, 
                                                config.cache_interactive_scripts );
						if (prog.get() != NULL)
							client->chr->start_script( prog.get(), false );
						return;
					}
				}
			}
		}
	}


	UOExecutorModule* uoemod = client->gd->find_gumpmod( pid );
	if (uoemod == NULL)
	{
	//	cerr << "Client (Account " << client->acct->name() << ", Character " << client->chr->name() << ") used out-of-sequence gump command?" << std::endl;
		return;
	}

	// printf( "PKTIN_B1: %ld bytes\n", cfBEu16(msg->msglen) );
	// fdump( stdout, msg, cfBEu16(msg->msglen) );

	PKTIN_B1::INTS_HEADER* intshdr = reinterpret_cast<PKTIN_B1::INTS_HEADER*>(hdr+1);
	u32 ints_count = cfBEu32( intshdr->count );
	unsigned stridx = sizeof(PKTIN_B1::HEADER) +
					  sizeof(PKTIN_B1::INTS_HEADER) +
					  sizeof(PKTIN_B1::INT_ENTRY) * ints_count +
					  sizeof(PKTIN_B1::STRINGS_HEADER);
	if ( stridx > msglen )
	{
		cerr << "Blech! B1 message specified too many ints!" << endl;
		clear_gumphandler( client, uoemod );
		return;
	}
	PKTIN_B1::INT_ENTRY* intentries = reinterpret_cast<PKTIN_B1::INT_ENTRY*>(intshdr+1);
	PKTIN_B1::STRINGS_HEADER* strhdr = reinterpret_cast<PKTIN_B1::STRINGS_HEADER*>(intentries+ints_count);
	u32 strings_count = cfBEu32( strhdr->count );
	// even if this is ok, it could still overflow.  Have to check each string.
	if (stridx + (sizeof(PKTIN_B1::STRING_ENTRY)-1) * strings_count > msglen+1u)
	{
		cerr << "Client (Account " << client->acct->name() << ", Character " << client->chr->name() << ") Blech! B1 message specified too many ints and/or strings!" << endl;
		uoemod->uoexec.ValueStack.top().set(new BObject(new BError("B1 message specified too many ints and/or strings.")));
		clear_gumphandler( client, uoemod );
		return;
	}


	BObjectImp* resimp = NULL;
	if (ints_count == 0 && strings_count == 0 && hdr->gumpid == 0)
	{
		resimp = new BLong(0);
	}
	else
	{
		BIntHash* hash = new BIntHash;
		hash->add( 0, new BLong( cfBEu32( hdr->gumpid ) ) );
		hash->add( cfBEu32( hdr->gumpid ), new BLong(1) );
		for( unsigned i = 0; i < ints_count; ++i )
		{
			hash->add( cfBEu32( intentries[i].value ), new BLong(1) );
		}
		for( unsigned i = 0; i < strings_count; ++i )
		{
			PKTIN_B1::STRING_ENTRY* strentry = reinterpret_cast<PKTIN_B1::STRING_ENTRY*>(msgbuf + stridx);
			unsigned short length = cfBEu16( strentry->length );
			stridx += offsetof( PKTIN_B1::STRING_ENTRY, data ) + length * 2;
			if (stridx > msglen)
			{
				cerr << "Client (Account " << client->acct->name() << ", Character " << client->chr->name() << ") Blech! B1 message strings overflow message buffer!" << endl;
				break;
			}
			string str;
			str = decint( cfBEu16(strentry->tag) ) + ": ";
			str.reserve( length + str.size() );
			u8 c;
			for( int si = 0; si < length; ++si ) //ENHANCE: Handle Unicode strings properly (add a "uc" member somewhere for each returned string that doesn't break existing code)
			{
				c = strentry->data[si*2+1];
				if(c >= 0x20) //dave added 4/13/3, strip control characters
					str.append( 1, c );
			}
			// oops we're throwing away tag!
			hash->add( cfBEu16( strentry->tag ), new String( str ) );
		}
		resimp = hash;
	}
	uoemod->uoexec.ValueStack.top().set( new BObject( resimp ) );
	clear_gumphandler( client, uoemod );
}
MESSAGE_HANDLER_VARLEN(PKTIN_B1, gumpbutton_handler );

BObjectImp* UOExecutorModule::mf_SendTextEntryGump()
{
	Character* chr;
	const String* line1;
	long cancel;
	long style;
	long maximum;
	const String* line2;

	if (! (getCharacterParam( exec, 0, chr ) &&
		   exec.getStringParam( 1, line1 ) &&
		   exec.getParam( 2, cancel ) &&
		   exec.getParam( 3, style ) &&
		   exec.getParam( 4, maximum ) &&
		   exec.getStringParam( 5, line2 )) )
	{
		return new BError( "Invalid parameter" );
	}
	if (!chr->has_active_client())
	{
		return new BError( "No client attached" );
	}
	static char buffer[ 8000 ];
	// FIXME buffer overflow
	unsigned short msglen;

	PKTOUT_AB::HEADER* phdr;
	PKTOUT_AB::TEXTLINE1* ptextline1;
	PKTOUT_AB::TEXTLINE2* ptextline2;

	phdr = reinterpret_cast<PKTOUT_AB::HEADER*>(&buffer[0]);
	phdr->msgtype = PKTOUT_AB_ID;
	phdr->serial = chr->serial_ext;
	phdr->type = 0;
	phdr->index = 0;
	msglen = sizeof(*phdr);

	unsigned int numbytes;

	ptextline1 = reinterpret_cast<PKTOUT_AB::TEXTLINE1*>(&buffer[msglen]);
	numbytes = line1->length()+1;
	msglen += static_cast<unsigned short>(offsetof( PKTOUT_AB::TEXTLINE1,text ) + numbytes);
	if (numbytes > sizeof(ptextline1->text))
		numbytes = sizeof(ptextline1->text);
	ptextline1->numbytes = ctBEu16( numbytes );
	memcpy( ptextline1->text, line1->data(), numbytes );

	ptextline2 = reinterpret_cast<PKTOUT_AB::TEXTLINE2*>(&buffer[msglen]);
	numbytes = line2->length() + 1;
	if (numbytes > sizeof(ptextline2->text))
		numbytes = sizeof(ptextline2->text);
	ptextline2->cancel = static_cast<u8>(cancel);  // PKTOUT_AB::TEXTLINE2::CANCEL_DISABLE;
	ptextline2->style = static_cast<u8>(style); // PKTOUT_AB::TEXTLINE2::STYLE_NORMAL;
	ptextline2->mask = ctBEu32(maximum);
	ptextline2->numbytes = ctBEu16( numbytes );
	memcpy( ptextline2->text, line2->data(), numbytes );
	msglen += static_cast<unsigned short>(offsetof( PKTOUT_AB::TEXTLINE2,text) + numbytes);

	phdr->msglen = ctBEu16( msglen );

	chr->client->transmit( buffer, msglen );
	chr->client->gd->textentry_uoemod = this;
	textentry_chr = chr;
	uoexec.os_module->suspend();
	return new BLong(0);
}

void handle_textentry( Client* client, PKTIN_AC* msg )
{
	if (client->gd->textentry_uoemod == NULL)
	{
		cerr << "Client (Account " << client->chr->acct->name() << ", Character " << client->chr->name() << ")used out-of-sequence textentry command?" << endl;
		return;
	}
	BObjectImp* resimp = new BLong(0);
	if (msg->retcode == PKTIN_AC::RETCODE_OKAY)
	{
		unsigned short datalen = cfBEu16( msg->datalen );
		if (datalen >= 1 && datalen <= 256 &&
			msg->data[ datalen-1 ] == '\0')
		{
			//dave added isprint checking 4/13/3
			bool ok = true;
			--datalen; // don't include null terminator (already checked)
			for( int i = 0; i < datalen; ++i )
			{
				if (!isprint(msg->data[i]))
				{
					ok = false;
					break;
				}
			}
			if (ok)
			{
				resimp = new String( msg->data, datalen );
			}
		}

	}

	client->gd->textentry_uoemod->uoexec.ValueStack.top().set( new BObject( resimp ) );
	client->gd->textentry_uoemod->uoexec.os_module->revive();
	client->gd->textentry_uoemod->textentry_chr = NULL;
	client->gd->textentry_uoemod = NULL;

}
MESSAGE_HANDLER_VARLEN( PKTIN_AC, handle_textentry );




class PolCore : public BObjectImp
{
public:
	PolCore();
	virtual BObjectRef get_member( const char* membername );
	virtual BObjectImp* call_method( const char* methodname, Executor& ex );
	virtual BObjectImp* copy() const;
	virtual std::string getStringRep() const;
	virtual unsigned long sizeEstimate() const { return sizeof(PolCore); }
private:
	// not implemented:
	PolCore& operator=( const PolCore& );
};

PolCore::PolCore() : BObjectImp( OTUnknown )
{
}

BObjectImp* PolCore::copy() const
{
	return new PolCore;
}

string PolCore::getStringRep() const
{
	return "<polcore>";
}

BObjectImp* GetPackageList()
{
	ObjArray* arr = new ObjArray;
	for( Packages::iterator itr = packages.begin(); itr != packages.end(); ++itr )
	{
		Package* pkg = (*itr);
		arr->addElement( new String( pkg->name() ) );
	}
	return arr;
}

void add_script( ObjArray* arr, UOExecutor* uoexec, const char* state )
{
	arr->addElement( new ScriptExObjImp( uoexec ) );
}

BObjectImp* GetRunningScriptList()
{
	ObjArray* arr = new ObjArray;
	for( ExecList::iterator itr = ranlist.begin(); itr != ranlist.end(); ++itr )
	{
		add_script( arr, *itr, "Running" );
	}
	for( ExecList::iterator itr = runlist.begin(); itr != runlist.end(); ++itr )
	{
		add_script( arr, *itr, "Running" );
	}
	return arr;
}

BObjectImp* GetAllScriptList()
{
	ObjArray* arr = new ObjArray;
	for( ExecList::iterator itr = ranlist.begin(); itr != ranlist.end(); ++itr )
	{
		add_script( arr, *itr, "Running" );
	}
	for( ExecList::iterator itr = runlist.begin(); itr != runlist.end(); ++itr )
	{
		add_script( arr, *itr, "Running" );
	}
	for( HoldList::iterator itr = holdlist.begin(); itr != holdlist.end(); ++itr )
	{
		add_script( arr, (*itr).second, "Sleeping" );
	}
	for( NoTimeoutHoldList::iterator itr = notimeoutholdlist.begin(); itr != notimeoutholdlist.end(); ++itr )
	{
		add_script( arr, *itr, "Sleeping" );
	}
	return arr;
}

BObjectImp* GetScriptProfiles( )
{
	ObjArray* arr = new ObjArray;

	ScriptStorage::iterator itr = scrstore.begin(), end=scrstore.end();
	u64 total_instr = 0;
	for(; itr != end; ++itr )
	{
		EScriptProgram* eprog = ((*itr).second).get();
		total_instr += eprog->instr_cycles;
	}

	itr = scrstore.begin();
	end=scrstore.end();

	for(; itr != end; ++itr )
	{
		EScriptProgram* eprog = ((*itr).second).get();


		BStruct* elem = new BStruct;
		elem->addMember( "name", new String( eprog->name ) );
		elem->addMember( "instr", new Double( eprog->instr_cycles ) );
		elem->addMember( "invocations", new BLong( eprog->invocations ) );
		u64 cycles_per_invoc = eprog->instr_cycles / (eprog->invocations?eprog->invocations:1);
		elem->addMember( "instr_per_invoc", new Double( cycles_per_invoc ) );
		double cycle_percent = static_cast<double>(eprog->instr_cycles) / total_instr * 100.0;
		elem->addMember( "instr_percent", new Double( cycle_percent ) );

		arr->addElement( elem );

	}
	return arr;
}

BObjectImp* GetIoStatsObj( const IOStats& stats )
{
	BStruct* arr = new BStruct;

	ObjArray* sent = new ObjArray;
	arr->addMember( "sent", sent );

	ObjArray* received = new ObjArray;
	arr->addMember( "received", received );

	for(unsigned i = 0; i < 256; ++i)
	{
		BStruct* elem = new BStruct;
		elem->addMember( "count", new BLong( stats.sent[i].count ) );
		elem->addMember( "bytes", new BLong( stats.sent[i].bytes ) );
		sent->addElement( elem );
	}

	for(unsigned i = 0; i < 256; ++i)
	{
		BStruct* elem = new BStruct;
		elem->addMember( "count", new BLong( stats.received[i].count ) );
		elem->addMember( "bytes", new BLong( stats.received[i].bytes ) );
		received->addElement( elem );
	}

	return arr;
}

BObjectImp* GetIoStats()
{
	return GetIoStatsObj( iostats );
}

BObjectImp* GetQueuedIoStats()
{
	return GetIoStatsObj( queuedmode_iostats );
}

BObjectImp* GetCoreVariable( const char* corevar )
{
#define LONG_COREVAR(name,expr) if (stricmp( corevar, #name ) == 0) return new BLong( expr );

	if (stricmp( corevar, "itemcount" ) == 0) return new BLong(get_toplevel_item_count());
	if (stricmp( corevar, "mobilecount" ) == 0) return new BLong(get_mobile_count());

	if (stricmp( corevar, "bytes_sent" ) == 0) return new Double(polstats.bytes_sent);
	if (stricmp( corevar, "bytes_received" ) == 0) return new Double(polstats.bytes_received);

	LONG_COREVAR( uptime, polclock() / POLCLOCKS_PER_SEC );
	LONG_COREVAR( sysload, last_sysload );
	LONG_COREVAR( sysload_severity, last_sysload_nprocs );
//	LONG_COREVAR( bytes_sent, polstats.bytes_sent );
//	LONG_COREVAR( bytes_received, polstats.bytes_received );
	LONG_COREVAR( version, polver );
	LONG_COREVAR( systime, time(NULL) );
	LONG_COREVAR( events_per_min, GET_PROFILEVAR_PER_MIN(events) );
	LONG_COREVAR( skill_checks_per_min, GET_PROFILEVAR_PER_MIN(skill_checks) );
	LONG_COREVAR( combat_operations_per_min, GET_PROFILEVAR_PER_MIN(combat_operations) );
	LONG_COREVAR( error_creations_per_min, GET_PROFILEVAR_PER_MIN(error_creations) );

	LONG_COREVAR( tasks_ontime_per_min, GET_PROFILEVAR_PER_MIN(tasks_ontime) );
	LONG_COREVAR( tasks_late_per_min, GET_PROFILEVAR_PER_MIN(tasks_late) );
	LONG_COREVAR( tasks_late_ticks_per_min, GET_PROFILEVAR_PER_MIN(tasks_late_ticks) );

	LONG_COREVAR( scripts_late_per_min, GET_PROFILEVAR_PER_MIN(scripts_late) );
	LONG_COREVAR( scripts_ontime_per_min, GET_PROFILEVAR_PER_MIN(scripts_ontime) );

	LONG_COREVAR( instr_per_min, last_sipm );
	LONG_COREVAR( priority_divide, priority_divide );
	if (stricmp( corevar, "verstr" ) == 0) return new String( progverstr );
	if (stricmp( corevar, "compiledate" ) == 0) return new String( compiledate );
	if (stricmp( corevar, "compiletime" ) == 0) return new String( compiletime );
	if (stricmp( corevar, "packages" ) == 0) return GetPackageList();
	if (stricmp( corevar, "running_scripts" ) == 0) return GetRunningScriptList();
	if (stricmp( corevar, "all_scripts" ) == 0) return GetAllScriptList();
	if (stricmp( corevar, "script_profiles" ) == 0) return GetScriptProfiles();
	if (stricmp( corevar, "iostats" ) == 0) return GetIoStats();
	if (stricmp( corevar, "queued_iostats" ) == 0) return GetQueuedIoStats();

	return new BError( string("Unknown core variable ") + corevar );
}

BObjectRef PolCore::get_member( const char* membername )
{
	return BObjectRef( GetCoreVariable( membername ) );
}

BObjectImp* PolCore::call_method( const char* methodname, Executor& ex )
{
	if (stricmp( methodname, "log_profile" ) == 0)
	{
		if (ex.numParams()!=1)
			return new BError( "polcore.log_profile(clear) requires 1 parameter." );
		long clear;
		if (ex.getParam( 0, clear ))
		{
			log_all_script_cycle_counts( clear ? true : false );
			return new BLong(1);

		}
	}
	else if (stricmp( methodname, "set_priority_divide" ) == 0)
	{
		if (ex.numParams()!=1)
			return new BError( "polcore.set_priority_divide(value) requires 1 parameter." );
		long div;
		if (ex.getParam( 0, div, 1, 1000 ))
		{
			priority_divide = div;
			return new BLong(1);
		}
		else
		{
			return NULL;
		}
	}
	else if (stricmp( methodname, "clear_script_profile_counters" ) == 0)
	{
		if (ex.numParams() > 0)
			return new BError( "polcore.clear_script_profile_counters() doesn't take parameters." );
		clear_script_profile_counters();
		return new BLong(1);
	}
	else if (stricmp( methodname, "internal" ) == 0) // Just for internal Development...
	{
		long type;
		if (ex.getParam( 0, type ))
		{
#ifdef MEMORYLEAK
			if (type == 1)
			{
				char buffer[30];
				time_t now = time(NULL);
				struct tm* tm_now = localtime( &now );

				strftime( buffer, sizeof buffer, "%m/%d %H:%M:%S", tm_now );
				if (mlog.is_open())
					mlog << "[" << buffer << "] polcore().internal" << endl;
				if (llog.is_open())
					llog << buffer << ";";

				bobject_alloc.log_stuff("bobject");
				uninit_alloc.log_stuff("uninit");
				blong_alloc.log_stuff("blong");
				double_alloc.log_stuff("double");
				ConfigFiles_log_stuff();
				PrintHeapData(); // Will print endl in llog
			}
#endif
			return new BLong(1);
		}
		else
			return new BError( "polcore.internal(value) requires 1 parameter." );
	}
	return NULL;
}

BObjectImp* UOExecutorModule::mf_PolCore()
{
	return new PolCore;
}

BObjectImp* MakeMapCacheImp( )
{
	ObjArray* arr = new ObjArray;
	return arr;
}

#include "../accounts/account.h"
#include "../accounts/accounts.h"
#include "../accounts/acscrobj.h"
BObjectImp* UOExecutorModule::mf_CreateAccount()
{
	const String* acctname;
	const String* password;
	long enabled;
	if (getStringParam( 0, acctname ) &&
		getStringParam( 1, password ) &&
		getParam( 2, enabled ))
	{
		if ( acctname->SafeCharAmt() < acctname->length() ) 
		{
			return new BError("Attempted to use username in account creation with non-allowed characters.");	
		}
		if ( password->SafeCharAmt() < password->length() )
		{
			return new BError("Attempted to use password in account creation with non-allowed characters.");	
		}

		if (find_account( acctname->data() ))
		{
			return new BError( "Account already exists" );
		}

		//Dave 6/5/3 let this function handle the hashing (Account ctor does it)
		Account* acct = create_new_account( acctname->value(),password->value(), enabled?true:false ); //MD5

		return new AccountObjImp( AccountPtrHolder(AccountRef(acct)) );
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

BObjectImp* UOExecutorModule::mf_FindAccount()
{
	const String* acctname;
	if (getStringParam( 0, acctname ))
	{
		Account* acct = find_account( acctname->data() );
		if (acct != NULL)
		{
			return new AccountObjImp( AccountPtrHolder(AccountRef(acct)) );
		}
		else
		{
			return new BError( "Account not found." );
		}
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

BObjectImp* UOExecutorModule::mf_ListAccounts()
{
	ObjArray* arr = new ObjArray;
  	for( unsigned idx = 0; idx < accounts.size(); idx++ )
	{
		arr->addElement( new String( accounts[idx]->name() ) );
	}
	return arr;
}

void handle_resurrect_menu( Client *client, PKTBI_2C *msg )
{
	if (msg->choice)
	{
		cout << "Resurrect Menu Choice: " << int(msg->choice) << endl;
		// transmit( client, msg, sizeof *msg );
	}

	if (client->chr != NULL &&
		client->gd != NULL &&
		client->gd->resurrect_uoemod != NULL)
	{
		client->gd->resurrect_uoemod->uoexec.ValueStack.top().set( new BObject( new BLong( msg->choice ) ) );
		client->gd->resurrect_uoemod->uoexec.os_module->revive();
		client->gd->resurrect_uoemod->resurrect_chr = NULL;
		client->gd->resurrect_uoemod = NULL;
	}
}
MESSAGE_HANDLER( PKTBI_2C, handle_resurrect_menu );

BObjectImp* UOExecutorModule::mf_SendInstaResDialog()
{
	Character* chr;
	if (!getCharacterParam( exec, 0, chr ))
		return new BError( "Invalid parameter type" );
	if (!chr->has_active_client())
		return new BError( "No client attached" );
	if (chr->client->gd->resurrect_uoemod != NULL)
		return new BError( "Client busy with another instares dialog" );

	PKTBI_2C msg;
	msg.msgtype = PKTBI_2C_ID;
	msg.choice = RESURRECT_CHOICE_SELECT;

	chr->client->transmit( &msg, sizeof msg );
	chr->client->gd->resurrect_uoemod = this;
	resurrect_chr = chr;
	uoexec.os_module->suspend();
	return new BLong(0);
}

void handle_selcolor( Client* client, PKTBI_95* msg )
{
	if (client->chr != NULL &&
		client->gd != NULL &&
		client->gd->selcolor_uoemod != NULL)
	{
		unsigned short color = cfBEu16(msg->graphic_or_color) & VALID_ITEM_COLOR_MASK;
		BObject *valstack;
		if ( color >= 2 && color <= 1001 )
		{
			valstack = new BObject( new BLong( color ) );
		}
		else
		{
			valstack = new BObject( new BError( "Client selected an out-of-range color" ) );

			//unsigned short newcolor = ((color - 2) % 1000) + 2;
			sprintf(buffer, "Client #%lu (account %s) selected an out-of-range color 0x%x",
							client->instance_,
							(client->acct != NULL) ? client->acct->name() : "unknown",
							color);
			cerr << buffer << endl;
			Log("%s\n", buffer);
		}

		//client->gd->selcolor_uoemod->uoexec.ValueStack.top().set( new BObject( new BLong( color ) ) );
		client->gd->selcolor_uoemod->uoexec.ValueStack.top().set( valstack );
		client->gd->selcolor_uoemod->uoexec.os_module->revive();
		client->gd->selcolor_uoemod->selcolor_chr = NULL;
		client->gd->selcolor_uoemod = NULL;
	}
}
MESSAGE_HANDLER( PKTBI_95, handle_selcolor );

BObjectImp* UOExecutorModule::mf_SelectColor()
{
	Character* chr;
	Item* item;
	if (!getCharacterParam( exec, 0, chr ) ||
		!getItemParam( exec, 1, item ))
	{
		return new BError( "Invalid parameter type" );
	}
	if (!chr->has_active_client())
		return new BError( "No client attached" );
	if (chr->client->gd->resurrect_uoemod != NULL)
		return new BError( "Client is already selecting a color" );

	PKTBI_95 msg;
	msg.msgtype = PKTBI_95_ID;
	msg.serial = item->serial_ext;
	msg.unk = 0;
	msg.graphic_or_color = item->graphic_ext;

	chr->client->transmit( &msg, sizeof msg );

	chr->client->gd->selcolor_uoemod = this;
	selcolor_chr = chr;
	uoexec.os_module->suspend();
	return new BLong(0);
}

BObjectImp* UOExecutorModule::mf_SendOpenBook()
{
	Character* chr;
	Item* book;

	if (! (getCharacterParam( exec, 0, chr ) &&
		   getItemParam( exec, 1, book )) )
	{
		return new BError( "Invalid parameter type" );
	}
	if (!chr->has_active_client())
	{
		return new BError( "No active client" );
	}

	bool writable = book->call_custom_method( "iswritable" )->isTrue();
	BObject nlines_ob( book->call_custom_method( "getnumlines" ) );
	long nlines;
	if (nlines_ob.isa( BObjectImp::OTLong ))
	{
		BLong* blong = static_cast<BLong*>(nlines_ob.impptr());
		nlines = blong->value();
	}
	else
	{
		return new BError( "book.GetNumLines() did not return an Integer" );
	}
	string title = book->call_custom_method( "gettitle" )->getStringRep();
	string author = book->call_custom_method( "getauthor" )->getStringRep();

	long npages = (nlines+7)/8;

	BObject contents_ob( UninitObject::create() );
	if (writable)
	{
		contents_ob.setimp( book->call_custom_method( "getcontents" ).impptr() );
		if (!contents_ob.isa( BObjectImp::OTArray ))
		{
			if (contents_ob.isa( BObjectImp::OTError ))
				return contents_ob->copy();
			else
				return new BError( "book.GetContents() must return an array" );
		}
	}

	PKTBI_93 msg;
	memset( &msg, 0, sizeof msg );
	msg.msgtype = PKTBI_93_ID;
	msg.serial = book->serial_ext;
	msg.writable = writable?1:0;
	msg.unk_1 = 1;
	msg.npages = ctBEu16( static_cast<u16>(npages) );

	strzcpy( msg.title, title.c_str(), sizeof msg.title );
	strzcpy( msg.author, author.c_str(), sizeof msg.author );

	chr->client->transmit( &msg, sizeof msg );

	if (writable)
	{
		//PolTimer timer;
		unsigned msglen = 0;
		PKTBI_66_HDR* phdr = reinterpret_cast<PKTBI_66_HDR*>(buffer);
		phdr->msgtype = PKTBI_66_ID;
		phdr->msglen = 0;
		phdr->book_serial = book->serial_ext;
		phdr->pages = ctBEu16( static_cast<u16>(npages) );
		msglen = sizeof(*phdr);

		ObjArray* arr = static_cast<ObjArray*>(contents_ob.impptr());

		long linenum = 1;
		for( int page = 1; page <= npages; ++page )
		{
			PKTBI_66_CONTENTS* ppage = reinterpret_cast<PKTBI_66_CONTENTS*>(&buffer[msglen]);
			msglen += sizeof(*ppage);
			if (msglen > sizeof buffer)
				return new BError( "Buffer overflow" );
			ppage->page = ctBEu16( page );

			int pagelines;
			for( pagelines = 0; pagelines < 8 && linenum <= nlines; ++pagelines, ++linenum )
			{
				const BObjectImp* line_imp = arr->imp_at( linenum );
				string linetext;
				if (line_imp)
					linetext = line_imp->getStringRep();

				char* linebuf = reinterpret_cast<char*>(&buffer[msglen]);
				msglen += linetext.size()+1;
				if (msglen > sizeof buffer)
					return new BError( "Buffer overflow" );
				memcpy( linebuf, linetext.c_str(), linetext.size()+1 );
			}
			ppage->lines = ctBEu16( pagelines );
		}

/*
		long linenum = 1;
		for( int page = 1; page <= npages; ++page )
		{
			PKTBI_66_CONTENTS* ppage = reinterpret_cast<PKTBI_66_CONTENTS*>(&buffer[msglen]);
			msglen += sizeof(*ppage);
			if (msglen > sizeof buffer)
				return new BError( "Buffer overflow" );
			ppage->page = ctBEu16( page );

			int pagelines;
			for( pagelines = 0; pagelines < 8 && linenum <= nlines; ++pagelines, ++linenum )
			{
				string linetext;

				BObjectVec params;
				params.push_back( BObject(new BLong(linenum)) );
				BObject line_ob = book->call_custom_method( "getline", params );
				linetext = line_ob->getStringRep();

				char* linebuf = reinterpret_cast<char*>(&buffer[msglen]);
				msglen += linetext.size()+1;
				if (msglen > sizeof buffer)
					return new BError( "Buffer overflow" );
				memcpy( linebuf, linetext.c_str(), linetext.size()+1 );
			}
			ppage->lines = ctBEu16( pagelines );
		}
*/

		phdr->msglen = ctBEu16( msglen );
		//timer.printOn( cout );
		chr->client->transmit( buffer, msglen );
	}

	return new BLong(1);
}

void read_book_page_handler( Client* client, PKTBI_66* msg )
{
	unsigned long book_serial = cfBEu32( msg->book_serial );
   	u16 page = cfBEu16(msg->page);
	Item *book = find_legal_item( client->chr, book_serial );
	if (book == NULL)
	{
		Log( "Unable to find book 0x%lx for character 0x%lx\n",
			  book_serial, client->chr->serial );
		return;
	}

	if (msg->lines == 0xFFFF)
	{
		//PolTimer timer;
		unsigned msglen = 0;

		BObject nlines_ob( book->call_custom_method( "getnumlines" ) );
		long nlines;
		if (nlines_ob.isa( BObjectImp::OTLong ))
		{
			BLong* blong = static_cast<BLong*>(nlines_ob.impptr());
			nlines = blong->value();
		}
		else
		{
			return;
		}

		PKTBI_66_HDR* phdr = reinterpret_cast<PKTBI_66_HDR*>(buffer);
		phdr->msgtype = PKTBI_66_ID;
		phdr->msglen = 0;
		phdr->book_serial = book->serial_ext;
		phdr->pages = ctBEu16(1);
		msglen = sizeof(*phdr);

		long linenum = (page-1)*8+1;

		PKTBI_66_CONTENTS* ppage = reinterpret_cast<PKTBI_66_CONTENTS*>(&buffer[msglen]);
		msglen += sizeof(*ppage);
		if (msglen > sizeof buffer)
			return;
		ppage->page = ctBEu16( page );

		int pagelines;
		for( pagelines = 0; pagelines < 8 && linenum <= nlines; ++pagelines, ++linenum )
		{
			string linetext;

			BObjectImpRefVec params;
			params.push_back( ref_ptr<BObjectImp>(new BLong(linenum)) );
			BObject line_ob = book->call_custom_method( "getline", params );
			linetext = line_ob->getStringRep();

			char* linebuf = reinterpret_cast<char*>(&buffer[msglen]);
			msglen += linetext.size()+1;
			if (msglen > sizeof buffer)
				return;
			memcpy( linebuf, linetext.c_str(), linetext.size()+1 );
		}
		ppage->lines = ctBEu16( pagelines );


		phdr->msglen = ctBEu16( msglen );
		//timer.printOn( cout );
		client->transmit( buffer, msglen );
	}
	else
	{
		//cout << "Write book page: " << endl;
		//fdump( stdout, msg, cfBEu16(msg->msglen) );
		size_t msglen = cfBEu16(msg->msglen);
		const char* ptext = msg->text;
		size_t bytesleft = msglen - offsetof( PKTBI_66, text );
		for( int i = 1; i <= 8; ++i )
		{
			string line;
			while (bytesleft)
			{
				--bytesleft;
				if (*ptext)
				{
					line.append( 1, *ptext );
					++ptext;
				}
				else
				{
					++ptext; // skip null terminator
					break;
				}
			}

			BObjectImpRefVec params;
			params.push_back( ref_ptr<BObjectImp>(new BLong((page-1)*8+i)) );
			params.push_back( ref_ptr<BObjectImp>(new String(line)) );

			BObject page_on = book->call_custom_method( "setline", params );
		}
	}
}
MESSAGE_HANDLER_VARLEN( PKTBI_66, read_book_page_handler );

char strip_ctrl_chars(char c)
{
	if(c < 0x20)
		return 0x20;
	else
		return c;
}

void open_book_handler( Client* client, PKTBI_93* msg )
{
	//fdump( stdout, msg, sizeof *msg );

	//string title( msg->title, sizeof msg->title );
	//string author( msg->author, sizeof msg->author );

	//Dave changed this 12/19 from sizeof msg->title. The protocol defines garbage after the terminator for
	//the title and author strings, so we were writing this garbage into save files. This caused some
	//"No SERIAL property" bugs, because the parser barfed on the bad characters.
	string title( msg->title, strlen(msg->title) );
	string author( msg->author, strlen(msg->author) );

	//dave 1/20/3 cheaters insert cntrl chars into books causing severe problems
	std::transform( title.begin(),title.end(),title.begin(),strip_ctrl_chars);
	std::transform( author.begin(),author.end(),author.begin(),strip_ctrl_chars);


	unsigned long book_serial = cfBEu32( msg->serial );
	Item *book = find_legal_item( client->chr, book_serial );
	if (book == NULL)
	{
		Log( "Unable to find book 0x%lx for character 0x%lx\n",
			  book_serial, client->chr->serial );
		return;
	}
	BObjectImpRefVec params;
	params.push_back( ref_ptr<BObjectImp>(new String(title)) );
	book->call_custom_method( "settitle", params );

	params[0].set( new String(author) );
	book->call_custom_method( "setauthor", params );
}
MESSAGE_HANDLER( PKTBI_93, open_book_handler );

BObjectImp* UOExecutorModule::mf_SendHousingTool()
{
	Character* chr;
	long house_serial;

	if (! (getCharacterParam( exec, 0, chr ) &&
		   exec.getParam( 1, house_serial )) )
	{
		return new BError( "Invalid parameter type" );
	}
	if( (chr->client->UOExpansionFlag & AOS) == 0 )
		return new BError( "Charater does not have AOS enabled." );

	UMulti* multi = system_find_multi(house_serial);
	if(multi == NULL)
		return new BError( "House not found." );

	UHouse* house = multi->as_house();
	if(house == NULL)
		return new BError( "Not a House multi." );

	if ( !house->IsCustom() )
		return new BError( "Not a Custom House." );

	if(house->editing)
		return new BError( "House currently being customized." );

	if( chr->realm->find_supporting_multi(chr->x,chr->y,chr->z) != house )
		return new BError( "You must be inside the house to customize it." );

	chr->client->gd->custom_house_serial = house_serial;

	PKTBI_BF msg;
	msg.msgtype = PKTBI_BF_ID;
	msg.subcmd = ctBEu16(PKTBI_BF::TYPE_ACTIVATE_CUSTOM_HOUSE_TOOL);
	msg.activatehousetool.house_serial = ctBEu32(house_serial);
	msg.activatehousetool.unk1 = 0x4;//begin
	msg.activatehousetool.unk2 = ctBEu16(0);//fixme what's the meaning
	msg.activatehousetool.unk3 = ctBEu32(0xFFFFFFFF); //fixme what's the meaning
	msg.activatehousetool.unk4 = 0xFF;//fixme what's the meaning
	msg.msglen = ctBEu16(17);
	chr->client->transmit(&msg,17);

	move_character_to(chr,house->x,house->y,house->z+7,MOVEITEM_FORCELOCATION, NULL);
	//chr->set_script_member("hidden",1);
	//chr->set_script_member("frozen",1);
	house->editing = true;
	house->editing_floor_num = 1;

	CustomHousesSendFull(house, chr->client,HOUSE_DESIGN_WORKING);

	return new BLong(1);

}

BObjectImp* UOExecutorModule::mf_SendCharacterRaceChanger(/* Character */)
{
	Character* chr;
	if (getCharacterParam( exec, 0, chr ))
	{
		PKTBI_BF msg;
		msg.msgtype = PKTBI_BF_ID;
		msg.subcmd = ctBEu16(PKTBI_BF::TYPE_CHARACTER_RACE_CHANGER);
		msg.characterracechanger.call.gender = static_cast<u8>(chr->gender);
		msg.characterracechanger.call.race = static_cast<u8>(chr->race+1);
		msg.msglen = ctBEu16(7);
		chr->client->transmit(&msg,7);

		return new BLong(1);
	}
	else
		return new BError( "Invalid parameter" );
}

bool validhair(u16 HairStyle);
bool validbeard(u16 BeardStyle);

void character_race_changer_handler( Client* client, PKTBI_BF* msg )
{
	Item* tmpitem;

	if (msg->msglen == ctBEu16(5))
		return;

	if ((msg->characterracechanger.result.BodyHue == 0) &&
		(msg->characterracechanger.result.HairId == 0) &&
		(msg->characterracechanger.result.HairHue == 0) &&
		(msg->characterracechanger.result.BeardId == 0) &&
		(msg->characterracechanger.result.BeardHue == 0))
		return;

	client->chr->setcolor( cfBEu16( msg->characterracechanger.result.BodyHue ) | 0x8000 );
    client->chr->truecolor = client->chr->color;

	client->chr->on_color_changed();

	// Create Hair
	if (client->chr->layer_is_equipped(LAYER_HAIR))
		destroy_item(client->chr->wornitem(LAYER_HAIR));

	if (validhair(cfBEu16(msg->characterracechanger.result.HairId)))
	{
		tmpitem=Item::create( cfBEu16(msg->characterracechanger.result.HairId) );
		tmpitem->layer = LAYER_HAIR;
		tmpitem->color = cfBEu16(msg->characterracechanger.result.HairHue);
		tmpitem->color_ext = ctBEu16(tmpitem->color);
		tmpitem->realm = client->chr->realm;
		client->chr->equip(tmpitem);
		send_wornitem_to_inrange( client->chr, tmpitem );
	}

	// Create Beard
	if (client->chr->layer_is_equipped(LAYER_BEARD))
		destroy_item(client->chr->wornitem(LAYER_BEARD));

	if (validbeard(cfBEu16(msg->characterracechanger.result.BeardId)))
	{
		tmpitem=Item::create( cfBEu16(msg->characterracechanger.result.BeardId) );
		tmpitem->layer = LAYER_BEARD;
		tmpitem->color = cfBEu16(msg->characterracechanger.result.BeardHue);
		tmpitem->color_ext = ctBEu16(tmpitem->color);
		tmpitem->realm = client->chr->realm;
		client->chr->equip(tmpitem);
		send_wornitem_to_inrange( client->chr, tmpitem );
	}
}
