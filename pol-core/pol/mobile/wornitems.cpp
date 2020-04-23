
#include "wornitems.h"

#include "../../bscript/bobject.h"
#include "../../clib/passert.h"
#include "../containr.h"
#include "../extobj.h"
#include "../globals/settings.h"
#include "../item/item.h"
#include "../item/itemdesc.h"
#include "../layers.h"
#include "../objtype.h"
#include "charactr.h"


namespace Pol
{
namespace Core
{
WornItemsContainer::WornItemsContainer()
    : UContainer( Items::find_container_desc( settingsManager.extobj.wornitems_container ) ),
      chr_owner( nullptr )
{
  contents_.resize( HIGHEST_LAYER + 1, EMPTY_ELEM );
}

size_t WornItemsContainer::estimatedSize() const
{
  return sizeof( Mobile::Character* ) /*chr_owner*/ + base::estimatedSize();
}

void WornItemsContainer::set_chr_owner( Mobile::Character* chr )
{
  chr_owner = chr;
}

void WornItemsContainer::PutItemOnLayer( Items::Item* item )
{
  passert( Items::valid_equip_layer(
      item ) );  // Calling code must make sure that item->tile_layer is valid!

  item->set_dirty();
  item->container = this;
  if ( item->realm() != nullptr )
    item->setposition( Core::Pos4d( item->pos().xyz(), nullptr ) );
  item->layer = item->tile_layer;
  contents_[item->tile_layer] = Contents::value_type( item );
  add_bulk( item );
}

void WornItemsContainer::RemoveItemFromLayer( Items::Item* item )
{
  passert( Items::valid_equip_layer(
      item ) );  // Calling code must make sure that item->tile_layer is valid!

  item->set_dirty();
  item->container = nullptr;
  contents_[item->tile_layer] = EMPTY_ELEM;
  // 12-17-2008 MuadDib added to clear item.layer properties.
  item->layer = 0;
  remove_bulk( item );
}

void WornItemsContainer::print( Clib::StreamWriter& sw_pc, Clib::StreamWriter& sw_equip ) const
{
  if ( !saveonexit() )
  {
    return;
  }
  for ( unsigned clayer = 0; clayer < contents_.size(); ++clayer )
  {
    const Items::Item* item = contents_[clayer];
    if ( item )
    {
      if ( !item->itemdesc().save_on_exit || !item->saveonexit() )
        continue;

      if ( ( clayer == LAYER_HAIR ) || ( clayer == LAYER_BEARD ) || ( clayer == LAYER_FACE ) ||
           ( clayer == LAYER_ROBE_DRESS && item->objtype_ == UOBJ_DEATH_SHROUD ) )
      {
        sw_pc << *item;
        item->clear_dirty();
      }
      else if ( clayer == LAYER_BACKPACK )
      {
        // write the backpack to the PC file,
        // and the backpack contents to the PCEQUIP file
        const UContainer* cont = static_cast<const UContainer*>( item );
        cont->printSelfOn( sw_pc );
        cont->clear_dirty();
        cont->printContents( sw_equip );
      }
      else
      {
        sw_equip << *item;
        item->clear_dirty();
      }
    }
  }
}

Bscript::BObjectImp* WornItemsContainer::make_ref()
{
  passert_always( chr_owner != nullptr );
  return chr_owner->make_offline_ref();
}

UObject* WornItemsContainer::owner()
{
  return chr_owner;
}

const UObject* WornItemsContainer::owner() const
{
  return chr_owner;
}

}  // namespace Core
}  // namespace Pol
