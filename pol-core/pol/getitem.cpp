/** @file
 *
 * @par History
 * - 2005/12/05 MuadDib:   Added set_decay_after(60) when item is approved to be "gotten" by
 *                         line 19x for a dirty fix for instant decay on items dropped in current
 *                         decay zone being processed.
 * - 2006/03/01 MuadDib:   Added check for != 1 on item-> amount to set_decay_after to help with
 *                         bug of changing the initial stack's decay timer.
 * - 2008/12/17 MuadDub:   unequipping item now resets item->layer to 0
 * - 2009/08/06 MuadDib:   Added gotten_by code for items.
 * - 2009/08/16 MuadDib:   Added Slot support to undo_get_item()
 * - 2009/09/03 MuadDib:   Changes for account related source file relocation
 *                         Changes for multi related source file relocation
 */

#include "getitem.h"

#include <cstdio>

#include "../clib/clib_endian.h"
#include "../clib/rawtypes.h"
#include "containr.h"
#include "item/item.h"
#include "mobile/charactr.h"
#include "network/client.h"
#include "network/pktdef.h"
#include "network/pktin.h"
#include "reftypes.h"
#include "statmsg.h"
#include "ufunc.h"
#include "uobject.h"
#include "uworld.h"


/* How get_item works:
   when the client drags an item off the ground,
     a GET_ITEM message is generated.
   when this item is dropped on the paperdoll,
     an EQUIP_ITEM message is generated.
   when this item is dropped on the ground,
     a DROP_ITEM message is generated
   when this item is placed in a container,
     a DROP_ITEM message is generated.


   if the item requested cannot be 'gotten', a ITEM_MOVE_FAILURE message is sent (0x27),
       reason code 0 (you cannot pick that up).  The client automatically places the
       item back where it was. (what do the real servers do?)

   if on equip, the item cannot be placed, an ITEM_MOVE_FAILURE(0x27,0x05) is sent.
       the client just beeps, and does not release the object.

*/
namespace Pol
{
namespace Core
{
void get_item( Network::Client* client, PKTIN_07* msg )
{
  u32 serial = cfBEu32( msg->serial );
  u16 amount = cfBEu16( msg->amount );

  Items::Item* item;

  if ( client->chr->has_gotten_item() )
  {
    send_item_move_failure( client, MOVE_ITEM_FAILURE_ALREADY_HOLDING_AN_ITEM );
    return;
  }
  if ( client->chr->dead() )
  {
    send_item_move_failure( client, MOVE_ITEM_FAILURE_CANNOT_PICK_THAT_UP );
    return;
  }
  // try to find the item the client referenced, in all the legal places it might be.

  bool inRemoteContainer = false, isRemoteContainer = false;
  item = find_legal_item( client->chr, serial, &inRemoteContainer, &isRemoteContainer );
  if ( item == nullptr || isRemoteContainer )
  {
    if ( find_snoopable_item( serial ) != nullptr )
    {
      send_item_move_failure( client, MOVE_ITEM_FAILURE_BELONGS_TO_OTHER );
      return;
    }

    send_item_move_failure( client, MOVE_ITEM_FAILURE_CANNOT_PICK_THAT_UP );
    return;
  }
  ItemRef itemref( item );  // dave 1/28/3 prevent item from being destroyed before function ends

  u8 oldSlot = item->slot_index();

  if ( pol_distance( client->chr, item ) > 2 && !client->chr->can_moveanydist() )
  {
    send_item_move_failure( client, MOVE_ITEM_FAILURE_TOO_FAR_AWAY );
    return;
  }
  if ( !client->chr->realm()->has_los( *client->chr, *( item->toplevel_owner() ) ) )
  {
    send_item_move_failure( client, MOVE_ITEM_FAILURE_OUT_OF_SIGHT );
    return;
  }
  if ( item->inuse() )
  {
    send_sysmessage( client, "That is already being used." );
    send_item_move_failure( client, MOVE_ITEM_FAILURE_CANNOT_PICK_THAT_UP );
    return;
  }

  if ( !client->chr->can_move( item ) )
  {
    send_sysmessage( client, "You cannot move that." );
    send_item_move_failure( client, MOVE_ITEM_FAILURE_CANNOT_PICK_THAT_UP );
    return;
  }

  if ( !item->check_unequiptest_scripts() || !item->check_unequip_script() )
  {
    send_sysmessage( client, "You cannot unequip that." );
    send_item_move_failure( client, MOVE_ITEM_FAILURE_CANNOT_PICK_THAT_UP );
    return;
  }
  if ( item->orphan() )  // dave added 1/28/3, item might be destroyed in RTC script
  {
    return;
  }

  if ( item->container )
  {
    if ( !item->container->check_can_remove_script( client->chr, item ) )
    {
      send_item_move_failure( client, MOVE_ITEM_FAILURE_CANNOT_PICK_THAT_UP );
      return;
    }
    if ( item->orphan() )  // dave added 1/28/3, item might be destroyed in RTC script
    {
      return;
    }
  }

  UObject* my_owner = item->toplevel_owner();

  send_remove_object_to_inrange( item );

  UContainer* orig_container = item->container;
  u16 orig_x = item->x(), orig_y = item->y();
  s8 orig_z = item->z();

  if ( item->container != nullptr )
  {
    if ( IsCharacter( item->container->serial ) )
      client->chr->gotten_item_source = Mobile::Character::GOTTEN_ITEM_EQUIPPED_ON_SELF;
    else
      client->chr->gotten_item_source = Mobile::Character::GOTTEN_ITEM_IN_CONTAINER;
    item->extricate();
  }
  else
  {
    client->chr->gotten_item_source = Mobile::Character::GOTTEN_ITEM_ON_GROUND;
    remove_item_from_world( item );
  }

  client->chr->gotten_item( item );
  item->inuse( true );
  item->gotten_by( client->chr );
  item->setposition( Pos4d( 0, 0, 0, item->realm() ) );  // don't let a boat carry it around

  if ( orig_container != nullptr )
  {
    orig_container->on_remove( client->chr, item );
    if ( item->orphan() )  // dave added 1/28/3, item might be destroyed in RTC script
    {
      return;
    }
  }

  /* Check for moving part of a stack.  Here are the possibilities:
      1) Client specified more amount than was in the stack.
      2) Client specified exactly what was in the stack.
      These are handled identically.  The amount specified is ignored, and
      the item is effectively treated as normal unstackable atomic object.
      (the stack is moved as a whole)
      3) Client specified less than is in the stack.
      In this case, a new object is created at the same location as the old object,
      with the balance of the amount not removed.
      */
  if ( item->amount_to_remove_is_partial( amount ) )
  {
    Items::Item* new_item = item->slice_stacked_item( amount );
    if ( new_item != nullptr )
    {
      new_item->restart_decay_timer();
      new_item->setposition( Pos4d( orig_x, orig_y, orig_z, item->realm() ) );
      if ( orig_container != nullptr )
      {
        // NOTE: we just removed 'item' from its container,
        // so there's room for new_item.
        if ( !orig_container->can_add_to_slot( oldSlot ) || !item->slot_index( oldSlot ) )
        {
          new_item->set_dirty();
          new_item->setposition( client->chr->pos() );
          add_item_to_world( new_item );
          register_with_supporting_multi( new_item );
          move_item( new_item, new_item->x(), new_item->y(), new_item->z(), nullptr );
        }
        else
        {
          orig_container->add( new_item );
          send_put_in_container_to_inrange( new_item );
        }
      }
      else
      {
        add_item_to_world( new_item );
        register_with_supporting_multi( new_item );
        send_item_to_inrange( new_item );
      }
    }
  }
  else
  {
    item->set_decay_after( 60 );
  }

  // FIXME : Are these all the possibilities for sources and updating, correctly?
  if ( client->chr->gotten_item_source == Mobile::Character::GOTTEN_ITEM_ON_GROUND )
  {
    // Item was on the ground, so we ONLY need to update the character's weight
    // to the client.
    send_full_statmsg( client, client->chr );
  }
  else if ( client->chr->gotten_item_source == Mobile::Character::GOTTEN_ITEM_EQUIPPED_ON_SELF )
  {
    // Item was equipped, let's send the full update for ar and statmsg.
    client->chr->refresh_ar();
  }
  else if ( my_owner->isa( UOBJ_CLASS::CLASS_CONTAINER ) )
  {
    // Toplevel owner was a container (not a character). Only update weight.
    send_full_statmsg( client, client->chr );
  }
  else if ( ( my_owner->ismobile() ) && my_owner->serial != client->chr->serial )
  {
    // Toplevel was a mob. Make sure mob was not us. If it's not, send update to weight.
    send_full_statmsg( client, client->chr );
  }
}


/*
  undo_get_item:
  when a client issues a get_item command, the item is moved into gotten_items.
  all other clients are told to delete it, so they no longer have access to it.
  when the client finally tries to do something with it, if that fails, the
  object must be put back where it was.
  Sometimes, (ie trying to equip) the client just beeps, and keeps holding onto
  the item.  In those cases, this function is not called but rather the item
  is replaced in gotten_items, for a later EQUIP_ITEM message.
  */

void undo_get_item( Mobile::Character* chr, Items::Item* item )
{
  // item needs to be returned to where it was..  either on
  // the ground, or equipped on the current character,
  // or in whatever it used to be in.
  ItemRef itemref( item );  // dave 1/28/3 prevent item from being destroyed before function ends
  item->restart_decay_timer();  // MuadDib: moved to top to help with instant decay.

  item->gotten_by( nullptr );
  if ( chr->gotten_item_source == Mobile::Character::GOTTEN_ITEM_EQUIPPED_ON_SELF )
  {
    if ( chr->equippable( item ) && item->check_equiptest_scripts( chr ) &&
         item->check_equip_script( chr, false ) )
    {
      if ( item->orphan() )  // dave added 1/28/3, item might be destroyed in RTC script
      {
        return;
      }
      // is it possible the character doesn't exist? no, it's my character doing the undoing.
      chr->equip( item );
      send_wornitem_to_inrange( chr, item );
      return;
    }
    if ( item->orphan() )  // dave added 1/28/3, item might be destroyed in RTC script
    {
      return;
    }
    chr->gotten_item_source = Mobile::Character::GOTTEN_ITEM_IN_CONTAINER;
  }

  if ( chr->gotten_item_source == Mobile::Character::GOTTEN_ITEM_IN_CONTAINER )
  {
    // First attempt to put it back in the original container.
    // NOTE: This is lost somewhere before it gets here and so never happens.
    u8 newSlot = 1;
    UContainer* orig_container = item->container;
    if ( orig_container && orig_container->can_insert_add_item( chr, UContainer::MT_PLAYER, item ) )
    {
      if ( item->orphan() )
      {
        return;
      }
      else if ( orig_container->is_legal_posn( item, item->x(), item->y() ) )
      {
        if ( !orig_container->can_add_to_slot( newSlot ) || !item->slot_index( newSlot ) )
        {
          item->set_dirty();
          item->setposition( chr->pos() );
          add_item_to_world( item );
          register_with_supporting_multi( item );
          move_item( item, chr->x(), chr->y(), chr->z(), nullptr );
          return;
        }
        else
          orig_container->add( item );
      }
      else
      {
        if ( !orig_container->can_add_to_slot( newSlot ) || !item->slot_index( newSlot ) )
        {
          item->set_dirty();
          item->setposition( chr->pos() );
          add_item_to_world( item );
          register_with_supporting_multi( item );
          move_item( item, chr->x(), chr->y(), chr->z(), nullptr );
          return;
        }
        else
          orig_container->add_at_random_location( item );
      }
      update_item_to_inrange( item );
      orig_container->on_insert_add_item( chr, UContainer::MT_PLAYER, item );
      return;
    }

    // Attempt to place the item in the player's backpack.
    UContainer* bp = chr->backpack();
    if ( bp != nullptr && bp->can_add( *item ) &&
         bp->can_insert_add_item( chr, UContainer::MT_PLAYER, item ) )
    {
      if ( item->orphan() )
        return;
      else if ( bp->is_legal_posn( item, item->x(), item->y() ) )
      {
        // NOTE it's never in a legal position, cause we clear the x/y/z in getitem
        if ( !bp->can_add_to_slot( newSlot ) || !item->slot_index( newSlot ) )
        {
          item->set_dirty();
          item->setposition( chr->pos() );
          add_item_to_world( item );
          register_with_supporting_multi( item );
          move_item( item, chr->x(), chr->y(), chr->z(), nullptr );
          return;
        }
        else
          bp->add( item );
      }
      else
      {
        if ( !bp->can_add_to_slot( newSlot ) || !item->slot_index( newSlot ) )
        {
          item->set_dirty();
          item->setposition( chr->pos() );
          add_item_to_world( item );
          register_with_supporting_multi( item );
          move_item( item, chr->x(), chr->y(), chr->z(), nullptr );
          return;
        }
        else
          bp->add_at_random_location( item );
      }
      update_item_to_inrange( item );
      bp->on_insert_add_item( chr, UContainer::MT_PLAYER, item );
      return;
    }
  }

  // Last resort - put it at the player's feet.
  item->set_dirty();
  item->setposition( chr->pos() );
  item->container = nullptr;
  // 12-17-2008 MuadDib added to clear item.layer properties.
  item->layer = 0;

  add_item_to_world( item );

  register_with_supporting_multi( item );
  send_item_to_inrange( item );
}
}  // namespace Core
}  // namespace Pol
