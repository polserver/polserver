/** @file
 *
 * @par History
 * - 2007/06/17 Shinigami: added config.world_data_path
 * - 2008/12/17 MuadDib:   Added item.tile_layer - returns layer entry from tiledata/tiles.cfg
 * - 2009/09/14 MuadDib:   Added slot support to equip_loaded_item() and add_loaded_item()
 * - 2009/09/18 MuadDib:   Spellbook rewrite to deal with only bits, not scrolls inside them.
 */


#include <stddef.h>
#include <string>

#include "clib/cfgelem.h"
#include "clib/cfgfile.h"
#include "clib/fileutil.h"
#include "clib/logfacility.h"
#include "clib/rawtypes.h"
#include "clib/strutil.h"
#include "clib/timer.h"
#include "plib/clidata.h"
#include "plib/systemstate.h"
#include "pol/containr.h"
#include "pol/fnsearch.h"
#include "pol/globals/object_storage.h"
#include "pol/globals/state.h"
#include "pol/item/item.h"
#include "pol/loaddata.h"
#include "pol/mobile/charactr.h"
#include "pol/mobile/corpse.h"
#include "pol/objecthash.h"
#include "pol/polclass.h"
#include "pol/spelbook.h"
#include "pol/uobject.h"


namespace Pol::Core
{
void defer_item_insertion( Items::Item* item, pol_serial_t container_serial, u8 saved_layer )
{
  objStorageManager.deferred_insertions.insert(
      std::make_pair( container_serial, DeferredInsertion{ item, saved_layer } ) );
}

void insert_deferred_items()
{
  if ( objStorageManager.deferred_insertions.empty() )
    return;

  int num_until_dot = 1000;
  unsigned int nobjects = 0;
  Tools::Timer<> timer;

  INFO_PRINT( "  deferred inserts:" );

  for ( auto& deferred_insertion : objStorageManager.deferred_insertions )
  {
    if ( --num_until_dot == 0 )
    {
      INFO_PRINT( "." );
      num_until_dot = 1000;
    }

    pol_serial_t container_serial = deferred_insertion.first;
    UObject* obj = deferred_insertion.second.obj;

    if ( IsCharacter( container_serial ) )
    {
      Mobile::Character* chr = system_find_mobile( container_serial );
      Items::Item* item = static_cast<Items::Item*>( obj );
      if ( chr != nullptr )
      {
        equip_loaded_item( chr, item );
      }
      else
      {
        ERROR_PRINTLN(
            "Item {:#x} is supposed to be on Character {:#x}, but that character cannot be ",
            item->serial, container_serial );

        // Austin - Aug. 10, 2006
        // Removes the object if ignore_load_errors is enabled and the character can't be found.
        if ( !Plib::systemstate.config.ignore_load_errors )
          throw std::runtime_error( "Data file integrity error" );

        ERROR_PRINTLN( "Ignore load errors enabled. Removing object." );
        obj->destroy();
      }
    }
    else
    {
      Items::Item* cont_item = system_find_item( container_serial );
      Items::Item* item = static_cast<Items::Item*>( obj );
      if ( cont_item != nullptr )
      {
        add_loaded_item( cont_item, item, deferred_insertion.second.saved_layer );
      }
      else
      {
        ERROR_PRINTLN(
            "Item {:#x} is supposed to be in container {:#x}, but that container cannot be found.",
            item->serial, container_serial );

        // Austin - Aug. 10, 2006
        // Removes the object if ignore_load_errors is enabled and the character can't be found.
        if ( !Plib::systemstate.config.ignore_load_errors )
          throw std::runtime_error( "Data file integrity error" );

        ERROR_PRINTLN( "Ignore load errors enabled. Removing object." );
        obj->destroy();
      }
    }
    ++nobjects;
  }
  timer.stop();
  INFO_PRINTLN( " {} elements in {} ms.", nobjects, timer.ellapsed() );

  objStorageManager.deferred_insertions.clear();
}

void equip_loaded_item( Mobile::Character* chr, Items::Item* item )
{
  item->tile_layer = Plib::tilelayer( item->graphic );  // adjust for tiledata changes

  // The equip script above runs after equippable() has already said yes, and is free to make the
  // item unequippable again before it is actually worn. Character::equip opens with a
  // passert_r( equippable( item ) ), so that used to abort the server part-way through reading the
  // world; relocate refuses and the item goes to the backpack like any other item that cannot be
  // worn.
  if ( chr->equippable( item ) && item->check_equiptest_scripts( chr, true ) &&
       item->check_equip_script( chr, true ) &&
       !item->orphan() &&  // dave added 1/28/3, item might be destroyed in RTC script
       Items::relocate( *item, Items::Equipped{ chr, item->tile_layer } ) )
  {
    item->clear_dirty();  // equipping sets dirty
    return;
  }

  ERROR_PRINTLN(
      "Item {:#x} is supposed to be equipped on Character {:#x}, but is not 'equippable' on that "
      "character.",
      item->serial, chr->serial );
  UContainer* bp = chr->backpack();
  if ( bp )
  {
    stateManager.gflag_enforce_container_limits = false;
    bool canadd = bp->can_add( *item );
    u8 slotIndex = item->slot_index();
    bool add_to_slot = bp->can_add_to_slot( slotIndex );
    if ( canadd && add_to_slot && item->slot_index( slotIndex ) &&
         Items::relocate( *item, Items::InContainer{ bp, bp->get_random_location(), slotIndex } ) )
    {
      // leaving dirty
      stateManager.gflag_enforce_container_limits = true;
      ERROR_PRINTLN( "I'm so cool, I put it in the character's backpack!" );
      return;
    }

    stateManager.gflag_enforce_container_limits = true;
    ERROR_PRINTLN( "Tried to put it in the character's backpack, but it wouldn't fit." );
  }
  else
  {
    ERROR_PRINTLN(
        "Tried to put it in the character's backpack, but there isn't one.  That's naughty..." );
  }
  throw std::runtime_error( "Data file integrity error" );
}

namespace
{
/**
 * Puts a loaded item back on the layer it was rendered on, if it was on one.
 *
 * A corpse shows its owner's equipment from a layer list that is not saved: all that survives a
 * restart is each item's own Layer line, so the list has to be rebuilt here or the corpse comes
 * back with nothing on it. Which layer an item renders on today is a tiledata question, so the
 * saved value only decides whether it was worn at all.
 *
 * Returns false for anything that is ordinary contents -- the swallowed backpack's items, say --
 * and for the item that loses a layer it now shares with another, which tiledata changes can
 * produce. Those are still legitimate corpse contents and the caller inserts them normally.
 */
bool add_loaded_item_to_layer( UContainer* cont, Items::Item* item, u8 slot, u8 saved_layer )
{
  if ( saved_layer == 0 || !cont->script_isa( POLCLASS_CORPSE ) ||
       !Items::valid_equip_layer( item ) )
    return false;

  UCorpse* corpse = static_cast<UCorpse*>( cont );
  return Items::relocate( *item, Items::OnCorpse{ corpse, item->pos2d(), slot, item->tile_layer } );
}
}  // namespace

void add_loaded_item( Items::Item* cont_item, Items::Item* item, u8 saved_layer )
{
  if ( cont_item->isa( UOBJ_CLASS::CLASS_CONTAINER ) )
  {
    UContainer* cont = static_cast<UContainer*>( cont_item );

    // Convert spellbook to use bitwise system, not scrolls.
    if ( cont->script_isa( POLCLASS_SPELLBOOK ) )
    {
      // if can't add, means spell already there.
      if ( !cont->can_add( *item ) )
      {
        item->destroy();
        return;
      }
      // this is an oldschool book, oldschool contents. We need to create the bitwise
      // and handle for the first time before destroying the scrolls.
      Spellbook* book = static_cast<Spellbook*>( cont );

      u16 spellnum =
          USpellScroll::convert_objtype_to_spellnum( item->objtype_, book->spell_school );
      u8 spellslot = spellnum & 7;
      if ( spellslot == 0 )
        spellslot = 8;
      book->bitwise_contents[( spellnum - 1 ) >> 3] |= 1 << ( spellslot - 1 );
      item->destroy();
      return;
    }

    stateManager.gflag_enforce_container_limits = false;
    bool canadd = cont->can_add( *item );
    u8 slotIndex = item->slot_index();
    bool add_to_slot = cont->can_add_to_slot( slotIndex );
    if ( !canadd )
    {
      ERROR_PRINTLN( "Can't add Item {:#x} to container {:#x}", item->serial, cont->serial );
      throw std::runtime_error( "Data file error" );
    }

    if ( !add_to_slot || !item->slot_index( slotIndex ) )
    {
      ERROR_PRINTLN( "Can't add Item {:#x} to container {:#x} at slot {:#x}", item->serial,
                     cont->serial, slotIndex );
      throw std::runtime_error( "Data file error" );
    }

    // The loader used to reach into the container itself. Going through relocate also means the
    // world file no longer gets the benefit of the doubt on two things UContainer::add would
    // simply have done: adding to a destroyed container, which add answers with a passert, and
    // putting a container inside itself, which nothing checked at all.
    if ( !add_loaded_item_to_layer( cont, item, slotIndex, saved_layer ) &&
         !Items::relocate( *item, Items::InContainer{ cont, item->pos2d(), slotIndex } ) )
    {
      ERROR_PRINTLN( "Can't add Item {:#x} to container {:#x}", item->serial, cont->serial );
      throw std::runtime_error( "Data file error" );
    }
    item->clear_dirty();  // adding sets dirty

    stateManager.gflag_enforce_container_limits = true;

    // if (new_parent_cont)
    //  parent_conts.push( cont );
    // if (item->isa( UObject::CLASS_CONTAINER ))
    //  parent_conts.push( static_cast<UContainer*>(item) );
  }
  else
  {
    INFO_PRINTLN( "Container type {:#x} contains items, but is not a container class",
                  cont_item->objtype_ );
    throw std::runtime_error( "Config file error" );
  }
}
}  // namespace Pol::Core
