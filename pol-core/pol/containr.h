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
#include "item/item.h"

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


namespace Pol {
  namespace Clib {
	class ConfigElem;
  }
  namespace Mobile {
	class Character;
  }
  namespace Items {
	class ContainerDesc;
  }
  namespace Module {
    bool send_vendorwindow_contents( Network::Client* client, Core::UContainer* for_sale, bool send_aos_tooltip );
  }
  namespace Core {

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
      virtual size_t estimatedSize( ) const;

	  virtual void destroy();
	  void destroy_contents();
	  typedef std::vector<Items::Item*> Contents;
	  typedef Contents::iterator iterator;
	  typedef Contents::const_iterator const_iterator;

	  virtual void builtin_on_use( Network::Client *client );

	  // can_add(): doesn't look for matching stacks to add to.
	  virtual bool can_add( const Items::Item& item ) const;
	  virtual bool can_add( unsigned short more_weight ) const;

	  virtual void spill_contents( Multi::UMulti* supporting_multi );

	  virtual void add( Items::Item *item ); // NOTE: points item->container to self on insertion
	  void add_at_random_location( Items::Item* item );
	  unsigned count() const;

	  virtual unsigned int item_count() const;
	  virtual unsigned int weight() const;

	  bool can_add_bulk( int tli_diff, int item_count_diff, int weight_diff ) const;
	  virtual void add_bulk( int item_count_delta, int weight_delta );
	  void add_bulk( const Items::Item* item );
	  void remove_bulk( const Items::Item* item );

	  virtual bool can_add_to_slot( u8& slotIndex );
	  bool is_slot_empty( u8& slotIndex );
	  bool find_empty_slot( u8& slotIndex );

	  Items::Item* find( u32 serial ) const;
	  Items::Item* find_toplevel( u32 serial ) const;
	  Items::Item* find_toplevel_objtype( u32 objtype ) const;
	  Items::Item* find_toplevel_objtype( u32 objtype, unsigned short maxamount ) const;
	  Items::Item* find_toplevel_polclass( unsigned int polclass ) const;
	  Items::Item* find_addable_stack( const Items::Item* adding_item ) const;

	  Items::Item* find_toplevel_objtype_noninuse( u32 objtype ) const;
	  Items::Item* find_toplevel_objtype_noninuse( u32 objtype, unsigned short maxamount ) const;
	  Items::Item* find_objtype_noninuse( u32 objtype ) const;

	  virtual void for_each_item( void( *f )( Item* item, void* a ), void* arg );

	  virtual bool script_isa( unsigned isatype ) const;
	  virtual Items::Item* clone( ) const;

	  unsigned int find_sumof_objtype_noninuse( u32 objtype ) const;
	  unsigned int find_sumof_objtype_noninuse( u32 objtype, u32 amtToGet, Contents& saveItemsTo, int flags ) const;
	  void consume_sumof_objtype_noninuse( u32 objtype, unsigned int amount );

	  UContainer* find_container( u32 serial ) const;

	  // remove(): tells what subcontainer used to hold the item
	  //			 points item->container to NULL on removal			
	  Items::Item *remove( u32 serial, UContainer * * found_in = NULL );
	  void remove( Items::Item* item ); // item must be in this container
	  void remove( iterator itr );

	  enum MoveType
	  {
		MT_PLAYER = 0, // default
		MT_CORE_MOVED = 1,
		MT_CORE_CREATED = 2
	  };

	  // TODO: rework these too.
	  bool check_can_remove_script( Mobile::Character* chr, Items::Item* item, MoveType move = MT_PLAYER );
	  void on_remove( Mobile::Character* chr, Items::Item* item, MoveType move = MT_PLAYER );
	  virtual void printProperties( Clib::StreamWriter& sw ) const;
	  virtual void readProperties( Clib::ConfigElem& elem );
	  virtual Bscript::BObjectImp* get_script_member( const char* membername ) const;
	  virtual Bscript::BObjectImp* get_script_member_id( const int id ) const; ///id test
	  virtual Bscript::BObjectImp* set_script_member( const char* membername, int value );
	  virtual Bscript::BObjectImp* set_script_member_id( const int id, int value ); //id test

	  bool can_insert_increase_stack( Mobile::Character* mob, MoveType move, Items::Item* existing_item, unsigned short amt_to_add, Items::Item* adding_item );
	  bool can_insert_add_item( Mobile::Character* mob, MoveType move, Items::Item* new_item );

	  void on_insert_increase_stack( Mobile::Character* mob, MoveType move, Items::Item* existing_item, unsigned short amt_added );
	  void on_insert_add_item( Mobile::Character* mob, MoveType move, Items::Item* new_item );

	  virtual Mobile::Character* get_chr_owner() { return NULL; };
	  // system_find: bypasses all locks, etc. 
	  Items::Item *system_find( u32 serial ) const;

	  u16 gump() const;
	  void get_random_location( u16* px, u16* py ) const;
	  bool is_legal_posn( const Items::Item* item, u16 x, u16 y ) const;
	  void enumerate_contents( Bscript::ObjArray* arr, int flags );
	  void extract( Contents& cnt );

	  bool can_swap( const UContainer& cont ) const;
	  void swap( UContainer& cont );

	  const Items::ContainerDesc& descriptor() const;

	  iterator begin();
	  const_iterator begin() const;
	  iterator end();
	  const_iterator end() const;

	  // Add max_items(), max_weight() and max_slots() functions to make the code more clear
	  unsigned short max_items() const;
	  unsigned short max_weight() const;
	  u8 max_slots() const;

	protected:
	  Contents contents_;

	  const Items::ContainerDesc& desc;

	  u16 held_weight_; // in stones
	  unsigned int held_item_count_;

	  Items::Item *operator[]( unsigned idx ) const;

	  Items::Item *find( u32 serial, iterator& where_in_container ); // return the position in the array where it was found.

	  // sticky places that currently need to know the internals: 
	  friend class UContainerIterator;
	  //	friend class Character; // uses the [] operator for quick layer access.
	  friend bool Module::send_vendorwindow_contents( Network::Client* client, UContainer* for_sale, bool send_aos_tooltip ); // also []
	  friend bool send_vendorsell( Network::Client* client, NPC* merchant, UContainer* sellfrom, bool send_aos_tooltip );
	  friend Items::Item* Items::Item::create( const Items::ItemDesc& itemdesc, u32 serial );
	  friend class WornItemsContainer;
	  //friend class Item;

	protected:
	  explicit UContainer( const Items::ContainerDesc& descriptor );
	  //DAVE added this 11/17 so WornItemsContainer could pass up its class to UObject constructor
	  UContainer( u32 objtype, UOBJ_CLASS pol_class );
	  // uses Items::classname()
	  virtual void printOn( Clib::StreamWriter& sw ) const;
	  virtual void printSelfOn( Clib::StreamWriter& sw ) const;
	  void printContents( Clib::StreamWriter& sw ) const;
	};

	inline unsigned UContainer::count() const
	{
	  return static_cast<unsigned>( contents_.size() );
	}


	inline const Items::ContainerDesc& UContainer::descriptor() const
	{
	  return desc;
	}

	inline Items::Item *UContainer::operator[]( unsigned idx ) const
	{
	  return ITEM_ELEM_PTR( contents_[idx] );
	}

	// Corpses must NEVER EVER be movable.
	// They can decay even if they are immobile.
	class UCorpse : public UContainer
	{
	  typedef UContainer base;
	public:
      virtual ~UCorpse() {};
      virtual size_t estimatedSize( ) const;
	  virtual u16 get_senditem_amount() const;
      
      virtual void add (Item *item);

	  u16 corpsetype;
	  bool take_contents_to_grave;
	  u32	ownerserial; // NPCs get deleted on death, so serial is used.
	  Items::Item* GetItemOnLayer( unsigned idx ) const;
	  void RemoveItemFromLayer( Items::Item* item );
	protected:
        void PutItemOnLayer(Items::Item* item);

	  explicit UCorpse( const Items::ContainerDesc& desc );
	  virtual void spill_contents( Multi::UMulti* supporting_multi );
	  virtual void printProperties( Clib::StreamWriter& sw ) const;
	  virtual void readProperties( Clib::ConfigElem& elem );
	  friend Items::Item* Items::Item::create( const Items::ItemDesc& itemdesc, u32 serial );
	  //virtual Bscript::BObjectImp* script_member( const char *membername );
	  virtual Bscript::BObjectImp* get_script_member( const char *membername ) const;
	  virtual Bscript::BObjectImp* get_script_member_id( const int id ) const; ///id test
	  //virtual Bscript::BObjectImp* set_script_member( const char *membername, const std::string& value );
	  //virtual Bscript::BObjectImp* set_script_member( const char *membername, int value );
	  virtual bool script_isa( unsigned isatype ) const;
	  Contents layer_list_;
	};

	inline Items::Item *UCorpse::GetItemOnLayer( unsigned idx ) const
	{
		// Checks if the requested layer is valid
		if (Items::valid_equip_layer(idx))
			return ITEM_ELEM_PTR( layer_list_[idx] );
		
		return EMPTY_ELEM;
	}

	class WornItemsContainer : public UContainer
	{
      typedef UContainer base;
	public:
	  WornItemsContainer();
	  //explicit WornItemsContainer(u16 objtype);
      virtual ~WornItemsContainer() {};
      virtual size_t estimatedSize( ) const;

	  virtual Bscript::BObjectImp* make_ref();
	  virtual Mobile::Character* get_chr_owner() { return chr_owner; };
	  Mobile::Character* chr_owner;

	  virtual UObject* owner();
	  virtual const UObject* owner() const;
	  virtual UObject* self_as_owner();
	  virtual const UObject* self_as_owner() const;

	  virtual void for_each_item( void( *f )( Items::Item* item, void* a ), void* arg );

	  Items::Item* GetItemOnLayer( unsigned idx ) const;
	  void PutItemOnLayer( Item* item );
	  void RemoveItemFromLayer( Item* item );

	  virtual bool saveonexit() const;
	  virtual void saveonexit( bool newvalue );

	  void print( Clib::StreamWriter& sw_pc, Clib::StreamWriter& sw_equip ) const;
	};

	inline Items::Item *WornItemsContainer::GetItemOnLayer( unsigned idx ) const
	{
		if (Items::valid_equip_layer(idx))
		  return ITEM_ELEM_PTR( contents_[idx] );

		return NULL;
	}
  }
}
#endif
