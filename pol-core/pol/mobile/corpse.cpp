/** @file
 *
 * @par History
 * - 2009/08/07 MuadDib:   Added layer_list_ and functions like WornItems to corpse class. Used to
 * handle showing equippable items
 *                         on a corpse.
 */


#include "corpse.h"

#include <stddef.h>

#include "../../bscript/executor.h"
#include "../../clib/cfgelem.h"
#include "../../clib/passert.h"
#include "../../clib/streamsaver.h"
#include "../containr.h"
#include "../globals/uvars.h"
#include "../layers.h"
#include "../syshookscript.h"
#include "../ufunc.h"
#include "../uobject.h"


namespace Pol
{
namespace Core
{
UCorpse::UCorpse( const Items::ContainerDesc& descriptor )
    : UContainer( descriptor ), corpsetype( 0 ), ownerserial( 0 )
{
  movable( false );
  can_equip_list_.resize( HIGHEST_LAYER + 1 );
}

const Core::ItemRef& UCorpse::GetItemOnLayer( unsigned idx ) const
{
  passert( Items::valid_equip_layer( idx ) );
  return can_equip_list_.at( idx );
}

bool UCorpse::take_contents_to_grave() const
{
  return flags_.get( OBJ_FLAGS::CONTENT_TO_GRAVE );
}

void UCorpse::take_contents_to_grave( bool newvalue )
{
  flags_.change( OBJ_FLAGS::CONTENT_TO_GRAVE, newvalue );
}

void UCorpse::add( Item* item )
{
  // When an item is added, check if it's equippable and add to the appropriate layer
  if ( Items::valid_equip_layer( item ) && GetItemOnLayer( item->tile_layer ) == item )
  {
    PutItemOnLayer( item );
  }

  // plus the defaults from UContainer
  base::add( item );
}

void UCorpse::equip_and_add( Item* item, unsigned layer )
{
  can_equip_list_[layer].set( item );
  add_at_random_location( item );
}

void UCorpse::remove( iterator itr )
{
  Item* item = *itr;

  if ( Items::valid_equip_layer( item ) )
  {
    auto& item_on_layer = GetItemOnLayer( item->tile_layer );

    if ( item_on_layer != nullptr && item_on_layer->serial == item->serial )
    {
      RemoveItemFromLayer( item );
    }
  }
  base::remove( itr );
}


u16 UCorpse::get_senditem_amount() const
{
  return corpsetype;
}

void UCorpse::spill_contents( Multi::UMulti* multi )
{
  bool any;
  do
  {
    any = false;
    for ( iterator itr = begin(); itr != end(); ++itr )
    {
      Item* item = *itr;
      if ( item->tile_layer == LAYER_HAIR || item->tile_layer == LAYER_BEARD ||
           item->tile_layer == LAYER_FACE || item->movable() == false )
      {
        Core::destroy_item( item );
        any = true;
        break;  // our iterators are now useless, so start over
      }
    }
  } while ( any );

  if ( !take_contents_to_grave() )
    base::spill_contents( multi );
}

void UCorpse::PutItemOnLayer( Item* item )
{
  passert( Items::valid_equip_layer(
      item ) );  // Calling code must make sure that item->tile_layer is valid!

  item->set_dirty();
  set_dirty();
  item->layer = item->tile_layer;
}

void UCorpse::RemoveItemFromLayer( Item* item )
{
  passert( Items::valid_equip_layer(
      item ) );  // Calling code must make sure that item->tile_layer is valid!

  item->set_dirty();
  set_dirty();
  item->layer = 0;
}

void UCorpse::printProperties( Clib::StreamWriter& sw ) const
{
  base::printProperties( sw );
  sw.add( "CorpseType", corpsetype );
  sw.add( "OwnerSerial", ownerserial );
  sw.add( "TakeContentsToGrave", take_contents_to_grave() );
}

void UCorpse::readProperties( Clib::ConfigElem& elem )
{
  // corpses can be the same color as mobiles
  u16 savecolor = elem.remove_ushort( "COLOR", 0 );

  base::readProperties( elem );

  color = savecolor;

  elem.remove_prop( "CorpseType", &corpsetype );
  elem.remove_prop( "OwnerSerial", &ownerserial );
  take_contents_to_grave( elem.remove_bool( "TakeContentsToGrave", false ) );
  movable( false );
}

size_t UCorpse::estimatedSize() const
{
  size_t size = base::estimatedSize() + sizeof( u16 ) /*corpsetype*/
                + sizeof( u32 )                       /*ownerserial*/
                // no estimateSize here element is in objhash
                + 3 * sizeof( Items::Item** ) + can_equip_list_.capacity() * sizeof( Items::Item* );
  return size;
}

void UCorpse::on_insert_add_item( Mobile::Character* mob, MoveType move, Items::Item* new_item )
{
  // If we are a corpse and the item has a valid_equip_layer, we equipped it and need to send an
  // update
  if ( Items::valid_equip_layer( new_item ) )
  {
    UCorpse* corpse = static_cast<UCorpse*>( this );
    auto& item_on_layer = corpse->GetItemOnLayer( new_item->tile_layer );
    if ( item_on_layer != nullptr && !item_on_layer->orphan() &&
         item_on_layer->serial == new_item->serial )
    {
      send_corpse_equip_inrange( corpse );
    }
  }

  base::on_insert_add_item( mob, move, new_item );
}

bool UCorpse::get_method_hook( const char* methodname, Bscript::Executor* ex, ExportScript** hook,
                               unsigned int* PC ) const
{
  if ( gamestate.system_hooks.get_method_hook( gamestate.system_hooks.corpse_method_script.get(),
                                               methodname, ex, hook, PC ) )
    return true;
  return base::get_method_hook( methodname, ex, hook, PC );
}
}  // namespace Core
}  // namespace Pol
