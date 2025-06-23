/** @file
 *
 * @par History
 * - 10/22/2008 Luth:      on_insert_add_item() on_insert_script now called with all appropriate
 * parameters
 * - 2008/12/17 MuadDib:   unequipping item now resets item->layer to 0
 * - 2009/07/20 MuadDib:   find_addable_stack() now runs correct check to see if can add to an
 * existing stack.
 * - 2009/08/07 MuadDib:   Added Corpse checks in on_remove and on_insert_add to add to corpse
 * layers for equips.
 *                         Create vector for item storage called layer_list_ for corpses, to work as
 * storage to show
 *                         items listed as "equipped". Also created necessary functions to make use
 * of this like
 *                         WornItem Container has.
 * - 2009/09/03 MuadDib:   Relocation of multi related cpp/h
 * - 2009/11/12 Turley:    Changed "can add"-functions to only check weight recursive
 * - 2009/11/17 Turley:    Fixed problem with sending remove object packet
 * - 2011/11/12 Tomi:      added extobj.wornitems_container
 */


#include "containr.h"

#include <algorithm>
#include <assert.h>
#include <cstddef>

#include "../bscript/executor.h"
#include "../clib/cfgelem.h"
#include "../clib/clib.h"
#include "../clib/logfacility.h"
#include "../clib/passert.h"
#include "../clib/random.h"
#include "../clib/stlutil.h"
#include "../clib/streamsaver.h"
#include "../plib/uconst.h"
#include "globals/state.h"
#include "globals/uvars.h"
#include "item/itemdesc.h"
#include "mobile/charactr.h"
#include "network/client.h"
#include "profile.h"
#include "scrdef.h"
#include "scrsched.h"
#include "statmsg.h"
#include "syshookscript.h"
#include "ufunc.h"
#include "umanip.h"
#include "uobject.h"
#include "uoscrobj.h"
#include "uworld.h"


namespace Pol
{
namespace Core
{
UContainer::UContainer( const Items::ContainerDesc& id )
    : ULockable( id, UOBJ_CLASS::CLASS_CONTAINER ),
      desc( Items::find_container_desc( objtype_ ) ),  // NOTE still grabs the permanent descriptor.
      held_weight_( 0 ),
      held_item_count_( 0 )
{
  no_drop_exception( id.no_drop_exception );
  held_weight_multiplier( id.held_weight_multiplier );
}

UContainer::~UContainer()
{
  passert_always( contents_.empty() );
}

size_t UContainer::estimatedSize() const
{
  const size_t size = base::estimatedSize() + sizeof( u16 ) /*held_weight_*/
                      + sizeof( unsigned int )              /*held_item_count_*/
                      // no estimateSize here element is in objhash
                      + Clib::memsize( contents_ );
  return size;
}

void UContainer::destroy_contents()
{
  while ( !contents_.empty() )
  {
    Contents::value_type item = contents_.back();
    if ( item != nullptr )  // this is really only for wornitems.
    {
      item->container = nullptr;
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
    if ( item != nullptr )
    {
      if ( item->itemdesc().save_on_exit && item->saveonexit() )
      {
        item->printOn( sw );
        item->clear_dirty();
      }
    }
  }
}

bool UContainer::can_add_bulk( int tli_diff, int item_count_diff, int weight_diff ) const
{
  if ( stateManager.gflag_enforce_container_limits )
  {
    if ( settingsManager.ssopt.use_slot_index )
    {
      if ( contents_.size() + tli_diff >= MAX_SLOTS )
        return false;

      if ( contents_.size() + tli_diff >= max_slots() )
        return false;
    }

    if ( contents_.size() + tli_diff >= MAX_CONTAINER_ITEMS )
      return false;

    if ( weight() + weight_diff * held_weight_multiplier() > USHRT_MAX )
      return false;

    if ( ( held_weight_ + weight_diff ) * held_weight_multiplier() > max_weight() )
      return false;

    if ( held_item_count_ + item_count_diff > max_items() )
      return false;

    if ( container != nullptr )
    {
      int modded_diff =
          Clib::clamp_convert<int>( ( ( held_weight_ + weight_diff ) * held_weight_multiplier() ) -
                                    ( held_weight_ * held_weight_multiplier() ) );
      return container->can_add_bulk( 0, 0, modded_diff );
    }
    else
      return true;
  }
  else
  {
    if ( settingsManager.ssopt.use_slot_index )
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
  if ( settingsManager.ssopt.use_slot_index )
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

void UContainer::add( Items::Item* item )
{
  // passert( can_add( *item ) );
  if ( orphan() )
  {
    POLLOG_ERRORLN( "Trying to add item to orphan container!" );
    passert_always( 0 );  // TODO remove once found
  }
  INC_PROFILEVAR( container_adds );
  item->setposition( Pos4d( item->pos().xyz(), realm() ) );  // TODO POS realm should be a nullptr
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
  if ( item_count_delta == 0 && weight_delta == 0 )
    return;

  held_item_count_ += item_count_delta;

  // passert( !stateManager.gflag_enforce_container_limits || (held_weight_ + weight_delta <=
  // MAX_WEIGHT) );

  int oldweight = 0;
  if ( container != nullptr )
    oldweight = weight();
  held_weight_ = Clib::clamp_convert<u16>( held_weight_ + weight_delta );
  if ( container != nullptr )
    container->add_bulk( 0, weight() - oldweight );
}


unsigned int UContainer::weight() const
{
  return Clib::clamp_convert<u32>( Items::Item::weight() +
                                   held_weight_ * held_weight_multiplier() );
}

unsigned int UContainer::item_count() const
{
  return Items::Item::item_count() + held_item_count_;
}

bool UContainer::is_slot_empty( u8& slotIndex )
{
  if ( held_item_count_ == 0 )
    return true;

  for ( const auto& item : contents_ )
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
    for ( const auto& item : contents_ )
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
  item->setposition( Pos4d( get_random_location(), 0, item->realm() ) );  // TODO POS realm nullptr

  add( item );
}

void UContainer::enumerate_contents( Bscript::ObjArray* arr, int flags )
{
  for ( auto& item : contents_ )
  {
    if ( item )  // dave 1/1/03, wornitemscontainer can have null items!
    {
      arr->addElement( new Module::EItemRefObjImp( item ) );
      // Austin 9-15-2006, added flag to not enumerate sub-containers.
      if ( !( flags & ENUMERATE_ROOT_ONLY ) &&
           ( item->isa( UOBJ_CLASS::CLASS_CONTAINER ) ) )  // FIXME check locks
      {
        UContainer* cont = static_cast<UContainer*>( item );
        if ( !cont->locked() || ( flags & ENUMERATE_IGNORE_LOCKED ) )
          cont->enumerate_contents( arr, flags );
      }
    }
  }
}

void UContainer::extract( Contents& cnt )
{
  if ( orphan() )
  {
    POLLOG_ERRORLN( "Trying to add item to orphan container!" );
    passert_always( 0 );  // TODO remove once found
  }
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

  if ( orphan() )
  {
    POLLOG_ERRORLN( "Trying to add item to orphan container!" );
    passert_always( 0 );  // TODO remove once found
  }
  contents_.swap( cont.contents_ );
}

Items::Item* UContainer::find_toplevel_polclass( unsigned int polclass ) const
{
  for ( auto& item : contents_ )
  {
    if ( item && ( item->script_isa( polclass ) ) )
      return item;
  }
  return nullptr;
}

Items::Item* UContainer::find_toplevel_objtype( u32 objtype ) const
{
  for ( auto& item : contents_ )
  {
    if ( item && ( item->objtype_ == objtype ) )
      return item;
  }
  return nullptr;
}
Items::Item* UContainer::find_toplevel_objtype_noninuse( u32 objtype ) const
{
  for ( auto& item : contents_ )
  {
    if ( item && ( item->objtype_ == objtype ) && !item->inuse() )
      return item;
  }
  return nullptr;
}

Items::Item* UContainer::find_toplevel_objtype( u32 objtype, unsigned short maxamount ) const
{
  for ( auto& item : contents_ )
  {
    if ( item && ( item->objtype_ == objtype ) && ( item->getamount() <= maxamount ) )
      return item;
  }
  return nullptr;
}
Items::Item* UContainer::find_toplevel_objtype_noninuse( u32 objtype,
                                                         unsigned short maxamount ) const
{
  for ( auto& item : contents_ )
  {
    if ( item && ( item->objtype_ == objtype ) && ( item->getamount() <= maxamount ) &&
         !item->inuse() )
      return item;
  }
  return nullptr;
}

Items::Item* UContainer::find_addable_stack( const Items::Item* adding_item ) const
{
  if ( !adding_item->stackable() )
    return nullptr;

  unsigned short maxamount = adding_item->itemdesc().stack_limit - adding_item->getamount();

  if ( maxamount > 0 )
  {
    for ( auto& item : contents_ )
    {
      if ( item && item->can_add_to_self( *adding_item, false ) )
      {
        return item;
      }
    }
  }

  return nullptr;
}

Items::Item* UContainer::find_objtype( u32 objtype, int flags ) const
{
  Items::Item* _item = find_toplevel_objtype( objtype );
  if ( _item != nullptr )
    return _item;
  if ( !( flags & FINDOBJTYPE_ROOT_ONLY ) )
  {
    for ( const auto& item : contents_ )
    {
      if ( item && item->isa( UOBJ_CLASS::CLASS_CONTAINER ) )
      {
        UContainer* cont = static_cast<UContainer*>( item );
        if ( !cont->locked() || ( flags & FINDOBJTYPE_IGNORE_LOCKED ) )
        {
          auto child_item = cont->find_objtype( objtype, flags );
          if ( child_item != nullptr )
            return child_item;
        }
      }
    }
  }
  return nullptr;
}


Items::Item* UContainer::find_objtype_noninuse( u32 objtype ) const
{
  Items::Item* _item = find_toplevel_objtype_noninuse( objtype );
  if ( _item != nullptr )
    return _item;

  for ( const auto& item : contents_ )
  {
    if ( item && item->isa( UOBJ_CLASS::CLASS_CONTAINER ) && !item->inuse() )
    {
      UContainer* cont = static_cast<UContainer*>( item );
      if ( !cont->locked() )
      {
        auto child_item = cont->find_objtype_noninuse( objtype );
        if ( child_item != nullptr )
          return child_item;
      }
    }
  }
  return nullptr;
}

unsigned int UContainer::find_sumof_objtype_noninuse( u32 objtype ) const
{
  unsigned int amt = 0;

  for ( auto& item : contents_ )
  {
    if ( item && !item->inuse() )
    {
      if ( item->objtype_ == objtype )
        amt += item->getamount();

      if ( item->isa( UOBJ_CLASS::CLASS_CONTAINER ) )
      {
        UContainer* cont = static_cast<UContainer*>( item );
        if ( !cont->locked() )
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
    passert_always( item != nullptr );

    unsigned short thisamt = item->getamount();
    if ( thisamt > amount )
      thisamt = static_cast<unsigned short>( amount );
    subtract_amount_from_item( item, thisamt );
    amount -= thisamt;
  }
}

Items::Item* UContainer::remove( u32 objserial, UContainer** found_in )
{
  Items::Item* item;
  iterator itr;

  item = find( objserial, itr );
  if ( item != nullptr )
  {
    if ( found_in != nullptr )
      *found_in = item->container;

    item->container->remove( itr );
  }
  return item;
}

void UContainer::remove( Items::Item* item )
{
  if ( item->container != this )
  {
    POLLOGLN( "UContainer::remove(Item*), serial={:#x}, item={:#x}, item->cont={:#x}", serial,
              item->serial, item->container->serial );
    passert_always( item->container == this );
    int* p = nullptr;
    *p = 6;
  }

  iterator itr = std::find( contents_.begin(), contents_.end(), item );
  passert_always( itr != contents_.end() );

  // DAVE added this 11/17. refresh owner's weight on delete
  Mobile::Character* chr_owner = item->GetCharacterOwner();

  if ( chr_owner != nullptr && chr_owner->client != nullptr )
  {
    send_remove_object_to_inrange( item );
  }
  remove( itr );

  if ( chr_owner != nullptr && chr_owner->client != nullptr )
  {
    send_full_statmsg( chr_owner->client, chr_owner );
    // chr_owner->refresh_ar();
  }
}

// This is the only function that actually removes the item from the container. This could be
// inlined into UContainer::remove(Item *item) if we change UContainer::remove(u32 serial, ...)
// to call remove(*itr) instead of remove(itr). The code would be cleaner at the cost of an extra
// find.
// As I don't have time to test if that would make a big difference, I leave this note for the
// future.
// I wish you luck. (Nando, 2014/10/30)
void UContainer::remove( iterator itr )
{
  INC_PROFILEVAR( container_removes );
  Items::Item* item = *itr;
  contents_.erase( itr );
  item->container = nullptr;
  item->reset_slot();
  item->set_dirty();
  remove_bulk( item );
}

// FIXME this is depth-first.  Not sure I like that.
UContainer* UContainer::find_container( u32 objserial ) const
{
  for ( auto& item : contents_ )
  {
    if ( item && item->isa( UOBJ_CLASS::CLASS_CONTAINER ) )
    {
      UContainer* cont = static_cast<UContainer*>( item );
      if ( cont->serial == objserial )
        return cont;
      cont = cont->find_container( objserial );
      if ( cont != nullptr )
        return cont;
    }
  }
  return nullptr;
}

Items::Item* UContainer::find( u32 objserial, iterator& where_in_container )
{
  for ( iterator itr = contents_.begin(); itr != contents_.end(); ++itr )
  {
    Items::Item* item = *itr;
    passert( item != nullptr );
    if ( item != nullptr )
    {
      if ( item->serial == objserial )
      {
        where_in_container = itr;
        return item;
      }
      if ( item->isa( UOBJ_CLASS::CLASS_CONTAINER ) )
      {
        UContainer* cont = static_cast<UContainer*>( item );
        if ( !cont->locked() )
        {
          item = cont->find( objserial, where_in_container );
          if ( item != nullptr )
            return item;
        }
      }
    }
  }
  return nullptr;
}

Items::Item* UContainer::find( u32 objserial ) const
{
  for ( const auto& item : contents_ )
  {
    passert( item != nullptr );
    if ( item != nullptr )
    {
      if ( item->serial == objserial )
        return item;

      if ( item->isa( UOBJ_CLASS::CLASS_CONTAINER ) )
      {
        UContainer* cont = static_cast<UContainer*>( item );
        if ( !cont->locked() )
        {
          auto child_item = cont->find( objserial );
          if ( child_item != nullptr )
            return child_item;
        }
      }
    }
  }
  return nullptr;
}

Items::Item* UContainer::find_toplevel( u32 objserial ) const
{
  for ( auto& item : contents_ )
  {
    passert( item != nullptr );
    if ( item != nullptr )
    {
      if ( item->serial == objserial )
        return item;
    }
  }
  return nullptr;
}

void UContainer::for_each_item( void ( *f )( Items::Item* item, void* a ), void* arg )
{
  for ( auto& item : contents_ )
  {
    if ( item->isa( UOBJ_CLASS::CLASS_CONTAINER ) )
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

void UContainer::builtin_on_use( Network::Client* client )
{
  client->pause();

  if ( !locked() )
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

Core::Pos2d UContainer::get_random_location() const
{
  const auto range = desc.bounds.se() - desc.bounds.nw();
  u16 x = desc.bounds.nw().x();
  u16 y = desc.bounds.nw().y();
  if ( range.x() > 0 )
    x += static_cast<u16>( Clib::random_int( range.x() - 1 ) );
  if ( range.y() > 0 )
    y += static_cast<u16>( Clib::random_int( range.y() - 1 ) );
  return { x, y };
}

bool UContainer::is_legal_posn( const Core::Pos2d& pos ) const
{
  return desc.bounds.contains( pos );
}

void UContainer::spill_contents( Multi::UMulti* multi )
{
  passert( container == nullptr );
  if ( !locked() )
  {
    while ( !contents_.empty() )
    {
      Items::Item* item = contents_.back();
      if ( item->movable() )
      {
        contents_.pop_back();

        item->setposition( toplevel_pos() );
        item->container = nullptr;
        add_item_to_world( item );
        move_item( item, item->pos() );
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


void UContainer::on_remove( Mobile::Character* chr, Items::Item* item, MoveType move,
                            Items::Item* split_from )
{
  if ( !desc.on_remove_script.empty() )
  {
    // static code analysis indicates (C6211) that this might leak, but I can't use an auto_ptr<>
    // because of UninitObject::create() ... ideas? Nando - 2010-07-10
    Bscript::BObjectImp* chrParam = nullptr;
    if ( chr )  // consider: move this into make_mobileref
      chrParam = new Module::ECharacterRefObjImp( chr );
    else
      chrParam = Bscript::UninitObject::create();
    // Luth: 10/22/2008 - on_remove_script now called with all appropriate parameters
    call_script( desc.on_remove_script, chrParam, new Module::EItemRefObjImp( this ),
                 new Module::EItemRefObjImp( item ), new Bscript::BLong( item->getamount() ),
                 new Bscript::BLong( move ),
                 split_from ? split_from->make_ref() : Bscript::UninitObject::create() );
  }
}

bool UContainer::can_insert_increase_stack( Mobile::Character* mob, MoveType movetype,
                                            Items::Item* existing_item, unsigned short amt_to_add,
                                            Items::Item* adding_item )
{
  if ( !desc.can_insert_script.empty() )
  {
    return call_script( desc.can_insert_script,
                        mob ? mob->make_ref() : Bscript::UninitObject::create(), make_ref(),
                        new Bscript::BLong( movetype ), new Bscript::BLong( INSERT_INCREASE_STACK ),
                        adding_item ? adding_item->make_ref() : Bscript::UninitObject::create(),
                        existing_item->make_ref(), new Bscript::BLong( amt_to_add ) );
  }
  else
  {
    return true;
  }
}

void UContainer::on_insert_increase_stack( Mobile::Character* mob, MoveType movetype,
                                           Items::Item* existing_item, unsigned short amt_to_add )
{
  if ( !desc.on_insert_script.empty() )
  {
    call_script( desc.on_insert_script, mob ? mob->make_ref() : Bscript::UninitObject::create(),
                 make_ref(), new Bscript::BLong( movetype ),
                 new Bscript::BLong( INSERT_INCREASE_STACK ), Bscript::UninitObject::create(),
                 existing_item->make_ref(), new Bscript::BLong( amt_to_add ) );
  }
}

bool UContainer::can_insert_add_item( Mobile::Character* mob, MoveType movetype,
                                      Items::Item* new_item )
{
  if ( !desc.can_insert_script.empty() )
  {
    return call_script( desc.can_insert_script,
                        mob ? mob->make_ref() : Bscript::UninitObject::create(), make_ref(),
                        new Bscript::BLong( movetype ), new Bscript::BLong( INSERT_ADD_ITEM ),
                        new_item->make_ref() );
  }
  else
  {
    return true;
  }
}

void UContainer::on_insert_add_item( Mobile::Character* mob, MoveType movetype,
                                     Items::Item* new_item )
{
  if ( !desc.on_insert_script.empty() )
  {
    Items::Item* existing_stack = find_addable_stack( new_item );

    call_script( desc.on_insert_script, mob ? mob->make_ref() : Bscript::UninitObject::create(),
                 make_ref(), new Bscript::BLong( movetype ), new Bscript::BLong( INSERT_ADD_ITEM ),
                 new_item->make_ref(),
                 existing_stack ? existing_stack->make_ref() : Bscript::UninitObject::create(),
                 new Bscript::BLong( new_item->getamount() ) );
  }
}

bool UContainer::check_can_remove_script( Mobile::Character* chr, Items::Item* item, MoveType move,
                                          u16 amount )
{
  if ( !desc.can_remove_script.empty() )
  {
    Bscript::BObjectImp* chrParam = nullptr;
    if ( chr != nullptr )  // TODO: consider moving this into make_mobileref
      chrParam = chr->make_ref();
    else
      chrParam = Bscript::UninitObject::create();

    if ( amount == 0 )
      amount = item->getamount();

    return call_script( desc.can_remove_script, chrParam, make_ref(), item->make_ref(),
                        new Bscript::BLong( move ), new Bscript::BLong( amount ) );
  }
  else
  {
    return true;
  }
}


void UContainer::printProperties( Clib::StreamWriter& sw ) const
{
  base::printProperties( sw );
  if ( has_max_items_mod() )
    sw.add( "Max_Items_mod", max_items_mod() );
  if ( has_max_weight_mod() )
    sw.add( "Max_Weight_mod", max_weight_mod() );
  if ( has_max_slots_mod() )
    sw.add( "Max_Slots_mod", max_slots_mod() );
  if ( no_drop_exception() != default_no_drop_exception() )
    sw.add( "NoDropException", no_drop_exception() );
  if ( has_held_weight_multiplier() )
    sw.add( "HeldWeightMultiplier", held_weight_multiplier() );
}

void UContainer::readProperties( Clib::ConfigElem& elem )
{
  base::readProperties( elem );
  max_items_mod( static_cast<s16>( elem.remove_int( "MAX_ITEMS_MOD", 0 ) ) );
  max_weight_mod( static_cast<s16>( elem.remove_int( "MAX_WEIGHT_MOD", 0 ) ) );
  max_slots_mod( static_cast<s8>( elem.remove_int( "MAX_SLOTS_MOD", 0 ) ) );
  no_drop_exception( elem.remove_bool( "NoDropException", default_no_drop_exception() ) );
  held_weight_multiplier(
      elem.remove_double( "HeldWeightMultiplier", desc.held_weight_multiplier ) );
}

unsigned int UContainer::find_sumof_objtype_noninuse( u32 objtype, u32 amtToGet,
                                                      Contents& saveItemsTo, int flags ) const
{
  unsigned int amt = 0;

  for ( auto& item : contents_ )
  {
    if ( item && !item->inuse() )
    {
      if ( item->objtype_ == objtype )
      {
        saveItemsTo.push_back( item );
        amt += item->getamount();
      }
      if ( !( flags & FINDSUBSTANCE_ROOT_ONLY ) && ( item->isa( UOBJ_CLASS::CLASS_CONTAINER ) ) )
      {
        UContainer* cont = static_cast<UContainer*>( item );
        if ( !cont->locked() || ( flags & FINDSUBSTANCE_IGNORE_LOCKED ) )
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
  auto item = static_cast<UContainer*>( base::clone() );

  item->max_items_mod( this->max_items_mod() );
  item->max_weight_mod( this->max_weight_mod() );
  item->max_slots_mod( this->max_slots_mod() );
  item->no_drop_exception( no_drop_exception() );
  item->held_weight_multiplier( held_weight_multiplier() );

  return item;
}

unsigned short UContainer::max_items() const
{
  const auto max_items = desc.max_items + max_items_mod();
  if ( max_items == 0 )
    return MAX_CONTAINER_ITEMS;

  return static_cast<u16>( std::clamp( max_items, 1, MAX_CONTAINER_ITEMS ) );
}

unsigned short UContainer::max_weight() const
{
  const s32 max_weight = desc.max_weight + max_weight_mod();

  if ( max_weight < 1 )
    return USHRT_MAX;
  else if ( max_weight <= USHRT_MAX )
    return static_cast<u16>( max_weight );
  else
    return USHRT_MAX;
}

u8 UContainer::max_slots() const
{
  const auto max_slots = desc.max_slots + max_slots_mod();
  return static_cast<u8>( std::clamp( max_slots, 0, MAX_SLOTS ) );
}

bool UContainer::no_drop_exception() const
{
  return flags_.get( Core::OBJ_FLAGS::NO_DROP_EXCEPTION );
}

void UContainer::no_drop_exception( bool newvalue )
{
  flags_.change( Core::OBJ_FLAGS::NO_DROP_EXCEPTION, newvalue );
}

bool UContainer::default_no_drop_exception() const
{
  return desc.no_drop_exception;
}

bool UContainer::get_method_hook( const char* methodname, Bscript::Executor* ex,
                                  ExportScript** hook, unsigned int* PC ) const
{
  if ( gamestate.system_hooks.get_method_hook( gamestate.system_hooks.container_method_script.get(),
                                               methodname, ex, hook, PC ) )
    return true;
  return base::get_method_hook( methodname, ex, hook, PC );
}
}  // namespace Core
}  // namespace Pol
