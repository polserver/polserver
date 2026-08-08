
#include "pol/mobile/wornitems.h"

#include "bscript/bobject.h"
#include "clib/passert.h"
#include "plib/objtype.h"
#include "pol/containr.h"
#include "pol/extobj.h"
#include "pol/globals/settings.h"
#include "pol/item/item.h"
#include "pol/item/itemdesc.h"
#include "pol/item/location.h"
#include "pol/layers.h"
#include "pol/mobile/charactr.h"


namespace Pol::Core
{
WornItemsContainer::WornItemsContainer()
    : UContainer( Items::find_container_desc( settingsManager.extobj.wornitems_container ) ),
      chr_owner( nullptr )
{
  contents_.resize( HIGHEST_LAYER + 1, nullptr );
}

size_t WornItemsContainer::estimatedSize() const
{
  return sizeof( Mobile::Character* ) /*chr_owner*/ + base::estimatedSize();
}


void WornItemsContainer::adopt( const Mobile::Character& chr )
{
  serial = chr.serial;
  serial_ext = chr.serial_ext;
  // It answers to a serial now, so it is no longer merely under construction. Detached is the
  // truthful resting state: it has a serial and belongs to no registry, and never will.
  set_location( Items::Detached{} );
}

void WornItemsContainer::for_each_item( void ( *f )( Items::Item* item, void* a ), void* arg )
{
  for ( auto& item : contents_ )
  {
    if ( item != nullptr )
    {
      if ( item->isa( UOBJ_CLASS::CLASS_CONTAINER ) )
      {
        UContainer* cont = static_cast<UContainer*>( item );
        cont->for_each_item( f, arg );
      }
      ( *f )( item, arg );
    }
  }
}

void WornItemsContainer::PutItemOnLayer( Items::Item* item )
{
  passert( Items::valid_equip_layer(
      item ) );  // Calling code must make sure that item->tile_layer is valid!

  item->set_dirty();
  item->setposition( Core::Pos4d( item->pos().xyz(), realm() ) );  // TODO POS nullptr
  contents_[item->tile_layer] = Contents::value_type( item );
  item->set_location( Items::Equipped{ chr_owner, item->tile_layer } );
  add_bulk( item );
}

void WornItemsContainer::RemoveItemFromLayer( Items::Item* item )
{
  passert( Items::valid_equip_layer(
      item ) );  // Calling code must make sure that item->tile_layer is valid!

  item->set_dirty();
  contents_[item->tile_layer] = nullptr;
  item->set_location( Items::Detached{} );
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
        item->printOn( sw_pc );
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
        item->printOn( sw_equip );
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

UObject* WornItemsContainer::self_as_owner()
{
  return chr_owner;
}

const UObject* WornItemsContainer::self_as_owner() const
{
  return chr_owner;
}
}  // namespace Pol::Core
