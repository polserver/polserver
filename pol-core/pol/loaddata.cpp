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

#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/fileutil.h"
#include "../clib/logfacility.h"
#include "../clib/rawtypes.h"
#include "../clib/strutil.h"
#include "../clib/timer.h"
#include "../plib/clidata.h"
#include "../plib/systemstate.h"
#include "containr.h"
#include "fnsearch.h"
#include "globals/object_storage.h"
#include "globals/state.h"
#include "item/item.h"
#include "loaddata.h"
#include "mobile/charactr.h"
#include "objecthash.h"
#include "polclass.h"
#include "spelbook.h"
#include "uobject.h"

namespace Pol
{
namespace Core
{
void defer_item_insertion( Items::Item* item, pol_serial_t container_serial )
{
  objStorageManager.deferred_insertions.insert( std::make_pair( container_serial, item ) );
}

void insert_deferred_items()
{
  if ( objStorageManager.deferred_insertions.empty() )
    return;

  int num_until_dot = 1000;
  unsigned int nobjects = 0;
  Tools::Timer<> timer;

  INFO_PRINT( "  deferred inserts:" );

  for ( DeferList::iterator itr = objStorageManager.deferred_insertions.begin();
        itr != objStorageManager.deferred_insertions.end(); ++itr )
  {
    if ( --num_until_dot == 0 )
    {
      INFO_PRINT( "." );
      num_until_dot = 1000;
    }

    pol_serial_t container_serial = ( *itr ).first;
    UObject* obj = ( *itr ).second;

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
        else
        {
          ERROR_PRINTLN( "Ignore load errors enabled. Removing object." );
          obj->destroy();
        }
      }
    }
    else
    {
      Items::Item* cont_item = system_find_item( container_serial );
      Items::Item* item = static_cast<Items::Item*>( obj );
      if ( cont_item != nullptr )
      {
        add_loaded_item( cont_item, item );
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
        else
        {
          ERROR_PRINTLN( "Ignore load errors enabled. Removing object." );
          obj->destroy();
        }
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
  item->layer = Plib::tilelayer( item->graphic );  // adjust for tiledata changes
  item->tile_layer = item->layer;                  // adjust for tiledata changes

  if ( chr->equippable( item ) && item->check_equiptest_scripts( chr, true ) &&
       item->check_equip_script( chr, true ) &&
       !item->orphan() )  // dave added 1/28/3, item might be destroyed in RTC script
  {
    chr->equip( item );
    item->clear_dirty();  // equipping sets dirty
    return;
  }
  else
  {
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
      if ( canadd && add_to_slot && item->slot_index( slotIndex ) )
      {
        bp->add_at_random_location( item );
        // leaving dirty
        stateManager.gflag_enforce_container_limits = true;
        ERROR_PRINTLN( "I'm so cool, I put it in the character's backpack!" );
        return;
      }
      else
      {
        stateManager.gflag_enforce_container_limits = true;
        ERROR_PRINTLN( "Tried to put it in the character's backpack, but it wouldn't fit." );
      }
    }
    else
    {
      ERROR_PRINTLN(
          "Tried to put it in the character's backpack, but there isn't one.  That's naughty..." );
    }
    throw std::runtime_error( "Data file integrity error" );
  }
}

void add_loaded_item( Items::Item* cont_item, Items::Item* item )
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

    cont->add( item, item->pos2d() );
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
}  // namespace Core
}  // namespace Pol
