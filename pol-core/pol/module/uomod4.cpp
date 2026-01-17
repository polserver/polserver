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
#include "../../clib/clib.h"
#include "../../plib/uconst.h"
#include "../containr.h"
#include "../core.h"
#include "../item/item.h"
#include "../mobile/charactr.h"
#include "../multi/boat.h"
#include "../multi/multidef.h"
#include "../polclass.h"
#include "../ufunc.h"
#include "../uobject.h"
#include "../uworld.h"
#include "realms/realm.h"
#include "realms/realms.h"
#include "uomod.h"


namespace Pol::Module
{
using namespace Bscript;
using namespace Mobile;
using namespace Core;
using namespace Items;
BObjectImp* UOExecutorModule::mf_MoveObjectToLocation( /*object, x, y, z, realm, flags*/ )
{
  UObject* obj;
  Core::Pos4d pos;
  int flags;

  // Initialize variables
  if ( !( getUObjectParam( 0, obj ) && getPos4dParam( 1, 2, 3, 4, &pos ) && getParam( 5, flags ) ) )
    return new BError( "Invalid parameter" );

  if ( obj->script_isa( POLCLASS_MOBILE ) )
    return internal_MoveCharacter( static_cast<Character*>( obj ), pos, flags );
  if ( obj->script_isa( POLCLASS_BOAT ) )
    return internal_MoveBoat( static_cast<Multi::UBoat*>( obj ), pos, flags );
  if ( obj->script_isa( POLCLASS_MULTI ) )
    return new BError( "Can't move multis at this time." );
  else if ( obj->script_isa( POLCLASS_CONTAINER ) )
    return internal_MoveContainer( static_cast<UContainer*>( obj ), pos, flags );
  else if ( obj->script_isa( POLCLASS_ITEM ) )
    return internal_MoveItem( static_cast<Item*>( obj ), pos, flags );
  return new BError( "Can't handle that object type." );
}

BObjectImp* UOExecutorModule::internal_MoveCharacter( Character* chr, const Core::Pos4d& newpos,
                                                      int flags )
{
  short newz;
  Multi::UMulti* supporting_multi = nullptr;
  Item* walkon_item = nullptr;

  if ( chr->is_piloting_boat() )
  {
    return new BError( "That character is piloting a boat and cannot be moved" );
  }

  if ( !( flags & MOVEITEM_FORCELOCATION ) )
  {
    if ( !newpos.realm()->walkheight( newpos.xy(), newpos.z(), &newz, &supporting_multi,
                                      &walkon_item, true, chr->movemode ) )
      return new BError( "Can't go there" );
  }
  if ( move_character_to( chr, newpos, flags ) )
    return new BLong( 1 );
  return new BError( "Can't go there" );
}

BObjectImp* UOExecutorModule::internal_MoveBoat( Multi::UBoat* boat, const Core::Pos4d& newpos,
                                                 int flags )
{
  // checked also in move_to, but to keep the error message leave it here
  if ( !( flags & MOVEITEM_FORCELOCATION ) )
  {
    Multi::UBoat::BoatMoveGuard registerguard( boat );
    if ( !boat->navigable( boat->multidef(), newpos ) )
      return new BError( "Position indicated is impassable" );
  }
  // always check if boat would fit into realm
  else if ( !boat->can_fit_at_location( boat->multidef(), newpos ) )
    return new BError( "Boat does not fit into map" );
  bool ok = boat->move_to( newpos, flags );
  return new BLong( ok );
}

BObjectImp* UOExecutorModule::internal_MoveContainer( UContainer* container,
                                                      const Core::Pos4d& newpos, int flags )
{
  Realms::Realm* newrealm = newpos.realm();
  Realms::Realm* oldrealm = container->realm();

  BObjectImp* ok = internal_MoveItem( static_cast<Item*>( container ), newpos, flags );
  // Check if container was successfully moved to a new realm and update contents.
  if ( !ok->isa( BObjectImp::OTError ) )
  {
    // TODO POS should be removed
    if ( newrealm != nullptr && oldrealm != newrealm )
      container->for_each_item( setrealm, (void*)newrealm );
  }

  return ok;
}

BObjectImp* UOExecutorModule::internal_MoveItem( Item* item, Core::Pos4d newpos, int flags )
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

  Multi::UMulti* multi = nullptr;
  if ( flags & MOVEITEM_FORCELOCATION )
  {
    short newz;
    Item* walkon;
    newpos.realm()->walkheight( newpos.xy(), newpos.z(), &newz, &multi, &walkon, true,
                                Plib::MOVEMODE_LAND );
    // note that newz is ignored...
  }
  else
  {
    short newz;
    Item* walkon;
    if ( !newpos.realm()->walkheight( newpos.xy(), newpos.z(), &newz, &multi, &walkon, true,
                                      Plib::MOVEMODE_LAND ) )
      return new BError( "Invalid location selected" );
    newpos.z( Clib::clamp_convert<s8>( newz ) );
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
      return new BError( "Could not remove item from its container." );
    }
    if ( item->orphan() )  // dave added 1/28/3, item might be destroyed in RTC script
    {
      return new BError( "Item was destroyed in CanRemove script" );
    }

    if ( !item->check_unequiptest_scripts() || !item->check_unequip_script() )
    {
      return new BError( "Item cannot be unequipped" );
    }
    if ( item->orphan() )  // dave added 1/28/3, item might be destroyed in RTC script
      return new BError( "Item was destroyed in Equip script" );

    oldcont->on_remove( chr_owner, item, UContainer::MT_CORE_MOVED );
    if ( item->orphan() )  // dave added 1/28/3, item might be destroyed in RTC script
    {
      return new BError( "Item was destroyed in OnRemove script" );
    }

    item->extricate();
    //  wherever it was, it wasn't in the world/on the ground
    item->setposition( oldroot->toplevel_pos() );
    if ( item->realm() == nullptr )
      item->setposition( Pos4d( item->pos3d(), newpos.realm() ) );
    // move_item calls MoveItemWorldLocation, so this gets it
    // in the right place to start with.
    add_item_to_world( item );
  }
  const Pos4d oldpos = item->toplevel_pos();
  item->setposition( newpos );
  move_item( item, oldpos );

  if ( multi != nullptr )
  {
    multi->register_object( item );
  }
  return new BLong( 1 );
}
}  // namespace Pol::Module
