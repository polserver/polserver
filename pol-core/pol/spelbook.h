/*
History
=======


Notes
=======

*/

#ifndef __SPELBOOK_H
#define __SPELBOOK_H

#ifndef __CONTAINR_H
#include "containr.h"
#endif

#include "item/itemdesc.h"
#include "network/client.h"

class Spellbook : public UContainer
{
public:
	explicit Spellbook( const SpellbookDesc& descriptor );
	virtual ~Spellbook();

    bool has_spellid( unsigned long spellid ) const;
	u8 bitwise_contents[8];
	short spell_school;
	virtual void add( Item *item );
	virtual void printProperties( ostream& os ) const;
	virtual void readProperties( ConfigElem& elem );
	virtual void printOn( ostream& os ) const;
	virtual void printSelfOn( ostream& os ) const;
	virtual void double_click( Client* client );
	virtual bool script_isa( unsigned isatype ) const;

private:
	virtual bool can_add( const Item& item ) const;
    virtual void add_bulk( long item_count_delta, long weight_delta );
	void calc_current_bitwise_contents();
protected:
	
	friend class Item;
};

class USpellScroll : public Item
{
public:
    virtual u16 get_senditem_amount() const;
    static u16 convert_objtype_to_spellnum( u16 objtype, int school );
protected:
    explicit USpellScroll( const ItemDesc& descriptor );
    friend Item* Item::create( const ItemDesc& itemdesc, u32 serial );
private:
};

#endif
