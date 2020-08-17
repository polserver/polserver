/** @file
 *
 * @par History
 * - 2005/12/06 Shinigami: fixed <un-/signed>(z)-bug inside internal_MoveCharacter() to fix z<0
 * problem
 */


/*
  UEMOD4.CPP
  Handles moving objects within a realm and from realm to realm.
*/

#include <stddef.h>
#include <string>

#include "../../bscript/berror.h"
#include "../../bscript/bobject.h"
#include "../../bscript/impstr.h"
#include "../../plib/uconst.h"
#include "../containr.h"
#include "../core.h"
#include "../item/item.h"
#include "../mobile/charactr.h"
#include "../multi/boat.h"
#include "../polclass.h"
#include "realms/realms.h"
#include "realms/realm.h"
#include "../ufunc.h"
#include "../uobject.h"
#include "../uworld.h"
#include "uomod.h"

namespace Pol
{
namespace Module
{
using namespace Bscript;
using namespace Mobile;
using namespace Core;
using namespace Items;
BObjectImp* UOExecutorModule::mf_MoveObjectToLocation( /*object, x, y, z, realm, flags*/ )
{
  UObject* obj;
  unsigned short x, y;
  short z;
  int flags;
  const String* realm_name;

  // Initialize variables
  if ( !( getUObjectParam( 0, obj ) && getParam( 1, x ) && getParam( 2, y ) &&
          getParam( 3, z, ZCOORD_MIN, ZCOORD_MAX ) && getStringParam( 4, realm_name ) &&
          getParam( 5, flags ) ) )
  {
    return new BError( "Invalid parameter" );
  }
  Realms::Realm* realm = find_realm( realm_name->value() );
  if ( !realm )
    return new BError( "Realm not found." );
  else if ( !realm->valid( x, y, z ) )
    return new BError( "Invalid coordinates for realm." );

  if ( obj->script_isa( POLCLASS_MOBILE ) )
    return internal_MoveCharacter( static_cast<Character*>( obj ), x, y, z, flags, realm );
  else if ( obj->script_isa( POLCLASS_BOAT ) )
    return internal_MoveBoat( static_cast<Multi::UBoat*>( obj ), x, y, z, flags, realm );
  else if ( obj->script_isa( POLCLASS_MULTI ) )
    return new BError( "Can't move multis at this time." );
  else if ( obj->script_isa( POLCLASS_CONTAINER ) )
    return internal_MoveContainer( static_cast<UContainer*>( obj ), x, y, z, flags, realm );
  else if ( obj->script_isa( POLCLASS_ITEM ) )
    return internal_MoveItem( static_cast<Item*>( obj ), x, y, z, flags, realm );
  else
    return new BError( "Can't handle that object type." );
}

BObjectImp* UOExecutorModule::internal_MoveCharacter( Character* chr, xcoord x, ycoord y, zcoord z,
                                                      int flags, Realms::Realm* newrealm )
{
  short newz;
  Multi::UMulti* supporting_multi = nullptr;
  Item* walkon_item = nullptr;

  if ( !( flags & MOVEITEM_FORCELOCATION ) )
  {
    if ( newrealm != nullptr )
    {
      if ( !newrealm->walkheight( x, y, z, &newz, &supporting_multi, &walkon_item, true,
                                  chr->movemode ) )
        return new BError( "Can't go there" );
    }
  }
  Realms::Realm* oldrealm = chr->realm;

  bool ok;
  if ( newrealm == nullptr || oldrealm == newrealm )
  {  // Realm is staying the same, just changing X Y Z coordinates.
    ok = move_character_to( chr, x, y, z, flags, nullptr );
  }
  else
  {  // Realm and X Y Z change.

    // Notify NPCs in the old realm that the player left the realm.
    oldrealm->notify_left( *chr );

    send_remove_character_to_nearby( chr );
    if ( chr->client != nullptr )
      remove_objects_inrange( chr->client );
    chr->realm = newrealm;
    chr->realm_changed();
    ok = move_character_to( chr, x, y, z, flags, oldrealm );
  }

  if ( ok )
    return new BLong( 1 );
  else
    return new BError( "Can't go there" );
}

BObjectImp* UOExecutorModule::internal_MoveBoat( Multi::UBoat* boat, xcoord x, ycoord y, zcoord z,
                                                 int flags, Realms::Realm* newrealm )
{
  Realms::Realm* oldrealm = boat->realm;
  {  // local scope for reg/unreg guard
    Multi::UBoat::BoatMoveGuard registerguard( boat );
    if ( !boat->navigable( boat->multidef(), x, y, z, newrealm ) )
    {
      return new BError( "Position indicated is impassable" );
    }
  }
  if ( newrealm !=
       boat->realm )  // boat->move_xy removes on xy change so only realm change check is needed
  {
    send_remove_object_to_inrange( boat );
  }
  boat->realm = newrealm;

  s8 deltaz = static_cast<s8>( z - boat->z );
  boat->z = (s8)z;
  boat->adjust_traveller_z( deltaz );
  boat->realm_changed();
  bool ok = boat->move_xy( x, y, flags, oldrealm );
  return new BLong( ok );
}

BObjectImp* UOExecutorModule::internal_MoveContainer( UContainer* container, xcoord x, ycoord y,
                                                      zcoord z, int flags, Realms::Realm* newrealm )
{
  Realms::Realm* oldrealm = container->realm;

  BObjectImp* ok = internal_MoveItem( static_cast<Item*>( container ), x, y, z, flags, newrealm );
  // Check if container was successfully moved to a new realm and update contents.
  if ( !ok->isa( BObjectImp::OTError ) )
  {
    if ( newrealm != nullptr && oldrealm != newrealm )
      container->for_each_item( setrealm, (void*)newrealm );
  }

  return ok;
}

BObjectImp* UOExecutorModule::internal_MoveItem( Item* item, xcoord x, ycoord y, zcoord z,
                                                 int flags, Realms::Realm* newrealm )
{
  ItemRef itemref( item );  // dave 1/28/3 prevent item from being destroyed before function ends
  if ( !( flags & MOVEITEM_IGNOREMOVABLE ) && !item->movable() )
  {
    Character* chr = controller_.get();
    if ( chr == nullptr || !chr->can_move( item ) )
      return new BError( "That is immobile" );
  }
  if ( item->inuse() && !is_reserved_to_me( item ) )
  {
    return new BError( "That item is being used." );
  }

  Realms::Realm* oldrealm = item->realm;
  item->realm = newrealm;
  if ( !item->realm->valid( x, y, z ) )
  {  // Should probably have checked this already.
    item->realm = oldrealm;
    std::string message = "Location (" + Clib::tostring( x ) + "," + Clib::tostring( y ) + "," +
                          Clib::tostring( z ) + ") is out of bounds";
    return new BError( message );
  }

  Multi::UMulti* multi = nullptr;
  if ( flags & MOVEITEM_FORCELOCATION )
  {
    short newz;
    Item* walkon;
    item->realm->walkheight( x, y, z, &newz, &multi, &walkon, true, Plib::MOVEMODE_LAND );
    // note that newz is ignored...
  }
  else
  {
    short newz;
    Item* walkon;
    if ( !item->realm->walkheight( x, y, z, &newz, &multi, &walkon, true, Plib::MOVEMODE_LAND ) )
    {
      item->realm = oldrealm;
      return new BError( "Invalid location selected" );
    }
    z = newz;
  }

  if ( item->container != nullptr )
  {
    // DAVE added this 12/04, call can/onRemove scripts for the old container
    UObject* oldroot = item->toplevel_owner();
    UContainer* oldcont = item->container;
    Character* chr_owner = oldcont->GetCharacterOwner();
    if ( chr_owner == nullptr )
      if ( controller_.get() != nullptr )
        chr_owner = controller_.get();

    // dave changed 1/26/3 order of scripts to call. added unequip/test script call
    if ( !oldcont->check_can_remove_script( chr_owner, item, UContainer::MT_CORE_MOVED ) )
    {
      item->realm = oldrealm;
      return new BError( "Could not remove item from its container." );
    }
    else if ( item->orphan() )  // dave added 1/28/3, item might be destroyed in RTC script
    {
      return new BError( "Item was destroyed in CanRemove script" );
    }

    if ( !item->check_unequiptest_scripts() || !item->check_unequip_script() )
    {
      item->realm = oldrealm;
      return new BError( "Item cannot be unequipped" );
    }
    if ( item->orphan() )  // dave added 1/28/3, item might be destroyed in RTC script
      return new BError( "Item was destroyed in Equip script" );

    oldcont->on_remove( chr_owner, item, UContainer::MT_CORE_MOVED );
    if ( item->orphan() )  // dave added 1/28/3, item might be destroyed in RTC script
    {
      return new BError( "Item was destroyed in OnRemove script" );
    }

    item->set_dirty();

    item->extricate();

    // wherever it was, it wasn't in the world/on the ground
    item->x = oldroot->x;
    item->y = oldroot->y;
    // move_item calls MoveItemWorldLocation, so this gets it
    // in the right place to start with.
    item->realm = oldrealm;
    add_item_to_world( item );
    item->realm = newrealm;
  }

  move_item( item, x, y, static_cast<signed char>( z ), oldrealm );

  if ( multi != nullptr )
  {
    multi->register_object( item );
  }
  return new BLong( 1 );
}
}  // namespace Module
}  // namespace Pol
