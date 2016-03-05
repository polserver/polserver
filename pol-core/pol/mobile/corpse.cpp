/** @file
 *
 * @par History
 * - 2009/08/07 MuadDib:   Added layer_list_ and functions like WornItems to corpse class. Used to handle showing equippable items
 *                         on a corpse.
 */


#include "corpse.h"

#include "../../clib/cfgelem.h"
#include "../../clib/clib_endian.h"
#include "../../clib/streamsaver.h"

#include "../layers.h"
#include "../ufunc.h"

namespace Pol
{
namespace Core
{
UCorpse::UCorpse( const Items::ContainerDesc& descriptor ) :
  UContainer( descriptor ),
  corpsetype( 0 ),
  take_contents_to_grave( false ),
  ownerserial(0)
{
  movable_ = false;
  layer_list_.resize( HIGHEST_LAYER + 1, EMPTY_ELEM );
}

void UCorpse::add(Item* item)
{

  // When an item is added, check if it's equippable and add to the appropriate layer
  if (Items::valid_equip_layer(item) && GetItemOnLayer(item->tile_layer) == EMPTY_ELEM)
  {
    PutItemOnLayer(item);
  }

  // plus the defaults from UContainer
  base::add(item);
}

void UCorpse::remove(iterator itr)
{
  Item* item = GET_ITEM_PTR(itr);

  if (Items::valid_equip_layer(item))
  {
    Item* item_on_layer = GetItemOnLayer(item->tile_layer);

    if (item_on_layer != EMPTY_ELEM && item_on_layer->serial == item->serial)
    {
      RemoveItemFromLayer(item);
    }
  }
  base::remove(itr);
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
      Item* item = GET_ITEM_PTR( itr );
      if ( item->tile_layer == LAYER_HAIR || item->tile_layer == LAYER_BEARD || item->tile_layer == LAYER_FACE || item->movable() == false )
      {
        Core::destroy_item( item );
        any = true;
        break; // our iterators are now useless, so start over
      }
    }
  }
  while ( any );

  if ( !take_contents_to_grave )
    base::spill_contents( multi );
}

void UCorpse::PutItemOnLayer( Item* item )
{
  passert(Items::valid_equip_layer(item)); // Calling code must make sure that item->tile_layer is valid!

  item->set_dirty();
  set_dirty();
  item->layer = item->tile_layer;
  layer_list_[item->tile_layer] = Contents::value_type( item );
  add_bulk( item );
}

void UCorpse::RemoveItemFromLayer( Item* item )
{
  passert(Items::valid_equip_layer(item)); // Calling code must make sure that item->tile_layer is valid!

  item->set_dirty();
  set_dirty();
  layer_list_[item->tile_layer] = EMPTY_ELEM;
  item->layer = 0;
}

void UCorpse::printProperties( Clib::StreamWriter& sw ) const
{
  base::printProperties( sw );
  sw() << "\tCorpseType\t" << corpsetype << pf_endl;
  sw() << "\tOwnerSerial\t" << ownerserial << pf_endl;
  sw() << "\tTakeContentsToGrave\t" << take_contents_to_grave << pf_endl;
}

void UCorpse::readProperties( Clib::ConfigElem& elem )
{
  // corpses can be the same color as mobiles
  u16 savecolor = elem.remove_ushort( "COLOR", 0 );

  base::readProperties( elem );

  color = savecolor;

  elem.remove_prop( "CorpseType", &corpsetype );
  elem.remove_prop( "OwnerSerial", &ownerserial );
  take_contents_to_grave = elem.remove_bool( "TakeContentsToGrave", false );
  movable_ = false;
}

size_t UCorpse::estimatedSize( ) const
{
  size_t size = base::estimatedSize( )
                + sizeof(u16)/*corpsetype*/
                +sizeof(bool)/*take_contents_to_grave*/
                +sizeof(u32)/*ownerserial*/
                // no estimateSize here element is in objhash
                +3 * sizeof( Items::Item**) + layer_list_.capacity( ) * sizeof( Items::Item*);
  return size;
}

void UCorpse::on_insert_add_item(Mobile::Character* mob, MoveType move, Items::Item* new_item)
{
  // If we are a corpse and the item has a valid_equip_layer, we equipped it and need to send an update
  if (Items::valid_equip_layer(new_item))
  {
    UCorpse* corpse = static_cast<UCorpse*>(this);
    Item* item_on_layer = corpse->GetItemOnLayer(new_item->tile_layer);
    if (item_on_layer != NULL && item_on_layer->serial == new_item->serial)
    {
      send_corpse_equip_inrange(corpse);
    }
  }

  base::on_insert_add_item(mob, move, new_item);
}

}
}

