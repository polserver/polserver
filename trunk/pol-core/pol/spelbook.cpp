/*
History
=======
2005/02/14 Shinigami: double_click - simple logical error in layer_is_equipped check
2009/07/26 MuadDib:   Packet struct refactoring.

Notes
=======

*/

#include "clib/stl_inc.h"
#include "clib/cfgelem.h"
#include "clib/endian.h"
#include "charactr.h"
#include "itemdesc.h"
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

	if( !(client->UOExpansionFlag & AOS) && 
		     (spell_school == 0) )
	{
		 base::double_click(client);
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
			base::double_click(client);
			return;
		}

		if(bitwise_contents[0] == 0) //assume never been clicked using the new bitwise spellscheme
			calc_current_bitwise_contents();

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
    for( UContainer::const_iterator itr = begin(); itr != end(); ++itr )
	{
		const Item* scroll = GET_ITEM_PTR( itr );
        if ( (unsigned long)(USpellScroll::convert_objtype_to_spellnum(scroll->objtype_, spell_school) + (spell_school * 100)) == spellid)
			return true;
	}

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
    for( UContainer::const_iterator itr = begin(); itr != end(); ++itr )
	{
		const Item* scroll = GET_ITEM_PTR( itr );
		if (scroll->objtype_ == item.objtype_)
			return false;
	}

	return true;
}

void Spellbook::add_bulk( long /* item_count_delta */, long /* weight_delta */ )
{
    // spellbooks don't modify their weight, either when adding
    // or when removing an item.
}

void Spellbook::add( Item *item )
{
	UContainer::add(item);
	u16 spellnum = USpellScroll::convert_objtype_to_spellnum(item->objtype_, spell_school);
	u8  spellslot = spellnum % 8;
	if(spellslot == 0) spellslot = 8;
	bitwise_contents[ (spellnum-1) / 8 ] |= 1 << (spellslot-1);
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
}

USpellScroll::USpellScroll( const ItemDesc& itemdesc ) :
    Item(itemdesc, CLASS_ITEM)
{
}

// See docs\spells.txt for what's going on here.
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
