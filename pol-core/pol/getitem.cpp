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
#include "fnsearch.h"
#include "item/item.h"
#include "los.h"
#include "mobile/charactr.h"
#include "network/client.h"
#include "network/packethelper.h"
#include "network/pktdef.h"
#include "network/pktin.h"
#include "reftypes.h"
#include "statmsg.h"
#include "systems/suspiciousacts.h"
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
bool drop_item_on_object( Network::Client* client, Items::Item* item, u32 target_serial,
                          u8 slotIndex );

bool place_item( Network::Client* client, Items::Item* item, u32 target_serial, const Pos2d& pos,
                 u8 slotIndex );

void GottenItem::handle_lift( Network::Client* client, PKTIN_07* msg )
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

  if ( !client->chr->pos().in_range( item->pos(), 2 ) && !client->chr->can_moveanydist() )
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
  if ( item->orphan() )
    return;

  if ( item->container )
  {
    if ( !item->container->check_can_remove_script( client->chr, item ) )
    {
      send_item_move_failure( client, MOVE_ITEM_FAILURE_CANNOT_PICK_THAT_UP );
      return;
    }
    if ( item->orphan() )
      return;
  }

  UObject* my_owner = item->toplevel_owner();

  send_remove_object_to_inrange( item );

  UContainer* orig_container = item->container;
  Pos4d orig_pos = item->pos();  // potential container pos
  Pos4d orig_toppos = item->toplevel_pos();

  GottenItem gotten_info{ item, orig_pos };
  if ( orig_container != nullptr )
  {
    if ( IsCharacter( orig_container->serial ) )
      gotten_info._source = GOTTEN_ITEM_TYPE::GOTTEN_ITEM_EQUIPPED_ON_SELF;
    else
      gotten_info._source = GOTTEN_ITEM_TYPE::GOTTEN_ITEM_IN_CONTAINER;
    gotten_info._slot_index = item->slot_index();
    item->extricate();
  }
  else
  {
    gotten_info._source = GOTTEN_ITEM_TYPE::GOTTEN_ITEM_ON_GROUND;
    remove_item_from_world( item );
  }

  client->chr->gotten_item( gotten_info );
  item->inuse( true );
  item->gotten_by( client->chr );
  item->setposition( Pos4d( 0, 0, 0, item->realm() ) );  // don't let a boat carry it around

  if ( orig_container != nullptr )
  {
    orig_container->on_remove( client->chr, item );
    if ( item->orphan() )
      return;
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
      new_item->setposition( orig_pos );
      if ( orig_container != nullptr )
      {
        // NOTE: we just removed 'item' from its container,
        // so there's room for new_item.
        if ( !orig_container->can_add_to_slot( oldSlot ) || !item->slot_index( oldSlot ) )
        {
          new_item->setposition( client->chr->pos() );
          add_item_to_world( new_item );
          register_with_supporting_multi( new_item );
          move_item( new_item, orig_toppos );
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
  if ( gotten_info._source == GOTTEN_ITEM_TYPE::GOTTEN_ITEM_ON_GROUND )
  {
    // Item was on the ground, so we ONLY need to update the character's weight
    // to the client.
    send_full_statmsg( client, client->chr );
  }
  else if ( gotten_info._source == GOTTEN_ITEM_TYPE::GOTTEN_ITEM_EQUIPPED_ON_SELF )
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


bool GottenItem::drop( Network::Client* client, u32 item_serial, const Pos3d& pos,
                       u32 target_serial, u8 slot_index )
{
  if ( _item == nullptr )
  {
    SuspiciousActs::DropItemButNoneGotten( client, item_serial );
    return false;
  }
  if ( _item->serial != item_serial )
  {
    SuspiciousActs::DropItemOtherThanGotten( client, item_serial, _item->serial );
    _item->gotten_by( nullptr );  // TODO: shouldn't we clear_gotten_item() here?
    return false;
  }
  _item->inuse( false );
  _item->gotten_by( nullptr );
  client->chr->gotten_item( {} );

  bool res;
  if ( target_serial == 0xFFffFFffLu )
  {
    res = drop_on_ground( client, pos );
  }
  else if ( pos.x() == 0xFFFF )
  {
    res = drop_item_on_object( client, _item, target_serial, slot_index );
  }
  else
  {
    Multi::UMulti* multi = system_find_multi( target_serial );

    if ( multi != nullptr )
      res = drop_on_ground( client, pos + Vec2d( multi->x(), multi->y() ) );
    else
      res = place_item( client, _item, target_serial, pos.xy(), 0 );
  }

  if ( !_item->orphan() )
  {
    if ( !res )
      undo( client->chr );
    _item->inuse( false );
    _item->gotten_by( nullptr );
  }
  return true;
}

/* DROP_ITEM messages come in a couple varieties:

    1)  Dropping an item on another object, or a person:
    item_serial: serial number of item to drop
    x: 0xFFFF
    y: 0xFFFF
    z: 0
    target_serial: serial number of item or character to drop on.

    2)  Dropping an item on the ground:
    item_serial: serial number of item to drop
    x,y,z: position
    target_serial: 0xFFFFFFFF

    3)  Placing an item in a container, or in an existing pile:
    item_serial: serial number of item to drop
    x: x-position
    y: y-position
    z: 0
    target_serial: serial number of item or character or pile to drop on.
*/
/*Original version of packet is supported by this function.*/
void GottenItem::handle_drop( Network::Client* client, PKTIN_08_V1* msg )
{
  u32 item_serial = cfBEu32( msg->item_serial );
  auto pos = Pos3d( cfBEu16( msg->x ), cfBEu16( msg->y ), msg->z );
  u32 target_serial = cfBEu32( msg->target_serial );

  GottenItem info = client->chr->gotten_item();
  info.drop( client, item_serial, pos, target_serial, 0 );
  send_full_statmsg( client, client->chr );
}

/*This is used for the version of the packet introduced in clients 6.0.1.7 2D and
UO:KR+ to support Slots*/
void GottenItem::handle_drop_v2( Network::Client* client, PKTIN_08_V2* msg )
{
  u32 item_serial = cfBEu32( msg->item_serial );
  auto pos = Pos3d( cfBEu16( msg->x ), cfBEu16( msg->y ), msg->z );
  u8 slotIndex = msg->slotindex;
  u32 target_serial = cfBEu32( msg->target_serial );

  GottenItem info = client->chr->gotten_item();
  if ( !info.drop( client, item_serial, pos, target_serial, slotIndex ) )
    return;

  Network::PktHelper::PacketOut<Network::PktOut_29> drop_msg;
  drop_msg.Send( client );
  send_full_statmsg( client, client->chr );
}

bool GottenItem::drop_on_ground( Network::Client* client, const Pos3d& pos )
{
  if ( _item->no_drop() )
  {
    send_item_move_failure( client, MOVE_ITEM_FAILURE_UNKNOWN );
    return false;
  }

  Mobile::Character* chr = client->chr;

  Multi::UMulti* multi;
  short newz;
  if ( !chr->pos().in_range( pos, 2 ) && !client->chr->can_moveanydist() )
  {
    SuspiciousActs::DropItemOutOfRange( client, _item->serial );
    send_item_move_failure( client, MOVE_ITEM_FAILURE_TOO_FAR_AWAY );
    return false;
  }

  if ( !chr->realm()->dropheight( pos, client->chr->z(), &newz, &multi ) )
  {
    SuspiciousActs::DropItemOutAtBlockedLocation( client, _item->serial, pos );
    send_item_move_failure( client, MOVE_ITEM_FAILURE_TOO_FAR_AWAY );
    return false;
  }

  LosObj tgt( Pos4d( pos.xy(), static_cast<s8>( newz ), chr->realm() ) );
  if ( !chr->realm()->has_los( *client->chr, tgt ) )
  {
    send_item_move_failure( client, MOVE_ITEM_FAILURE_OUT_OF_SIGHT );
    return false;
  }

  _item->set_dirty();
  _item->restart_decay_timer();
  auto oldrealm = _item->realm();  // TODO POS get rid of all the realm for_each
  _item->setposition( tgt.pos() );
  if ( oldrealm != chr->realm() )
  {
    if ( _item->isa( UOBJ_CLASS::CLASS_CONTAINER ) )
    {
      UContainer* cont = static_cast<UContainer*>( _item );
      cont->for_each_item( setrealm, (void*)chr->realm() );
    }
  }
  _item->container = nullptr;
  _item->reset_slot();
  add_item_to_world( _item );
  if ( multi != nullptr )
    multi->register_object( _item );

  send_item_to_inrange( _item );
  return true;
}

GottenItem::GottenItem( Items::Item* item, Core::Pos4d pos )
    : _item( item ), _pos( std::move( pos ) ), _cnt_serial( 0 )
{
}

/*
  undo:
  when a client issues a get_item command, the item is moved into gotten_items.
  all other clients are told to delete it, so they no longer have access to it.
  when the client finally tries to do something with it, if that fails, the
  object must be put back where it was.
  Sometimes, (ie trying to equip) the client just beeps, and keeps holding onto
  the item.  In those cases, this function is not called but rather the item
  is replaced in gotten_items, for a later EQUIP_ITEM message.
  */
void GottenItem::undo( Mobile::Character* chr )
{
  if ( !_item )
    return;
  // item needs to be returned to where it was..  either on
  // the ground, or equipped on the current character,
  // or in whatever it used to be in.
  ItemRef itemref( _item );  // dave 1/28/3 prevent item from being destroyed before function ends
  _item->restart_decay_timer();  // MuadDib: moved to top to help with instant decay.
  _item->gotten_by( nullptr );
  if ( _source == GOTTEN_ITEM_TYPE::GOTTEN_ITEM_EQUIPPED_ON_SELF )
  {
    if ( chr->equippable( _item ) && _item->check_equiptest_scripts( chr ) &&
         _item->check_equip_script( chr, false ) )
    {
      if ( _item->orphan() )
        return;
      // is it possible the character doesn't exist? no, it's my character doing the undoing.
      chr->equip( _item );
      send_wornitem_to_inrange( chr, _item );
      return;
    }
    if ( _item->orphan() )
      return;
    _source = GOTTEN_ITEM_TYPE::GOTTEN_ITEM_IN_CONTAINER;
    _pos = chr->pos();
  }

  if ( _source == GOTTEN_ITEM_TYPE::GOTTEN_ITEM_IN_CONTAINER )
  {
    // First attempt to put it back in the original container.
    UContainer* container = nullptr;
    auto* orig_obj = system_find_object( _cnt_serial );
    if ( orig_obj && orig_obj->isa( UOBJ_CLASS::CLASS_CONTAINER ) )
    {
      container = static_cast<UContainer*>( orig_obj );
      if ( !container->can_add( *_item ) ||
           !container->can_insert_add_item( chr, UContainer::MT_PLAYER, _item ) )
        container = nullptr;
    }
    if ( _item->orphan() )
      return;
    if ( !container )
    {
      // Attempt to place the item in the player's backpack.
      container = chr->backpack();
      if ( !container || !container->can_add( *_item ) ||
           !container->can_insert_add_item( chr, UContainer::MT_PLAYER, _item ) )
        container = nullptr;
    }
    if ( _item->orphan() )
      return;
    if ( container )
    {
      u8 newSlot = _slot_index ? _slot_index : 1;
      if ( container->can_add_to_slot( newSlot ) && _item->slot_index( newSlot ) )
      {
        if ( container->is_legal_posn( _pos.xy() ) )
        {
          _item->setposition( _pos );
          container->add( _item );
        }
        else
          container->add_at_random_location( _item );
        update_item_to_inrange( _item );
        container->on_insert_add_item( chr, UContainer::MT_PLAYER, _item );
        return;
      }
    }
    _pos = chr->pos();
  }

  // Last resort - put it on the ground, to players feet in case of error from above.
  _item->setposition( _pos );
  _item->container = nullptr;
  // 12-17-2008 MuadDib added to clear item.layer properties.
  _item->layer = 0;

  add_item_to_world( _item );

  register_with_supporting_multi( _item );
  send_item_to_inrange( _item );
}
}  // namespace Core
}  // namespace Pol
