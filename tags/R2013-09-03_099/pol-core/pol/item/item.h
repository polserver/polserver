/*
History
=======
2008/12/17 MuadDub:   Added item.tile_layer - returns layer entry from tiledata/tiles.cfg
2009/08/06 MuadDib:   Added gotten_by for character ref.
2009/08/06 MuadDib:   Added gotten_by code for items.

Notes
=======

*/

#ifndef __ITEM_H
#define __ITEM_H
#define ITEM_H

#include "../uobject.h"
#include "../../bscript/bobject.h"
#include "../ssopt.h"

class Character;
class UContainer;
class Client;
class ItemDesc;
class UMulti;
struct USTRUCT_TILE;

std::string format_description( unsigned int polflags, const std::string& descdef, unsigned short amount );

// NOTE: All derived classes should have protected constructors, 
//			and make Item a friend.
class Item : public UObject
{
	typedef UObject base;
public:
	bool stackable() const;

	virtual ~Item();
	
	virtual void double_click( Client* client );
	virtual void builtin_on_use( Client* client );
	virtual void walk_on( Character* chr );
	virtual u8 los_height() const;
	
	const ItemDesc& itemdesc() const;

	virtual u16 get_senditem_amount() const;
	u16 getamount() const;
	void setamount( u16 amount );
	void subamount( u16 amount_subtract );

	bool movable() const;
	void movable( bool newvalue );
	bool default_movable() const;
	bool default_invisible() const;

	bool inuse() const;
	void inuse( bool newvalue );

	bool is_gotten() const;
	void is_gotten( bool newvalue );

	bool invisible() const;
	void invisible( bool newvalue );
	void on_invisible_changed();

	virtual bool saveonexit() const;
	virtual void saveonexit( bool newvalue );

	void set_decay_after( unsigned int seconds );
	bool should_decay( unsigned int gameclock ) const;
	void restart_decay_timer();
	void disable_decay();

	bool setlayer( unsigned char layer );
	virtual bool setgraphic( u16 newobjtype );
	virtual bool setcolor( u16 newcolor );
	virtual void on_color_changed();
	virtual void spill_contents( UMulti* supporting_multi );

	virtual void setfacing( u8 newfacing );
	virtual void on_facing_changed();

	std::string description() const;
	std::string merchant_description() const;
	std::string get_use_script_name() const;
	unsigned int sellprice() const;
	unsigned int buyprice() const; //Dave added this 11/28
	bool getbuyprice( unsigned int& buyprice ) const;

	bool newbie() const;
	bool default_newbie() const;
	void newbie( bool newvalue );

	u8 slot_index() const;
	bool slot_index( u8 newvalue );

	virtual unsigned int item_count() const;
	unsigned int weight_of( unsigned short amount ) const;
	virtual unsigned int weight() const;

	virtual std::string name() const;
	virtual UObject* owner();
	virtual const UObject* owner() const;
	virtual UObject* toplevel_owner();
	virtual const UObject* toplevel_owner() const;

	bool can_add_to_self( unsigned short amount ) const;
	virtual bool can_add_to_self( const Item& item ) const;
	bool has_only_default_cprops(const ItemDesc* compare = NULL) const;
	virtual void add_to_self( Item*& item ); // deletes the item passed

  #ifdef PERGON
  virtual void ct_merge_stacks_pergon( Item*& item_sub ); // Pergon: Re-Calculate Property CreateTime after Merging of two Stacks
  virtual void ct_merge_stacks_pergon( u16 amount_sub );  // Pergon: Re-Calculate Property CreateTime after Adding Items to a Stack
  #endif

	virtual bool amount_to_remove_is_partial( u16 amount_to_remove ) const;
			
			// NOTE: this_item_amount is the amount that this item
			//	  will have.  A new item will be created, whos amount
			//		is the remainder. This is the returned item.
	virtual Item* slice_stacked_item( u16 this_item_new_amount );

	Item* remove_part_of_stack( u16 amount_to_remove );
	
	void set_use_script( const std::string& scriptname );
	void extricate();

	bool has_equip_script() const;
	BObjectImp* run_equip_script( Character* chr, bool startup );
	bool check_equip_script( Character* chr, bool startup );
	BObjectImp* run_unequip_script( Character* who );
	bool check_unequip_script();

	bool check_test_scripts( Character* chr, const std::string& script_ecl, bool startup );
	bool check_equiptest_scripts( Character* chr, bool startup = false );
	bool check_unequiptest_scripts( Character* chr );
	bool check_unequiptest_scripts();

	//DAVE added this 11/17, Shortcut function to get a pointer to the owner character
	Character* GetCharacterOwner();
	// I'm using the named constructor idiom for creation,
	// so that the right kind of object gets created
	// and so that object serial numbers get generated.
	// If no serial passed, one will be generated.
	static Item* create( u32 objtype, u32 serial = 0 );
	static Item* create( const ItemDesc& itemdesc, u32 serial = 0 );
	virtual void readProperties( ConfigElem& elem );
	virtual Item* clone() const;
	//virtual class BObjectImp* script_member( const char *membername );
	virtual BObjectImp* make_ref();
	virtual BObjectImp* get_script_member( const char *membername ) const;
	virtual BObjectImp* get_script_member_id( const int id ) const; ///id test
	virtual BObjectImp* set_script_member( const char *membername, const std::string& value );
	virtual BObjectImp* set_script_member( const char *membername, int value );
	virtual BObjectImp* set_script_member_id( const int id, const std::string& value );///id test
	virtual BObjectImp* set_script_member_id( const int id, int value );///id test
    virtual BObjectImp* set_script_member_double( const char *membername, double value );
    virtual BObjectImp* set_script_member_id_double( const int id, double value ); //id test
	virtual BObjectImp* script_method( const char *methodname, Executor& ex );
	virtual BObjectImp* script_method_id( const int id, Executor& ex );

	virtual bool script_isa( unsigned isatype ) const;
	virtual const char* target_tag() const;
	virtual const char* classname() const;
	virtual BObjectImp* custom_script_method( const char* methodname, Executor& ex );
	BObject call_custom_method( const char* methodname, BObjectImpRefVec& pmore );
	BObject call_custom_method( const char* methodname );

protected: // only derived classes need the constructor
	virtual void printProperties( std::ostream& os ) const;
	virtual void printDebugProperties( std::ostream& os ) const;
	Item(const ItemDesc& itemdesc, UOBJ_CLASS uobj_class);

public:
	UContainer* container;
	Character* gotten_by;
protected:
	unsigned int decayat_gameclock_;
	unsigned int sellprice_;
	unsigned int buyprice_;
	u16 amount_;
	bool newbie_;
	bool movable_;
	bool inuse_;
	bool is_gotten_;
	bool invisible_;
	
	u8 slot_index_;

	std::string on_use_script_;
	std::string equip_script_;
	std::string unequip_script_;
public:
	u8 layer;
	u8 tile_layer;
    unsigned short hp_;
	unsigned short maxhp() const;
	double quality_;

	s16 calc_element_resist( unsigned element ) const;
	s16 calc_element_damage( unsigned element ) const;
	bool has_resistance(Character* chr);
	bool has_element_damage();
};

//typedef ref_ptr<Item> ItemRefPtr;

inline u16 Item::getamount() const
{
	return amount_;
}

inline bool Item::movable() const
{
	return movable_;
}

inline void Item::movable( bool newvalue )
{
	movable_ = newvalue;
}

inline bool Item::inuse() const
{
	return inuse_;
}

inline void Item::inuse( bool newvalue )
{
	inuse_ = newvalue;
}

inline bool Item::is_gotten() const
{
	return is_gotten_;
}

inline void Item::is_gotten( bool newvalue )
{
	is_gotten_ = newvalue;
}

inline bool Item::invisible() const
{
	return invisible_;
}

inline void Item::invisible( bool newvalue )
{
	invisible_ = newvalue;
}

inline bool Item::newbie() const
{
	return newbie_;
}

inline void Item::newbie( bool newvalue )
{
	newbie_ = newvalue;
}

inline u8 Item::slot_index() const
{
	return slot_index_;
}

inline bool Item::slot_index( u8 newvalue )
{
	if (ssopt.use_slot_index)
	{
		if (newvalue < ssopt.default_max_slots)
		{
			slot_index_ = newvalue;
			return true;
		}
		else
			return false;
	}
	return true;
}

#endif
