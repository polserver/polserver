/*
History
=======
2005/02/14 Shinigami: double_click - simple logical error in layer_is_equipped check
2009/07/26 MuadDib:   Packet struct refactoring.
2009/09/17 MuadDib:   Spellbook::can_add upgraded to check bitflags instead of contents.
2009/09/18 MuadDib:   Spellbook rewrite to deal with only bits, not scrolls inside them.

Notes
=======

*/

#include "../clib/stl_inc.h"
#include "../clib/cfgelem.h"
#include "../clib/endian.h"
#include "mobile/charactr.h"
#include "item/itemdesc.h"
#include "spelbook.h"
#include "objtype.h"
#include "pktboth.h"
#include "polcfg.h"
#include "polclass.h"
#include "spells.h"
#include "ufunc.h"
#include "ssopt.h"

Spellbook::Spellbook( const SpellbookDesc& descriptor ) :
	UContainer( descriptor ),
	spell_school(0)
{
	if(descriptor.spelltype == "Magic")
		spell_school = 0;
	else if(descriptor.spelltype == "Necro")
		spell_school = 1;
	else if(descriptor.spelltype == "Paladin")
		spell_school = 2;
	else if(descriptor.spelltype == "Bushido")
		spell_school = 4;
	else if(descriptor.spelltype == "Ninjitsu")
		spell_school = 5;
	else if(descriptor.spelltype == "SpellWeaving")
		spell_school = 6;
	for(int i=0; i<8; i++)
		bitwise_contents[i] = 0;
}

Spellbook::~Spellbook()
{
}

void Spellbook::double_click( Client* client )
{
	if(client->chr->is_equipped(this))
	{
		send_put_in_container( client, this ); 
		send_wornitem( client, client->chr, this );
	}
	else if(container != NULL)
		send_put_in_container( client, this );
	else
	{
		send_sysmessage( client, "Spellbooks must be equipped or in the top level backpack to use." );
		return;
	}

	if(bitwise_contents[0] == 0) //assume never been clicked using the new bitwise spell scheme
		calc_current_bitwise_contents();

	if( !(client->UOExpansionFlag & AOS) && 
		     (spell_school == 0) )
	{
		 send_book_old(client);
	}
	else if( !(client->UOExpansionFlag & AOS) && 
		     (spell_school == 1 || spell_school == 2))
	{
		send_sysmessage( client, "This item requires at least the Age of Empires Expansion." );
		return;
	}
	else if( !(client->UOExpansionFlag & SE) && 
		     (spell_school == 4 || spell_school == 5))
	{
		 send_sysmessage( client, "This item requires at least the Samurai Empire Expansion." );
		return;
	}
	else if( !(client->UOExpansionFlag & ML) && 
		     spell_school == 6)
	{
		 send_sysmessage( client, "This item requires at least the Mondain's Legacy Expansion." );
		return;
	}
	else
	{
		// Ok, now we do a strange check. This is for those people who have no idea that you
		// must have AOS Features Enabled on an acct with AOS Expansion to view Magery book.
		// All newer spellbooks will bug out if you use this method though.
		if( (client->UOExpansionFlag & AOS) && (spell_school == 0) 
			&& !(ssopt.uo_feature_enable & PKTOUT_A9_START_FLAGS::FLAG_AOS_FEATURES))
		{
			if ( config.loglevel > 1 )
				cout << "Client with AOS Expansion Account using spellbook without UOFeatureEnable 0x20 Bitflag.\n";
			send_book_old(client);
			return;
		}

		send_open_gump(client, *this);
		PKTBI_BF msg;
		msg.msgtype = PKTBI_BF_ID;
		msg.msglen = ctBEu16(23);
		msg.subcmd = ctBEu16(PKTBI_BF::TYPE_NEW_SPELLBOOK);
		msg.newspellbook.unk = ctBEu16(1);
		msg.newspellbook.book_serial = serial_ext;
		msg.newspellbook.graphic = ctBEu16(graphic);
		msg.newspellbook.scroll_offset = ctBEu16((spell_school * 100) + 1);
		memcpy(msg.newspellbook.content,bitwise_contents,8);
		client->transmit(&msg, 23);
	}

}

bool Spellbook::has_spellid( unsigned long spellid ) const
{
	u16 spellnumber = static_cast<u16>(spellid);
	u8  spellslot = spellnumber % 8;
	if(spellslot == 0) spellslot = 8;

	int bitcheck = ((bitwise_contents[ (spellnumber-1) / 8 ]) & (1 << (spellslot-1)));
	if ( ((bitwise_contents[ (spellnumber-1) / 8 ]) & (1 << (spellslot-1))) != 0 )
		return true;

	return false;
}

bool Spellbook::can_add( const Item& item ) const
{
	// note: item count maximums are implicitly checked for.

	// you can only add scrolls to spellbooks
	if (item.objtype_ < spell_scroll_objtype_limits[spell_school][0] ||
		item.objtype_ > spell_scroll_objtype_limits[spell_school][1])
	{
		return false;
	}

	// you can only add one of each kind of scroll to a spellbook.
	u16 spellnum = USpellScroll::convert_objtype_to_spellnum(item.objtype_, spell_school);
	u8  spellslot = spellnum % 8;
	if(spellslot == 0) spellslot = 8;
	if ( bitwise_contents[ (spellnum-1) / 8 ] & (1 << (spellslot-1)) )
		return false;

	return true;
}

void Spellbook::add_bulk( long /* item_count_delta */, long /* weight_delta */ )
{
    // spellbooks don't modify their weight, either when adding
    // or when removing an item.
}

void Spellbook::add( Item *item )
{
//	UContainer::add(item);
	u16 spellnum = USpellScroll::convert_objtype_to_spellnum(item->objtype_, spell_school);
	u8  spellslot = spellnum % 8;
	if(spellslot == 0) spellslot = 8;
	bitwise_contents[ (spellnum-1) / 8 ] |= 1 << (spellslot-1);
	item->destroy();
//	item->saveonexit(0);
}

void Spellbook::printProperties( ostream& os ) const
{
    base::printProperties(os);
	
	for(int i=0; i<8; i++)
		os << "\tSpellbits" << i << "\t" << (int)bitwise_contents[i] << pf_endl;
}

void Spellbook::readProperties( ConfigElem& elem )
{
    base::readProperties( elem );
	ostringstream os;
	for(int i=0; i<8; i++)
	{
		os << "Spellbits" << i;
		bitwise_contents[i] = (u8) elem.remove_ushort( os.str().c_str(), 0 );
		os.str("");
	}
}

void Spellbook::printOn( ostream& os ) const
{
    base::printOn( os );
	printContents( os );
}
void Spellbook::printSelfOn( ostream& os ) const
{
    base::printOn( os );
}

void Spellbook::calc_current_bitwise_contents()
{
    for( UContainer::const_iterator itr = begin(); itr != end(); ++itr )
	{
		const Item* scroll = GET_ITEM_PTR( itr );
		u16 spellnum = USpellScroll::convert_objtype_to_spellnum(scroll->objtype_, spell_school);
		u8  spellslot = spellnum % 8;
		if(spellslot == 0) spellslot = 8;
		bitwise_contents[ (spellnum-1) / 8 ] |= 1 << (spellslot-1);
	}	

	// ok, it's been upgraded. Destroy everything inside it.
	for( UContainer::iterator itr = begin(); itr != end(); ++itr )
	{
		Item* scroll = GET_ITEM_PTR( itr );
		scroll->destroy();
	}
}

USpellScroll::USpellScroll( const ItemDesc& itemdesc ) :
    Item(itemdesc, CLASS_ITEM)
{
}

u16 USpellScroll::convert_objtype_to_spellnum( u16 objtype, int school )
{
	u16 spellnum = objtype - spell_scroll_objtype_limits[school][0] + 1;
	if(school == 0) //weirdness in order of original spells
	{
		if (spellnum == 1)
			spellnum = 7;
		else if (spellnum <= 7)
			spellnum--;
	}
	return spellnum;
}

// Spell scrolls send their spell ID in AMOUNT while they're in a spellbook.
// Otherwise, they're stackable I believe.
u16 USpellScroll::get_senditem_amount() const
{
    if ((container != NULL) && (container->script_isa(POLCLASS_SPELLBOOK) ))
    {
		Spellbook* book = static_cast<Spellbook*>(container);
		return convert_objtype_to_spellnum(objtype_, book->spell_school);
    }
    else // not contained, or not in a spellbook
    {
        return amount_;
    }
}

void Spellbook::send_book_old( Client *client )
{
	client->pause();

	if (!locked_)
	{
		send_open_gump( client, *this );
		send_spellbook_contents( client, *this );
	}
	else
	{
		send_sysmessage( client, "That is locked." );
	}

	client->restart();
}

void send_spellbook_contents( Client *client, Spellbook& spellbook )
{
	if ( client->ClientType & CLIENTTYPE_6017 )
	{
		static PKTOUT_3C_6017 msg;

		msg.msgtype = PKTOUT_3C_ID;

		int count = 0;
		for ( u16 i = 0; i < 64; ++i )
		{
			u16 objtype = spell_scroll_objtype_limits[0][0] + i;
			u16 spellnumber = USpellScroll::convert_objtype_to_spellnum( objtype, spellbook.spell_school );
			u8  spellpos = spellnumber % 8; // spellpos is the spell's position it it's circle's array.
			if(spellpos == 0) spellpos = 8;
			if ( ((spellbook.bitwise_contents[ ((spellnumber-1) / 8) ]) & (1 << (spellpos-1))) != 0 )
			{
				msg.items[count].serial = 0x7FFFFFFF - spellnumber;
				msg.items[count].graphic = ctBEu16(objtype);
				msg.items[count].unk6_00 = 0x00;

				msg.items[count].amount = ctBEu16( spellnumber );

				msg.items[count].x = ctBEu16(1);
				msg.items[count].y = ctBEu16(1);
				msg.items[count].slot_index = 0x00;
				msg.items[count].container_serial = spellbook.serial_ext;
				msg.items[count].color = 0x00;
				++count;
			}
		}

		unsigned short msglen = static_cast<unsigned short>(offsetof( PKTOUT_3C_6017, items ) + 
			count * sizeof msg.items[0]);
		msg.msglen = ctBEu16( msglen );
		msg.count = ctBEu16( count );

		client->transmit( &msg, msglen );	
	}
	else
	{
		static PKTOUT_3C msg;

		msg.msgtype = PKTOUT_3C_ID;

		int count = 0;
		for ( u16 i = 0; i < 64; ++i )
		{
			u16 objtype = spell_scroll_objtype_limits[0][0] + i;
			u16 spellnumber = USpellScroll::convert_objtype_to_spellnum( objtype, spellbook.spell_school );
			u8  spellpos = spellnumber % 8; // spellpos is the spell's position it it's circle's array.
			if(spellpos == 0) spellpos = 8;
			if ( ((spellbook.bitwise_contents[ ((spellnumber-1) / 8) ]) & (1 << (spellpos-1))) != 0 )
			{
				msg.items[count].serial = 0x7FFFFFFF - spellnumber;
				msg.items[count].graphic = ctBEu16(objtype);
				msg.items[count].unk6_00 = 0x00;

				msg.items[count].amount = ctBEu16( spellnumber );

				msg.items[count].x = ctBEu16(1);
				msg.items[count].y = ctBEu16(1);
				msg.items[count].container_serial = spellbook.serial_ext;
				msg.items[count].color = 0x00;
				++count;
			}
		}

		unsigned short msglen = static_cast<unsigned short>(offsetof( PKTOUT_3C, items ) + 
			count * sizeof msg.items[0]);
		msg.msglen = ctBEu16( msglen );
		msg.count = ctBEu16( count );

		client->transmit( &msg, msglen );	
	}
}
