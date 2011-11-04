/*
History
=======
2005/04/28 Shinigami: mf_EnumerateItemsInContainer/enumerate_contents - added flag to list content of locked container too
2006/05/07 Shinigami: mf_SendBuyWindow & mf_SendSellWindow - added Flags to send Item Description using AoS Tooltips
2009/08/07 MuadDib:   Added layer_list_ and functions like WornItems to corpse class. Used to handle showing equippable items
                      on a corpse.

Notes
=======

*/

#ifndef __CONTAINR_H
#define __CONTAINR_H

#ifndef __LOCKABLE_H
#include "lockable.h"
#endif

#ifndef _UCFG_H
#include "ucfg.h"
#endif

#include "layers.h"
#include "objtype.h"
#include "reftypes.h"

class Character;
class ContainerDesc;

#define CONTAINER_STORES_ITEMREF 0

#if CONTAINER_STORES_ITEMREF
#define GET_ITEM_PTR( itr )     ((*itr).get())
#define ITEM_ELEM_PTR( elem )   (elem.get())
#define EMPTY_ELEM              ItemRef(0)
#else
#define GET_ITEM_PTR( itr )     (*itr)
#define ITEM_ELEM_PTR( elem )   (elem)
#define EMPTY_ELEM              0
#endif
/* A container promises never to allow more than MAX_CONTAINER_ITEMS in it.
   The user must check can_add() before adding, however, to make sure the
   add would be legal, or an exception will be thrown.

   THAT IS: the user MUST check can_add() before adding.
*/

class UContainer : public ULockable
{
public:
    typedef ULockable base;
	virtual ~UContainer();

    virtual void destroy();
    void destroy_contents();

    typedef std::vector<Item*> Contents;
    typedef Contents::iterator iterator;
    typedef Contents::const_iterator const_iterator;

    virtual void builtin_on_use( Client *client );

	// can_add(): doesn't look for matching stacks to add to.
	virtual bool can_add( const Item& item ) const;
    virtual bool can_add( unsigned short more_weight ) const;

	virtual void spill_contents( UMulti* supporting_multi );
	
	virtual void add( Item *item ); // NOTE: points item->container to self on insertion
    void add_at_random_location( Item* item );
	unsigned count() const;


    virtual unsigned int item_count() const;
    virtual unsigned int weight() const;

    bool can_add_bulk( int tli_diff, int item_count_diff, int weight_diff ) const;
    virtual void add_bulk( int item_count_delta, int weight_delta );
    void add_bulk( const Item* item );
    void remove_bulk( const Item* item );

	virtual bool can_add_to_slot( u8& slotIndex );
	bool is_slot_empty(u8& slotIndex);
	bool find_empty_slot(u8& slotIndex);

	Item* find( u32 serial ) const;
    Item* find_toplevel( u32 serial ) const;
	Item* find_toplevel_objtype( u32 objtype ) const;
    Item* find_toplevel_objtype( u32 objtype, unsigned short maxamount ) const;
	Item* find_toplevel_polclass( unsigned int polclass ) const;
    Item* find_addable_stack( const Item* adding_item ) const;

	Item* find_toplevel_objtype_noninuse( u32 objtype ) const;
    Item* find_toplevel_objtype_noninuse( u32 objtype, unsigned short maxamount ) const;
    Item* find_objtype_noninuse( u32 objtype ) const;

	virtual void for_each_item( void (*f)(Item* item, void* a), void* arg );

    virtual bool script_isa( unsigned isatype ) const;
    
    
    unsigned int find_sumof_objtype_noninuse( u32 objtype ) const;
	unsigned int find_sumof_objtype_noninuse( u32 objtype, u32 amtToGet, Contents& saveItemsTo, int flags ) const;
	void consume_sumof_objtype_noninuse( u32 objtype, unsigned int amount );

	UContainer* find_container( u32 serial ) const;

		// remove(): tells what subcontainer used to hold the item
		//			 points item->container to NULL on removal			
	Item *remove( u32 serial, UContainer * * found_in = NULL ); 
    void remove( Item* item ); // item must be in this container
    void remove( iterator itr );

	enum MoveType {
		MT_PLAYER       = 0, // default
		MT_CORE_MOVED   = 1,
		MT_CORE_CREATED = 2
	};

    // TODO: rework these too.
    bool check_can_remove_script( Character* chr, Item* item, MoveType move = MT_PLAYER );
    void on_remove( Character* chr, Item* item, MoveType move = MT_PLAYER );
    virtual void readProperties( ConfigElem& elem );

    bool can_insert_increase_stack( Character* mob, MoveType move, Item* existing_item, unsigned short amt_to_add, Item* adding_item );
    bool can_insert_add_item( Character* mob, MoveType move, Item* new_item );
   
    void on_insert_increase_stack( Character* mob, MoveType move, Item* existing_item, unsigned short amt_added );
    void on_insert_add_item( Character* mob, MoveType move, Item* new_item );

	virtual Character* get_chr_owner(){return NULL;};
		// system_find: bypasses all locks, etc. 
	Item *system_find( u32 serial ) const;

    u16 gump() const;
    void get_random_location( u16* px, u16* py ) const;
    bool is_legal_posn( const Item* item, u16 x, u16 y ) const;
    void enumerate_contents( class ObjArray* arr, int flags );
    void extract( Contents& cnt );
    
    bool can_swap( const UContainer& cont ) const;
    void swap( UContainer& cont );

    const ContainerDesc& descriptor() const;

    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;
protected:
    Contents contents_;
    
    const ContainerDesc& desc;

    u16 held_weight_; // in stones
    unsigned int held_item_count_;

    Item *operator[](unsigned idx) const;

    Item *find( u32 serial, iterator& where_in_container ); // return the position in the array where it was found.

	// sticky places that currently need to know the internals: 
    friend class UContainerIterator;
//	friend class Character; // uses the [] operator for quick layer access.
    friend bool send_vendorwindow_contents( Client* client, UContainer* for_sale, bool send_aos_tooltip ); // also []
    friend bool send_vendorsell( Client* client, NPC* merchant, UContainer* sellfrom, bool send_aos_tooltip );
    friend Item* Item::create( const ItemDesc& itemdesc, u32 serial );
    friend class WornItemsContainer;
	//friend class Item;

protected:
	explicit UContainer( const ContainerDesc& descriptor );
	//DAVE added this 11/17 so WornItemsContainer could pass up its class to UObject constructor
	UContainer(u32 objtype, UOBJ_CLASS pol_class);
    // uses Item::classname()
    virtual void printOn( std::ostream& os ) const;
    virtual void printSelfOn( std::ostream& os ) const;
    void printContents( std::ostream& os ) const;
};

inline unsigned UContainer::count() const
{ 
	return contents_.size(); 
}


inline const ContainerDesc& UContainer::descriptor() const
{
    return desc;
}

inline Item *UContainer::operator[](unsigned idx) const
{
	return ITEM_ELEM_PTR( contents_[idx] );
}

class ConfigElem;


// Corpses must NEVER EVER be movable.
// They can decay even if they are immobile.
class UCorpse : public UContainer
{
    typedef UContainer base;
public:
    virtual u16 get_senditem_amount() const;
    u16 corpsetype;
    bool take_contents_to_grave;
	u32	ownerserial; // NPCs get deleted on death, so serial is used.
	Item* GetItemOnLayer( unsigned idx ) const;
	void PutItemOnLayer( Item* item );
	void RemoveItemFromLayer( Item* item );
protected:
    explicit UCorpse( const ContainerDesc& desc );
    virtual void spill_contents( UMulti* supporting_multi );
    virtual void printProperties( std::ostream& os ) const;
    virtual void readProperties( ConfigElem& elem );
    friend Item* Item::create( const ItemDesc& itemdesc, u32 serial );
    //virtual class BObjectImp* script_member( const char *membername );
    virtual BObjectImp* get_script_member( const char *membername ) const;
    virtual BObjectImp* get_script_member_id( const int id ) const; ///id test
    //virtual BObjectImp* set_script_member( const char *membername, const std::string& value );
    //virtual BObjectImp* set_script_member( const char *membername, int value );
    virtual bool script_isa( unsigned isatype ) const;
	Contents layer_list_;
};

inline Item *UCorpse::GetItemOnLayer(unsigned idx) const
{
	return ITEM_ELEM_PTR( layer_list_[idx] );
}

class WornItemsContainer : public UContainer
{
public:
	WornItemsContainer();
	//explicit WornItemsContainer(u16 objtype);

    virtual BObjectImp* make_ref();
	virtual Character* get_chr_owner(){return chr_owner;};
	Character* chr_owner;

    virtual UObject* owner();
    virtual const UObject* owner() const;
    virtual UObject* self_as_owner();
    virtual const UObject* self_as_owner() const;

	virtual void for_each_item( void (*f)(Item* item, void* a), void* arg );
    void SetInitialSize();

    Item* GetItemOnLayer( unsigned idx ) const;
    void PutItemOnLayer( Item* item );
    void RemoveItemFromLayer( Item* item );

    virtual bool saveonexit() const;
    virtual void saveonexit( bool newvalue );

	void print( std::ostream& ofs_pc, std::ostream& ofs_equip ) const;
};

inline Item *WornItemsContainer::GetItemOnLayer(unsigned idx) const
{
	return ITEM_ELEM_PTR( contents_[idx] );
}

#endif
