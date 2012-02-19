/*
History
=======
2005/02/14 Shinigami: double_click - simple logical error in layer_is_equipped check
2009/07/26 MuadDib:   Packet struct refactoring.
2009/09/17 MuadDib:   Spellbook::can_add upgraded to check bitflags instead of contents.
2009/09/18 MuadDib:   Spellbook rewrite to deal with only bits, not scrolls inside them.
2009/10/10 Turley:    Added spellbook.addspell() & .removespell() methods

Notes
=======

*/
#include "../clib/stl_inc.h"

#include "../clib/cfgelem.h"
#include "../clib/endian.h"
#include "item/itemdesc.h"
#include "mobile/charactr.h"
#include "network/packets.h"
#include "network/clienttransmit.h"
#include "objtype.h"
#include "pktboth.h"
#include "polcfg.h"
#include "polclass.h"
#include "spelbook.h"
#include "spells.h"
#include "ssopt.h"
#include "ufunc.h"

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
	// Use spell school 3 for Mysticism with spellid 678+ because on OSI nothing uses this so we can use this here.
	else if(descriptor.spelltype == "Mysticism")
		spell_school = 3;
	else if(descriptor.spelltype == "Bushido")
		spell_school = 4;
	else if(descriptor.spelltype == "Ninjitsu")
		spell_school = 5;
	else if(descriptor.spelltype == "SpellWeaving")
		spell_school = 6;
	else if(descriptor.spelltype == "BardMasteries")
		spell_school = 7;
	for(int i=0; i<8; ++i)
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
		send_sysmessage( client, "This item requires at least the Age of Shadows Expansion." );
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
	else if( !(client->UOExpansionFlag & SA) && (spell_school == 3 || spell_school == 7))
	{
		 send_sysmessage( client, "This item requires at least the Stygian Abyss Expansion." );
		 return;
	}
	else
	{
		// Ok, now we do a strange check. This is for those people who have no idea that you
		// must have AOS Features Enabled on an acct with AOS Expansion to view Magery book.
		// All newer spellbooks will bug out if you use this method though.
		if( (client->UOExpansionFlag & AOS) && (spell_school == 0) 
			&& !(ssopt.uo_feature_enable & PKTOUT_A9::FLAG_AOS_FEATURES))
		{
			if ( config.loglevel > 1 )
				cout << "Client with AOS Expansion Account using spellbook without UOFeatureEnable 0x20 Bitflag.\n";
			send_book_old(client);
			return;
		}

		send_open_gump(client, *this);

		PktHelper::PacketOut<PktOut_BF_Sub1B> msg;
		msg->WriteFlipped<u16>(23);
		msg->offset+=2; //sub
		msg->WriteFlipped<u16>(1);
		msg->Write<u32>(serial_ext);
		msg->WriteFlipped<u16>(graphic);

		// Check Mysticism spell here
		if (spell_school == 3)
			msg->WriteFlipped<u16>(678);
		else
			msg->WriteFlipped<u16>(static_cast<u16>((spell_school * 100) + 1));

		msg->Write(bitwise_contents,8);
		msg.Send(client);
	}
}

bool Spellbook::has_spellid( unsigned int spellid ) const
{
    u8 spellschool;

	// Check Mysticism here
	if (spellid >= 678 && spellid <= 693)
		spellschool = 3;
	else
		spellschool = static_cast<u8>(spellid / 100);

    if (spellschool == this->spell_school)
    {
        u16 spellnumber;

		// Check Mysticism here
		if ( spellid >= 678 && spellid <= 693)
			spellnumber = static_cast<u16>(spellid - 677);
		else
			spellnumber = static_cast<u16>(spellid % 100);

		// Limits spellnumber to be between 1-64
		spellnumber = spellnumber & 63;
		if (spellnumber == 0) spellnumber = 64;

        u8  spellslot = spellnumber & 7;
        if(spellslot == 0) spellslot = 8;

        if ( ((bitwise_contents[ (spellnumber-1) >> 3 ]) & (1 << (spellslot-1))) != 0 )
            return true;
    }
    return false;
}

bool Spellbook::remove_spellid( unsigned int spellid )
{
    if (has_spellid(spellid))
    {
        u16 spellnumber;

		// Check Mysticism here
		if ( spellid >= 678 && spellid <= 693)
			spellnumber = static_cast<u16>(spellid - 677);
		else
			spellnumber = static_cast<u16>(spellid % 100);

		// Limits spellnumber to be between 1-64
		spellnumber = spellnumber & 63;
		if (spellnumber == 0) spellnumber = 64;

        u8  spellslot = spellnumber & 7;
        if(spellslot == 0) spellslot = 8;
        bitwise_contents[ (spellnumber-1) >> 3 ] &= ~(1 << (spellslot-1));
        return true;
    }
    return false;
}

bool Spellbook::add_spellid( unsigned int spellid )
{
    if (!has_spellid(spellid))
    {
        u16 spellnumber;

		// Check Mysticism here
		if ( spellid >= 678 && spellid <= 693)
			spellnumber = static_cast<u16>(spellid - 677);
		else
			spellnumber = static_cast<u16>(spellid % 100);

		// Limits spellnumber to be between 1-64
		spellnumber = spellnumber & 63;
		if (spellnumber == 0) spellnumber = 64;

        u8  spellslot = spellnumber & 7;
        if(spellslot == 0) spellslot = 8;
        bitwise_contents[ (spellnumber-1) >> 3 ] |= 1 << (spellslot-1);
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
	u16 spellnum = USpellScroll::convert_objtype_to_spellnum(item.objtype_, spell_school);
	u8  spellslot = spellnum & 7;
	if(spellslot == 0) spellslot = 8;
	if ( bitwise_contents[ (spellnum-1) >> 3 ] & (1 << (spellslot-1)) )
		return false;

	return true;
}

void Spellbook::add_bulk( int /* item_count_delta */, int /* weight_delta */ )
{
    // spellbooks don't modify their weight, either when adding
    // or when removing an item.
}

void Spellbook::add( Item *item )
{
//	UContainer::add(item);
	u16 spellnum = USpellScroll::convert_objtype_to_spellnum(item->objtype_, spell_school);
	u8  spellslot = spellnum & 7;
	if(spellslot == 0) spellslot = 8;
	bitwise_contents[ (spellnum-1) >> 3 ] |= 1 << (spellslot-1);
	item->destroy();
//	item->saveonexit(0);
}

void Spellbook::printProperties( ostream& os ) const
{
    base::printProperties(os);
	
	for(int i=0; i<8; ++i)
		os << "\tSpellbits" << i << "\t" << (int)bitwise_contents[i] << pf_endl;
}

void Spellbook::readProperties( ConfigElem& elem )
{
    base::readProperties( elem );
	ostringstream os;
	for(int i=0; i<8; ++i)
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
		u8  spellslot = spellnum & 7;
		if(spellslot == 0) spellslot = 8;
		bitwise_contents[ (spellnum-1) >> 3 ] |= 1 << (spellslot-1);
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

u16 USpellScroll::convert_objtype_to_spellnum( u32 objtype, u8 school )
{
	u16 spellnum = static_cast<u16>(objtype - spell_scroll_objtype_limits[school][0] + 1);
	if(school == 0) //weirdness in order of original spells
	{
		if (spellnum == 1)
			spellnum = 7;
		else if (spellnum <= 7)
			--spellnum;
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
	PktHelper::PacketOut<PktOut_3C> msg;
	msg->offset+=4; //msglen+count
	u16 count = 0;
	for ( u16 i = 0; i < 64; ++i )
	{
		u32 objtype = spell_scroll_objtype_limits[0][0] + i;
		u16 spellnumber = USpellScroll::convert_objtype_to_spellnum( objtype, spellbook.spell_school );
		u8  spellpos = spellnumber & 7; // spellpos is the spell's position it it's circle's array.
		if(spellpos == 0) spellpos = 8;
		if ( ((spellbook.bitwise_contents[ ((spellnumber-1) >> 3) ]) & (1 << (spellpos-1))) != 0 )
		{
			msg->Write<u32>(static_cast<u32>(0x7FFFFFFF - spellnumber));
			msg->WriteFlipped<u16>(static_cast<u16>(objtype));
			msg->offset++; //unk6
			msg->WriteFlipped<u16>(spellnumber); //amount
			msg->WriteFlipped<u16>(1); //x
			msg->WriteFlipped<u16>(1); //y
			if ( client->ClientType & CLIENTTYPE_6017 )
				msg->Write<u8>(0); //slotindex
			msg->Write<u32>(spellbook.serial_ext);
			msg->WriteFlipped<u16>(0); //color
			++count;
		}
	}
	u16 len=msg->offset;
	msg->offset=1;
	msg->WriteFlipped<u16>(len);
	msg->WriteFlipped<u16>(count);
	msg.Send(client, len );
}
