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
#include "../realms.h"
#include "../realms/realm.h"
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
  Core::Pos4d pos;
  int flags;

  if ( !( getUObjectParam( 0, obj ) && getPos4dParam( 1, 2, 3, 4, &pos ) && getParam( 5, flags ) ) )
  {
    return new BError( "Invalid parameter" );
  }
  if ( obj->script_isa( POLCLASS_MOBILE ) )
    return internal_MoveCharacter( static_cast<Character*>( obj ), pos, flags );
  else if ( obj->script_isa( POLCLASS_BOAT ) )
    return internal_MoveBoat( static_cast<Multi::UBoat*>( obj ), pos, flags );
  else if ( obj->script_isa( POLCLASS_MULTI ) )
    return new BError( "Can't move multis at this time." );
  else if ( obj->script_isa( POLCLASS_ITEM ) )
    return internal_MoveItem( static_cast<Item*>( obj ), pos, flags );
  else
    return new BError( "Can't handle that object type." );
}

BObjectImp* UOExecutorModule::internal_MoveCharacter( Character* chr, Core::Pos4d newpos,
                                                      int flags )
{
  short newz;
  if ( !( flags & MOVEITEM_FORCELOCATION ) )
  {
    if ( newpos.realm() != nullptr )
    {
      if ( !newpos.realm()->walkheight( newpos.xy(), newpos.z(), &newz, nullptr, nullptr, true,
                                        chr->movemode ) )
        return new BError( "Can't go there" );
    }
  }
  if ( newpos.realm() == nullptr )
    newpos = Core::Pos4d( newpos.xyz(), chr->realm() );

  bool ok = move_character_to( chr, newpos, flags );
  if ( ok )
    return new BLong( 1 );
  else
    return new BError( "Can't go there" );
}

BObjectImp* UOExecutorModule::internal_MoveBoat( Multi::UBoat* boat, Core::Pos4d newpos, int flags )
{
  {  // local scope for reg/unreg guard
    Multi::UBoat::BoatMoveGuard registerguard( boat );
    if ( !boat->navigable( boat->multidef(), newpos ) )
    {
      return new BError( "Position indicated is impassable" );
    }
  }

  return new BLong( boat->move_to( newpos, flags ) );
}

BObjectImp* UOExecutorModule::internal_MoveItem( Item* item, Core::Pos4d newpos, int flags )
{
  ItemRef itemref( item );  // dave 1/28/3 prevent item from being destroyed before function ends
  Core::Pos4d oldpos( item->toplevel_pos() );
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
    newpos.realm()->walkheight( newpos.xy(), newpos.z(), &newz, &multi, nullptr, true,
                                Plib::MOVEMODE_LAND );
    // note that newz is ignored...
  }
  else
  {
    short newz;
    if ( !newpos.realm()->walkheight( newpos.xy(), newpos.z(), &newz, &multi, nullptr, true,
                                      Plib::MOVEMODE_LAND ) )
    {
      return new BError( "Invalid location selected" );
    }
    newpos.z( newz );
  }

  if ( item->container != nullptr )
  {
    // DAVE added this 12/04, call can/onRemove scripts for the old container
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
    else if ( item->orphan() )  // dave added 1/28/3, item might be destroyed in RTC script
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

    item->set_dirty();

    item->extricate();

    // wherever it was, it wasn't in the world/on the ground
    item->setposition( oldpos );
    // move_item calls MoveItemWorldLocation, so this gets it
    // in the right place to start with.
    add_item_to_world( item );  // TODO: we have to add it first....
  }

  move_item( item, newpos, oldpos );

  if ( multi != nullptr )
  {
    multi->register_object( item );
  }
  return new BLong( 1 );
}
}  // namespace Module
}  // namespace Pol
