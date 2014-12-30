/*
History
=======
10/22/2008 Luth:      on_insert_add_item() on_insert_script now called with all appropriate parameters
2008/12/17 MuadDib:   unequipping item now resets item->layer to 0
2009/07/20 MuadDib:   find_addable_stack() now runs correct check to see if can add to an existing stack.
2009/08/07 MuadDib:   Added Corpse checks in on_remove and on_insert_add to add to corpse layers for equips.
                      Create vector for item storage called layer_list_ for corpses, to work as storage to show
					  items listed as "equipped". Also created necessary functions to make use of this like
					  WornItem Container has.
2009/09/03 MuadDib:   Relocation of multi related cpp/h
2009/11/12 Turley:    Changed "can add"-functions to only check weight recursive
2009/11/17 Turley:    Fixed problem with sending remove object packet
2011/11/12 Tomi:	  added extobj.wornitems_container

Notes
=======

*/

#include "containr.h"

#include "../bscript/objmembers.h"

#include "../clib/cfgfile.h"
#include "../clib/cfgelem.h"
#include "../clib/logfacility.h"
#include "../clib/streamsaver.h"
#include "../clib/passert.h"
#include "../clib/random.h"
#include "../clib/stlutil.h"

#include "network/client.h"
#include "mobile/charactr.h"
#include "core.h"
#include "extobj.h"
#include "item/equipmnt.h"
#include "item/itemdesc.h"
#include "multi/multi.h"
#include "profile.h"
#include "statmsg.h"
#include "ucfg.h"
#include "ufunc.h"
#include "umanip.h"
#include "uvars.h"
#include "uworld.h"
#include "objtype.h"
#include "ufunc.h"
#include "sockio.h"
#include "scrsched.h"
#include "uoscrobj.h"

#include <climits>
#include <cstddef>


namespace Pol {
  namespace Core {
	UContainer::UContainer( const Items::ContainerDesc& descriptor ) :
	  ULockable( descriptor, CLASS_CONTAINER ),
	  desc( Items::find_container_desc( objtype_ ) ), // NOTE still grabs the permanent descriptor.
	  held_weight_( 0 ),
	  held_item_count_( 0 )
	{}

	UContainer::~UContainer()
	{
	  passert_always( contents_.empty() );
	}

    size_t UContainer::estimatedSize() const
    {
      size_t size = base::estimatedSize()
        + sizeof(u16)/*held_weight_*/
        +sizeof(unsigned int)/*held_item_count_*/
        // no estimateSize here element is in objhash
        +3 * sizeof( Items::Item** ) + contents_.capacity() * sizeof( Items::Item* );
      return size;
    }

	void UContainer::destroy_contents()
	{
	  while ( !contents_.empty() )
	  {
		Contents::value_type item = contents_.back();
		if ( ITEM_ELEM_PTR( item ) != NULL )  // this is really only for wornitems.
		{
		  item->container = NULL;
		  item->destroy();
		}
		contents_.pop_back();
	  }
	}


	void UContainer::destroy()
	{
	  destroy_contents();
	  base::destroy();
	}
	// Consider: writing an "item count" property.  On read,
	// recursively read items (eliminate a lot of searching)

	void UContainer::printOn( Clib::StreamWriter& sw ) const
	{
	  base::printOn( sw );
	  printContents( sw );
	}

    void UContainer::printSelfOn( Clib::StreamWriter& sw ) const
	{
	  base::printOn( sw );
	}

    void UContainer::printContents( Clib::StreamWriter& sw ) const
	{
	  for ( const auto& item : contents_ )
	  {
		if ( item != NULL )
		{
		  if ( item->itemdesc().save_on_exit && item->saveonexit() )
		  {
			sw << *item;
			item->clear_dirty();
		  }
		}
	  }
	}

	bool UContainer::can_add_bulk( int tli_diff, int item_count_diff, int weight_diff ) const
	{
	  if ( gamestate.gflag_enforce_container_limits )
	  {
		if ( gamestate.ssopt.use_slot_index )
		{
		  if ( contents_.size() + tli_diff >= MAX_SLOTS )
			return false;

		  if ( contents_.size() + tli_diff >= max_slots() )
			return false;
		}

		if ( contents_.size() + tli_diff >= MAX_CONTAINER_ITEMS )
		  return false;

		if ( weight() + weight_diff > USHRT_MAX /* gcc...std::numeric_limits<unsigned short>::max()*/ )
		  return false;

		if ( held_weight_ + weight_diff > max_weight() )
		  return false;

		if ( held_item_count_ + item_count_diff > max_items() )
		  return false;

		if ( container != NULL )
		  return container->can_add_bulk( 0, 0, weight_diff );
		else
		  return true;
	  }
	  else
	  {
		if ( gamestate.ssopt.use_slot_index )
		{
		  return ( ( contents_.size() < MAX_CONTAINER_ITEMS ) && ( contents_.size() < MAX_SLOTS ) );
		}
		else
		{
		  return ( contents_.size() < MAX_CONTAINER_ITEMS );
		}
	  }
	}

	bool UContainer::can_add( const Items::Item& item ) const
	{
	  return can_add_bulk( 1, 1, item.weight() );
	}

	bool UContainer::can_add( unsigned short more_weight ) const
	{
	  return can_add_bulk( 0, 0, more_weight );
	}

	bool UContainer::can_add_to_slot( u8& slotIndex )
	{
	  if ( gamestate.ssopt.use_slot_index )
	  {
		if ( slotIndex > max_slots() )
		  return false;

		if ( is_slot_empty( slotIndex ) )
		  return true;

		if ( find_empty_slot( slotIndex ) )
		  return true;
	  }
	  return true;
	}

	void UContainer::add( Items::Item *item )
	{
	  // passert( can_add( *item ) );

	  INC_PROFILEVAR( container_adds );
	  item->realm = realm;
	  item->container = this;
	  item->set_dirty();
	  contents_.push_back( Contents::value_type( item ) );

	  add_bulk( item );
	}
	void UContainer::add_bulk( const Items::Item* item )
	{
	  add_bulk( 1, item->weight() );
	}
	void UContainer::remove_bulk( const Items::Item* item )
	{
	  add_bulk( -static_cast<int>( 1 ), -static_cast<int>( item->weight() ) );
	}

	void UContainer::add_bulk( int item_count_delta, int weight_delta )
	{
	  held_item_count_ += item_count_delta;

	  // passert( !gamestate.gflag_enforce_container_limits || (held_weight_ + weight_delta <= MAX_WEIGHT) );

	  held_weight_ += static_cast<unsigned short>( weight_delta );
	  if ( container != NULL )
	  {
		container->add_bulk( 0, weight_delta );
	  }
	}


	unsigned int UContainer::weight() const
	{
	  return Items::Item::weight() + held_weight_;
	}

	unsigned int UContainer::item_count() const
	{
	  return Items::Item::item_count() + held_item_count_;
	}

	bool UContainer::is_slot_empty( u8& slotIndex )
	{
	  if ( held_item_count_ == 0 )
		return true;

      for ( const auto &item : contents_ )
      {
        if ( item == nullptr )
          continue;
        if ( item->slot_index() == slotIndex )
          return false;
      }
	  return true;
	}

	bool UContainer::find_empty_slot( u8& slotIndex )
	{
	  if ( held_item_count_ >= max_items() )
		return false;

	  if ( held_item_count_ >= max_slots() )
		return false;

	  bool slot_check = false;

	  for ( u8 slot_location = 1; slot_location <= max_items(); ++slot_location )
	  {
        for ( const auto &item : contents_ )
		{
          if ( item == nullptr )
            continue;
		  if ( item->slot_index() == slot_location )
			slot_check = true;
		  if ( !slot_check )
		  {
			break;
		  }
		}
		if ( !slot_check )
		{
		  slotIndex = slot_location;
		  return true;
		}
		else
		{
		  slot_check = false;
		}
	  }
	  return false;
	}

	void UContainer::add_at_random_location( Items::Item* item )
	{
	  u16 rx, ry;
	  get_random_location( &rx, &ry );

	  item->x = rx;
	  item->y = ry;
	  item->z = 0;

	  add( item );
	}

    void UContainer::enumerate_contents( Bscript::ObjArray* arr, int flags )
    {
      for ( auto &item : contents_ )
      {
        if ( item ) //dave 1/1/03, wornitemscontainer can have null items!
        {
          arr->addElement( new Module::EItemRefObjImp( item ) );
          // Austin 9-15-2006, added flag to not enumerate sub-containers.
          if ( !( flags & ENUMERATE_ROOT_ONLY ) && ( item->isa( CLASS_CONTAINER ) ) ) // FIXME check locks
          {
            UContainer* cont = static_cast<UContainer*>( item );
            if ( !cont->locked_ || ( flags & ENUMERATE_IGNORE_LOCKED ) )
              cont->enumerate_contents( arr, flags );
          }
        }
      }
    }

	void UContainer::extract( Contents& cnt )
	{
	  contents_.swap( cnt );
	  add_bulk( -static_cast<int>( held_item_count_ ), -static_cast<int>( held_weight_ ) );
	}

	bool UContainer::can_swap( const UContainer& cont ) const
	{
	  int weight_diff0 = cont.weight() - weight();
	  int item_count_diff0 = cont.item_count() - item_count();

	  int weight_diff1 = -weight_diff0;
	  int item_count_diff1 = -item_count_diff0;

	  return ( can_add_bulk( 0, item_count_diff0, weight_diff0 ) &&
			   cont.can_add_bulk( 0, item_count_diff1, weight_diff1 ) );
	}

	void UContainer::swap( UContainer& cont )
	{
	  assert( can_swap( cont ) );

	  int weight_diff = cont.weight() - weight();
	  int item_count_diff = cont.item_count() - item_count();

	  add_bulk( item_count_diff, weight_diff );
	  cont.add_bulk( -item_count_diff, -weight_diff );

	  contents_.swap( cont.contents_ );
	}

	Items::Item* UContainer::find_toplevel_polclass( unsigned int polclass ) const
	{
      for ( auto &item : contents_ )
	  {
		if ( item && ( item->script_isa( polclass ) ) )
		  return item;
	  }
	  return NULL;
	}

    Items::Item* UContainer::find_toplevel_objtype( u32 objtype ) const
	{
      for ( auto &item : contents_ )
	  {
		if ( item && ( item->objtype_ == objtype ) )
		  return item;
	  }
	  return NULL;
	}
    Items::Item* UContainer::find_toplevel_objtype_noninuse( u32 objtype ) const
	{
      for ( auto &item : contents_ )
	  {
		if ( item && ( item->objtype_ == objtype ) && !item->inuse() )
		  return item;
	  }
	  return NULL;
	}

	Items::Item* UContainer::find_toplevel_objtype( u32 objtype, unsigned short maxamount ) const
	{
      for ( auto &item : contents_ )
	  {
		if ( item && ( item->objtype_ == objtype ) && ( item->getamount() <= maxamount ) )
		  return item;
	  }
	  return NULL;
	}
	Items::Item* UContainer::find_toplevel_objtype_noninuse( u32 objtype, unsigned short maxamount ) const
	{
      for ( auto &item : contents_ )
	  {
		if ( item && ( item->objtype_ == objtype ) && ( item->getamount() <= maxamount ) && !item->inuse() )
		  return item;
	  }
	  return NULL;
	}

	Items::Item* UContainer::find_addable_stack( const Items::Item* adding_item ) const
	{
	  unsigned short maxamount = adding_item->itemdesc().stack_limit - adding_item->getamount();

	  if ( maxamount > 0 )
	  {
        for ( auto &item : contents_ )
		{
		  if ( item && item->can_add_to_self( *adding_item ) )
		  {
			return item;
		  }
		}
	  }

	  return NULL;
	}


	Items::Item* UContainer::find_objtype_noninuse( u32 objtype ) const
	{
	  Items::Item* _item = find_toplevel_objtype_noninuse( objtype );
	  if ( _item != NULL )
		return _item;

      for ( const auto& item : contents_ )
	  {
		if ( item &&
			 item->isa( UObject::CLASS_CONTAINER ) &&
			 !item->inuse() )
		{
		  UContainer* cont = static_cast<UContainer*>( item );
		  if ( !cont->locked_ )
		  {
			auto child_item = cont->find_objtype_noninuse( objtype );
            if ( child_item != NULL )
              return child_item;
		  }
		}
	  }
	  return NULL;
	}

	unsigned int UContainer::find_sumof_objtype_noninuse( u32 objtype ) const
	{
	  unsigned int amt = 0;

      for ( auto &item : contents_ )
	  {
		if ( item &&
			 !item->inuse() )
		{
		  if ( item->objtype_ == objtype )
			amt += item->getamount();

		  if ( item->isa( UObject::CLASS_CONTAINER ) )
		  {
			UContainer* cont = static_cast<UContainer*>( item );
			if ( !cont->locked_ )
			{
			  amt += cont->find_sumof_objtype_noninuse( objtype );
			}
		  }
		}
	  }
	  return amt;
	}

	void UContainer::consume_sumof_objtype_noninuse( u32 objtype, unsigned int amount )
	{
	  while ( amount != 0 )
	  {
		Items::Item* item = find_objtype_noninuse( objtype );
		passert_always( item != NULL );

		unsigned short thisamt = item->getamount();
		if ( thisamt > amount )
		  thisamt = static_cast<unsigned short>( amount );
		subtract_amount_from_item( item, thisamt );
		amount -= thisamt;
	  }
	}

	Items::Item *UContainer::remove( u32 serial, UContainer** found_in )
	{
	  Items::Item* item;
	  iterator itr;

	  item = find( serial, itr );
	  if ( item != NULL )
	  {
		if ( found_in != NULL )
		  *found_in = item->container;

		item->container->remove( itr );
	  }
	  return item;
	}

	void UContainer::remove( Items::Item* item )
	{
	  if ( item->container != this )
	  {
        POLLOG.Format( "UContainer::remove(Item*), serial=0x{:X}, item=0x{:X}, item->cont=0x{:X}\n" )
          << serial << item->serial << item->container->serial;
		passert_always( item->container == this );
		int* p = 0;
		*p = 6;
	  }

	  iterator itr = std::find(contents_.begin(),contents_.end(), item );
	  passert_always( itr != contents_.end() );

	  //DAVE added this 11/17. refresh owner's weight on delete
	  Mobile::Character* chr_owner = item->GetCharacterOwner();

	  if ( chr_owner != NULL && chr_owner->client != NULL )
	  {
		send_remove_object_to_inrange( item );
	  }
	  remove( itr );

	  if ( chr_owner != NULL && chr_owner->client != NULL )
	  {
		send_full_statmsg( chr_owner->client, chr_owner );
		//chr_owner->refresh_ar();
	  }
	}

    // This is the only function that actually removes the item from the container. This could be
    // inlined into UContainer::remove(Item *item) if we change UContainer::remove(u32 serial, ...)
    // to call remove(*itr) instead of remove(itr). The code would be cleaner at the cost of an extra find.
    // As I don't have time to test if that would make a big difference, I leave this note for the future.
    // I wish you luck. (Nando, 2014/10/30)
	void UContainer::remove( iterator itr )
	{
	  INC_PROFILEVAR( container_removes );
	  Items::Item* item = GET_ITEM_PTR( itr );
	  contents_.erase( itr );
	  item->container = NULL;
      item->reset_slot();
	  item->set_dirty();
	  remove_bulk( item );
	}

	// FIXME this is depth-first.  Not sure I like that.
	UContainer *UContainer::find_container( u32 serial ) const
	{
      for ( auto &item : contents_ )
	  {
		if ( item &&
			 item->isa( UObject::CLASS_CONTAINER ) )
		{
		  UContainer *cont = static_cast<UContainer *>( item );
		  if ( cont->serial == serial )
			return cont;
		  cont = cont->find_container( serial );
		  if ( cont != NULL )
			return cont;
		}
	  }
	  return NULL;
	}

	Items::Item *UContainer::find( u32 serial, iterator& where_in_container )
	{
	  for ( iterator itr = contents_.begin(); itr != contents_.end(); ++itr )
	  {
		Items::Item *item = GET_ITEM_PTR( itr );
		passert( item != NULL );
		if ( item != NULL )
		{
		  if ( item->serial == serial )
		  {
			where_in_container = itr;
			return item;
		  }
		  if ( item->isa( UObject::CLASS_CONTAINER ) )
		  {
			UContainer* cont = static_cast<UContainer*>( item );
			if ( !cont->locked_ )
			{
			  item = cont->find( serial, where_in_container );
			  if ( item != NULL )
				return item;
			}
		  }
		}
	  }
	  return NULL;
	}

	Items::Item *UContainer::find( u32 serial ) const
	{
      for ( const auto &item : contents_ )
	  {
		passert( item != NULL );
		if ( item != NULL )
		{
		  if ( item->serial == serial )
			return item;

		  if ( item->isa( UObject::CLASS_CONTAINER ) )
		  {
			UContainer* cont = static_cast<UContainer*>( item );
			if ( !cont->locked_ )
			{
			  auto child_item = cont->find( serial );
              if ( child_item != NULL )
                return child_item;
			}
		  }
		}
	  }
	  return NULL;
	}

	Items::Item *UContainer::find_toplevel( u32 serial ) const
	{
      for ( auto &item : contents_ )
	  {
		passert( item != NULL );
		if ( item != NULL )
		{
		  if ( item->serial == serial )
			return item;

		}
	  }
	  return NULL;
	}

	void UContainer::for_each_item( void( *f )( Items::Item* item, void* a ), void* arg )
	{
      for ( auto &item : contents_ )
	  {

		if ( item->isa( UObject::CLASS_CONTAINER ) )
		{
		  UContainer* cont = static_cast<UContainer*>( item );
		  cont->for_each_item( f, arg );
		}
		( *f )( item, arg );
	  }
	}

	UContainer::const_iterator UContainer::begin() const
	{
	  return contents_.begin();
	}
	UContainer::const_iterator UContainer::end() const
	{
	  return contents_.end();
	}

	UContainer::iterator UContainer::begin()
	{
	  return contents_.begin();
	}
	UContainer::iterator UContainer::end()
	{
	  return contents_.end();
	}

    void UContainer::builtin_on_use( Network::Client *client )
	{
	  client->pause();

	  if ( !locked_ )
	  {
		send_open_gump( client, *this );
		send_container_contents( client, *this );
	  }
	  else
	  {
		send_sysmessage( client, "That is locked." );
	  }

	  client->restart();
	}

	u16 UContainer::gump() const
	{
	  return desc.gump;
	}

	void UContainer::get_random_location( u16* px, u16* py ) const
	{
	  if ( desc.minx < desc.maxx )
	  {
		*px = desc.minx + static_cast<u16>( Clib::random_int( desc.maxx - desc.minx-1 ) );
	  }
	  else
	  {
		*px = desc.minx;
	  }

	  if ( desc.miny < desc.maxy )
	  {
        *py = desc.miny + static_cast<u16>( Clib::random_int( desc.maxy - desc.miny -1) );
	  }
	  else
	  {
		*py = desc.miny;
	  }
	}

	bool UContainer::is_legal_posn( const Items::Item* /*item*/, u16 x, u16 y ) const
	{
	  return ( x >= desc.minx && x <= desc.maxx &&
			   y >= desc.miny && y <= desc.maxy );
	}

	void UContainer::spill_contents( Multi::UMulti* multi )
	{
	  passert( container == NULL );
	  if ( !locked_ )
	  {
		while ( !contents_.empty() )
		{
		  Items::Item* item = ITEM_ELEM_PTR( contents_.back() );
		  if ( item->movable() )
		  {
			contents_.pop_back();
			item->set_dirty();
			item->x = x;
			item->y = y;
			item->z = z;
			item->container = NULL;
			add_item_to_world( item );
			move_item( item, x, y, z, NULL );
			if ( multi )
			  multi->register_object( item );
			item->layer = 0;
		  }
		  else
		  {
			destroy_item( item );
		  }
		}
	  }
	}


	void UContainer::on_remove( Mobile::Character* chr, Items::Item* item, MoveType move )
	{
	 
	  if ( !desc.on_remove_script.empty() )
	  {
		// static code analysis indicates (C6211) that this might leak, but I can't use an auto_ptr<>
		// because of UninitObject::create() ... ideas? Nando - 2010-07-10
		Bscript::BObjectImp* chrParam = NULL;
		if ( chr )// consider: move this into make_mobileref
		  chrParam = new Module::ECharacterRefObjImp( chr );
		else
          chrParam = Bscript::UninitObject::create( );
		//Luth: 10/22/2008 - on_remove_script now called with all appropriate parameters
		call_script( desc.on_remove_script,
					 chrParam,
                     new Module::EItemRefObjImp( this ),
                     new Module::EItemRefObjImp( item ),
                     new Bscript::BLong( item->getamount( ) ),
                     new Bscript::BLong( move ) );
	  }
	}

	bool UContainer::can_insert_increase_stack( Mobile::Character* mob,
												MoveType movetype,
												Items::Item* existing_item,
												unsigned short amt_to_add,
												Items::Item* adding_item )
	{
	  if ( !desc.can_insert_script.empty() )
	  {
		return call_script( desc.can_insert_script,
                            mob ? mob->make_ref( ) : Bscript::UninitObject::create( ),
							make_ref(),
                            new Bscript::BLong( movetype ),
                            new Bscript::BLong( INSERT_INCREASE_STACK ),
                            adding_item ? adding_item->make_ref( ) : Bscript::UninitObject::create( ),
							existing_item->make_ref(),
                            new Bscript::BLong( amt_to_add ) );
	  }
	  else
	  {
		return true;
	  }
	}

	void UContainer::on_insert_increase_stack( Mobile::Character* mob,
											   MoveType movetype,
											   Items::Item* existing_item,
											   unsigned short amt_to_add )
	{
	  if ( !desc.on_insert_script.empty() )
	  {
		call_script( desc.on_insert_script,
                     mob ? mob->make_ref( ) : Bscript::UninitObject::create( ),
					 make_ref(),
                     new Bscript::BLong( movetype ),
                     new Bscript::BLong( INSERT_INCREASE_STACK ),
                     Bscript::UninitObject::create( ),
					 existing_item->make_ref(),
                     new Bscript::BLong( amt_to_add ) );
	  }
	}

	bool UContainer::can_insert_add_item( Mobile::Character* mob, MoveType movetype, Items::Item* new_item )
	{
	  if ( !desc.can_insert_script.empty() )
	  {
		return call_script( desc.can_insert_script,
                            mob ? mob->make_ref( ) : Bscript::UninitObject::create( ),
							make_ref(),
                            new Bscript::BLong( movetype ),
                            new Bscript::BLong( INSERT_ADD_ITEM ),
							new_item->make_ref() );
	  }
	  else
	  {
		return true;
	  }
	}

	void UContainer::on_insert_add_item( Mobile::Character* mob, MoveType movetype, Items::Item* new_item )
	{
	  if ( !desc.on_insert_script.empty() )
	  {
		Items::Item* existing_stack = find_addable_stack( new_item );

		call_script( desc.on_insert_script,
                     mob ? mob->make_ref( ) : Bscript::UninitObject::create( ),
					 make_ref(),
                     new Bscript::BLong( movetype ),
                     new Bscript::BLong( INSERT_ADD_ITEM ),
					 new_item->make_ref(),
                     existing_stack ? existing_stack->make_ref( ) : Bscript::UninitObject::create( ),
                     new Bscript::BLong( new_item->getamount( ) )
					 );
	  }
	}

	bool UContainer::check_can_remove_script( Mobile::Character* chr, Items::Item* item, MoveType move )
	{
	  if ( !desc.can_remove_script.empty() )
	  {
        Bscript::BObjectImp* chrParam = NULL;
		if ( chr != NULL ) // TODO: consider moving this into make_mobileref
		  chrParam = chr->make_ref();
		else
          chrParam = Bscript::UninitObject::create( );
		return call_script( desc.can_remove_script,
							chrParam,
							make_ref(),
							item->make_ref(),
                            new Bscript::BLong( move ) );
	  }
	  else
	  {
		return true;
	  }
	}


	void UContainer::printProperties( Clib::StreamWriter& sw ) const
	{
	  base::printProperties( sw );
      short max_items_mod_ = this->max_items_mod();
      short max_weight_mod_ = this->max_weight_mod();
      s8 max_slots_mod_ = this->max_slots_mod();

	  if ( max_items_mod_ )
		sw() << "\tMax_Items_mod\t" << max_items_mod_ << pf_endl;
	  if ( max_weight_mod_ )
		sw() << "\tMax_Weight_mod\t" << max_weight_mod_ << pf_endl;
	  if ( max_slots_mod_ )
		sw() << "\tMax_Slots_mod\t" << max_slots_mod_ << pf_endl;
	}

	void UContainer::readProperties( Clib::ConfigElem& elem )
	{
	  base::readProperties( elem );
      this->max_items_mod( static_cast<s16>( elem.remove_int( "MAX_ITEMS_MOD", 0 ) ));
      this->max_weight_mod( static_cast<s16>( elem.remove_int( "MAX_WEIGHT_MOD", 0 ) ));
      this->max_slots_mod( static_cast<s8>( elem.remove_int( "MAX_SLOTS_MOD", 0 ) ));
	}

	unsigned int UContainer::find_sumof_objtype_noninuse( u32 objtype, u32 amtToGet, Contents& saveItemsTo, int flags ) const
	{
	  unsigned int amt = 0;

      for ( auto &item : contents_ )
	  {
		if ( item &&
			 !item->inuse() )
		{
		  if ( item->objtype_ == objtype )
		  {
			saveItemsTo.push_back( item );
			amt += item->getamount();
		  }
		  if ( !( flags & FINDSUBSTANCE_ROOT_ONLY ) && ( item->isa( UObject::CLASS_CONTAINER ) ) )
		  {
			UContainer* cont = static_cast<UContainer*>( item );
			if ( !cont->locked_ || ( flags & FINDSUBSTANCE_IGNORE_LOCKED ) )
			{
			  amt += cont->find_sumof_objtype_noninuse( objtype, amtToGet - amt, saveItemsTo, flags );
			}
		  }
		}
		if ( !( flags & FINDSUBSTANCE_FIND_ALL ) )
		{
		  if ( amt >= amtToGet )
			return amt;
		}
	  }
	  return amt;
	}

	Items::Item* UContainer::clone() const
	{
	  UContainer* item = static_cast<UContainer*>( base::clone() );

      item->max_items_mod(this->max_items_mod());
      item->max_weight_mod(this->max_weight_mod());
      item->max_slots_mod(this->max_slots_mod());

	  return item;
	}

	unsigned short UContainer::max_items() const
	{
      int max_items = desc.max_items + max_items_mod();

	  if ( max_items < 1 )
		return 1;
	  else if ( max_items <= MAX_CONTAINER_ITEMS )
		return static_cast<u16>( max_items );
	  else
		return MAX_CONTAINER_ITEMS;
	}

	unsigned short UContainer::max_weight() const
	{
      int max_weight = desc.max_weight + max_weight_mod();

	  if ( max_weight < 1 )
		return USHRT_MAX;
	  else if ( max_weight <= USHRT_MAX )
		return static_cast<u16>( max_weight );
	  else
		return USHRT_MAX;
	}

	u8 UContainer::max_slots() const
	{
      short max_slots = desc.max_slots + max_slots_mod();

	  if ( max_slots < 0 )
		return 0;
	  else if ( max_slots <= MAX_SLOTS )
		return static_cast<u8>( max_slots );
	  else
		return MAX_SLOTS;
	}


}
}