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

#include "pol/getitem.h"

#include <cstdio>
#include <optional>
#include <variant>

#include "clib/clib_endian.h"
#include "clib/rawtypes.h"
#include "pol/containr.h"
#include "pol/fnsearch.h"
#include "pol/item/item.h"
#include "pol/layers.h"
#include "pol/mobile/charactr.h"
#include "pol/mobile/corpse.h"
#include "pol/network/client.h"
#include "pol/network/pktdef.h"
#include "pol/network/pktin.h"
#include "pol/polclass.h"
#include "pol/realms/realms.h"
#include "pol/reftypes.h"
#include "pol/statmsg.h"
#include "pol/ufunc.h"
#include "pol/uobject.h"
#include "pol/uworld.h"


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

namespace Pol::Core
{
void release_gotten_item( Mobile::Character* chr )
{
  Items::Item* item = chr->gotten_item().item();
  // A destroyed item has no location to leave. Every core path that destroys an item clears the
  // cursor first, so this is only here to keep a character from getting stuck holding one.
  if ( item != nullptr && !item->orphan() )
    (void)Items::relocate( *item, Items::Detached{} );
  chr->gotten_item( {} );
}

void GottenItem::handle( Network::Client* client, PKTIN_07* msg )
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

  bool isRemoteContainer = false;
  item = find_legal_item( client->chr, serial, nullptr, &isRemoteContainer );
  if ( item == nullptr || isRemoteContainer )
  {
    Mobile::Character* owner = nullptr;
    item = find_snoopable_item( serial, &owner );

    if ( !item || !owner || !owner->is_equipped( item ) )
    {
      send_item_move_failure( client, MOVE_ITEM_FAILURE_CANNOT_PICK_THAT_UP );
      return;
    }
    if ( !client->chr->can_clothe( owner ) )
    {
      send_item_move_failure( client, MOVE_ITEM_FAILURE_BELONGS_TO_OTHER );
      return;
    }
  }
  ItemRef itemref( item );  // dave 1/28/3 prevent item from being destroyed before function ends

  u8 oldSlot = item->slot_index();

  if ( !client->chr->can_reach( item, 2 ) && !client->chr->can_moveanydist() )
  {
    send_item_move_failure( client, MOVE_ITEM_FAILURE_TOO_FAR_AWAY );
    return;
  }
  if ( !client->chr->stored_realm()->has_los( *client->chr, *( item->toplevel_owner() ) ) )
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

  if ( !item->check_unequiptest_scripts( client->chr ) ||
       !item->check_unequip_script( client->chr ) )
  {
    send_sysmessage( client, "You cannot unequip that." );
    send_item_move_failure( client, MOVE_ITEM_FAILURE_CANNOT_PICK_THAT_UP );
    return;
  }
  if ( item->orphan() )
    return;

  if ( UContainer* cont = item->container(); cont != nullptr )
  {
    if ( !cont->check_can_remove_script( client->chr, item, UContainer::MoveType::MT_PLAYER,
                                         amount ) )
    {
      send_item_move_failure( client, MOVE_ITEM_FAILURE_CANNOT_PICK_THAT_UP );
      return;
    }
    if ( item->orphan() )
      return;
  }

  send_remove_object_to_inrange( item );

  UContainer* orig_container = item->container();
  // Two questions that used to share one variable: which cell the item sat in inside whatever held
  // it, and where it was standing in the world. Exactly one of them applies in each branch below,
  // and the item is about to move, so both have to be read now.
  const Pos2d orig_grid = item->location().grid();
  const Pos4d orig_toppos = item->toplevel_pos();

  // One step: unlink from wherever the item is, build the return ticket describing it, and set
  // both halves of the cursor link. The rejection is reachable — the scripts run above can have
  // handed this character something else to hold, and overwriting the ticket would strand that
  // item. The removal was already sent by then, so put the item back on the clients.
  if ( !Items::relocate( *item, Items::OnCursor{ client->chr } ) )
  {
    update_item_to_inrange( item );
    send_item_move_failure( client, MOVE_ITEM_FAILURE_CANNOT_PICK_THAT_UP );
    return;
  }
  const GottenItem gotten_info = client->chr->gotten_item();

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
      if ( orig_container != nullptr )
      {
        orig_container->on_remove( client->chr, item, UContainer::MoveType::MT_PLAYER, new_item );
        if ( new_item->orphan() )
          return;
        // NOTE: we just removed 'item' from its container, so there's room for new_item.
        // The slot goes to new_item, which is what stays behind: the checks used to be run against
        // it but assigned to 'item', the part being picked up onto the cursor, which then carried a
        // slot in a container it was leaving while the remainder kept whatever it had.
        if ( Items::move_into( *new_item, *orig_container, orig_grid, oldSlot ) )
        {
          send_put_in_container_to_inrange( new_item );
        }
        else
        {
          // The remainder of the stack has to land somewhere. Besides the slot being unavailable,
          // the OnRemove script above can have destroyed the container out from under us, which
          // relocate refuses -- and used to be an assertion inside UContainer::add.
          if ( Items::place_at( *new_item, client->chr->pos() ) )
            send_item_moved( new_item, orig_toppos );
        }
      }
      else
      {
        // Nothing held it, so toplevel_owner() was the item itself and this is its own position.
        if ( Items::place_at( *new_item, orig_toppos ) )
          send_item_to_inrange( new_item );
      }
    }
  }
  else
  {
    if ( orig_container )
    {
      orig_container->on_remove( client->chr, item );
      if ( item->orphan() )
        return;
    }
    item->set_decay_after( 60 );
  }

  if ( orig_container )
  {
    orig_container->increv_send_object_recursive();
  }

  // Picking anything up changes what the character is carrying, so the stat bar always needs
  // resending. Coming off a layer changes the armor rating as well, and refresh_ar() sends the
  // stat bar itself -- to the wearer, who is not necessarily the one lifting.
  if ( const auto* from_layer = std::get_if<FromLayer>( &gotten_info._origin ) )
  {
    if ( auto chr = system_find_mobile( from_layer->serial ) )
      chr->refresh_ar();
  }
  else
  {
    send_full_statmsg( client, client->chr );
  }
}


GottenItem::GottenItem( Items::Item* item, Origin origin )
    : _item( item ), _origin( std::move( origin ) )
{
}

GottenItem GottenItem::for_item( Items::Item* item )
{
  const Items::Location loc = item->location();

  if ( const auto* equipped = loc.get_if<Items::Equipped>() )
  {
    // undo() resolves this with system_find_mobile, so it has to be the character's serial. The
    // worn-items container happens to carry the same one, but say which is meant.
    return GottenItem{ item, FromLayer{ equipped->chr->serial } };
  }
  if ( const auto* on_corpse = loc.get_if<Items::OnCorpse>() )
  {
    // Distinct from the container below on purpose: a corpse is a container, and the whole point
    // of the distinction is that an item rendered on one of its layers is not the same as an item
    // lying loose in it.
    return GottenItem{ item,
                       FromCorpse{ on_corpse->corpse->serial, on_corpse->grid, on_corpse->slot } };
  }
  // The alternative rather than the container() view, which also answers for Equipped and
  // OnCorpse: asking for InContainer directly means these three branches do not depend on the
  // order they are written in.
  if ( const auto* in_cont = loc.get_if<Items::InContainer>() )
    return GottenItem{ item, FromContainer{ in_cont->cont->serial, in_cont->grid, in_cont->slot } };

  // Whatever is left is InWorld: relocate refuses to put anything else on a cursor, precisely so
  // that this has a home to name. The toplevel position rather than item->pos() because the two
  // agree here and this says which is meant.
  const Core::Pos4d& pos = item->toplevel_pos();
  return GottenItem{ item, FromGround{ pos.xyz(), pos.realm()->name() } };
}

bool GottenItem::came_off_corpse_layer( const UContainer* cont ) const
{
  const auto* from_corpse = std::get_if<FromCorpse>( &_origin );
  return from_corpse != nullptr && _item != nullptr && cont != nullptr &&
         cont->serial == from_corpse->serial && cont->script_isa( POLCLASS_CORPSE ) &&
         Items::valid_equip_layer( _item );
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

void GottenItem::undo( Mobile::Character* chr ) const
{
  if ( !_item )
    return;
  // item needs to be returned to where it was..  either on
  // the ground, or equipped on the current character,
  // or in whatever it used to be in.
  ItemRef itemref( _item );  // dave 1/28/3 prevent item from being destroyed before function ends
  _item->restart_decay_timer();  // MuadDib: moved to top to help with instant decay.

  // What follows is a ladder of places to try, and it runs entirely on locals. The ticket says
  // where the item came from and keeps saying it, so every step below can still ask -- where the
  // ladder used to rewrite the ticket as it descended, which meant the answer to "where did this
  // come from" depended on how far down you already were.
  Realms::Realm* realm = nullptr;
  Core::Pos3d ground;               ///< where it lands if nothing else will take it
  std::optional<Core::Pos2d> cell;  ///< the cell to aim for, once a container is found
  u32 owner_serial = 0;             ///< the container to offer, while it is still in the running
  u8 slot = 0;
  bool try_container = false;

  if ( const auto* from_ground = std::get_if<FromGround>( &_origin ) )
  {
    ground = from_ground->pos;
  }
  else if ( const auto* from_container = std::get_if<FromContainer>( &_origin ) )
  {
    owner_serial = from_container->serial;
    cell = from_container->grid;
    slot = from_container->slot;
    try_container = true;
  }
  else if ( const auto* from_corpse = std::get_if<FromCorpse>( &_origin ) )
  {
    owner_serial = from_corpse->serial;
    cell = from_corpse->grid;
    slot = from_corpse->slot;
    try_container = true;
  }

  if ( const auto* from_layer = std::get_if<FromLayer>( &_origin ) )
  {
    if ( auto equipped_chr = system_find_mobile( from_layer->serial ) )
    {
      if ( equipped_chr->equippable( _item ) && _item->check_equiptest_scripts( equipped_chr ) &&
           _item->check_equip_script( equipped_chr, false ) )
      {
        if ( _item->orphan() )
          return;
        // is it possible the character doesn't exist? no, it's my character doing the undoing.
        if ( Items::relocate( *_item, Items::Equipped{ equipped_chr, _item->tile_layer } ) )
        {
          send_wornitem_to_inrange( equipped_chr, _item );
          return;
        }
      }
    }

    if ( _item->orphan() )
      return;
    // The layer would not take it back, so carry on at the containers. A layer origin names no
    // container to offer and no cell to aim for -- it came off a layer, not out of a gump -- so
    // owner_serial and cell stay empty and the backpack gets first refusal.
    try_container = true;
    ground = chr->pos().xyz();
    realm = chr->stored_realm();
  }

  // A corpse layer is returned to the same way an ordinary container is: the backpack is still
  // offered first, and only the fallback below puts the item back where it came from -- which is
  // the one branch where the layer matters.
  if ( try_container )
  {
    // First attempt to place the item in the player's backpack.
    UContainer* container = nullptr;
    if ( !_item->no_drop() )
    {
      container = chr->backpack();
      if ( !container || !container->can_add( *_item ) ||
           !container->can_insert_add_item( chr, UContainer::MT_PLAYER, _item ) )
        container = nullptr;
      if ( _item->orphan() )
        return;
    }
    // Attempt to put it back in the original container.
    if ( !container &&
         ( !Core::settingsManager.ssopt.undo_get_item_drop_here || _item->no_drop() ) )
    {
      auto* orig_obj = system_find_object( owner_serial );
      if ( orig_obj && orig_obj->isa( UOBJ_CLASS::CLASS_CONTAINER ) )
      {
        if ( _item->no_drop() || chr->can_moveanydist() ||
             !Core::settingsManager.ssopt.undo_get_item_enable_range_check ||
             chr->can_reach( orig_obj, Core::settingsManager.ssopt.default_accessible_range ) )
        {
          container = static_cast<UContainer*>( orig_obj );
          if ( !container->can_add( *_item ) ||
               !container->can_insert_add_item( chr, UContainer::MT_PLAYER, _item ) )
            container = nullptr;
        }
      }
      if ( _item->orphan() )
        return;
    }

    // No drop item has not returned to original container, place the item in the player's backpack.
    if ( !container && _item->no_drop() )
    {
      container = chr->backpack();
      if ( !container || !container->can_add( *_item ) ||
           !container->can_insert_add_item( chr, UContainer::MT_PLAYER, _item ) )
        container = nullptr;
      if ( _item->orphan() )
        return;
    }

    if ( container )
    {
      u8 newSlot = slot ? slot : 1;
      if ( container->can_add_to_slot( newSlot ) )
      {
        const Pos2d where = ( cell && container->is_legal_posn( *cell ) )
                                ? *cell
                                : container->get_random_location();
        Items::Location target = Items::InContainer{ container, where, newSlot };
        if ( came_off_corpse_layer( container ) )
          target = Items::OnCorpse{ static_cast<UCorpse*>( container ), where, newSlot,
                                    _item->tile_layer };
        if ( Items::relocate( *_item, target ) )
        {
          update_item_to_inrange( _item );
          container->on_insert_add_item( chr, UContainer::MT_PLAYER, _item );
          return;
        }
      }
    }
    ground = chr->pos3d();
    realm = chr->stored_realm();
  }

  // Only an item that was lying in the world has a spot of its own left to go back to. Every other
  // rung above has already settled on the character's own position, which is trivially within
  // range of the character and in the character's realm, so the check below has nothing to say
  // about them.
  const auto* from_ground = std::get_if<FromGround>( &_origin );

  if ( Core::settingsManager.ssopt.undo_get_item_drop_here )
  {
    ground = chr->pos3d();
    realm = chr->stored_realm();
  }
  else if ( from_ground != nullptr && !chr->can_moveanydist() &&
            Core::settingsManager.ssopt.undo_get_item_enable_range_check )
  {
    realm = Core::find_realm( from_ground->realm );
    if ( realm == nullptr ||
         !chr->in_range( Pos4d( ground, realm ),
                         Core::settingsManager.ssopt.default_accessible_range ) )
    {
      ground = chr->pos3d();
      realm = chr->stored_realm();
    }
  }

  // The (local variable) `realm` will be set in case of error from above.
  if ( realm == nullptr )
  {
    // Try finding the realm the item was lying in, by name.
    if ( from_ground != nullptr )
      realm = Core::find_realm( from_ground->realm );

    // If the realm is not found, set it to the position of the character.
    if ( realm == nullptr )
    {
      realm = chr->stored_realm();
      ground = chr->pos3d();
    }
  }

  // Last resort: nowhere else would take it, so the ground has to. Nothing can refuse it here —
  // the item is detached and the realm above is never null. place_at carries the realm down to the
  // contents if it changed.
  if ( !Items::place_at( *_item, Pos4d( ground, realm ) ) )
    return;

  send_item_to_inrange( _item );

  // Need to explicitly send remove_object to chr if realms mismatch. Scenario:
  //
  // 1. pick up item on ground in `britannia`
  // 2. switch to `shadow-britannia`
  // 3. drop item in invalid location
  //
  // Because of the failure in step 3, core will send packet 0x27 Item Move
  // Failure via `Core::send_item_move_failure`, and the client will
  // automatically place it back at old `x,y` location. The item is in
  // `britannia`, but character is still in `shadow-britannia`, so the item will
  // appear on the ground in the client.
  if ( chr->stored_realm() != realm )
  {
    send_remove_object( chr->client, _item );
  }
}
}  // namespace Pol::Core
