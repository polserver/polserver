/** @file
 *
 * @par History
 * - 2009/08/06 MuadDib:   Added gotten_by code for items.
 * - 2009/08/25 Shinigami: STLport-5.2.1 fix: init order changed of gotten_by, layer and slot_index_
 * - 2009/09/01 MuadDib:   Removed layer setting on item creation for mounts. Pointless.
 */


#include <stddef.h>

#include "../baseobject.h"
#include "../gameclck.h"
#include "../globals/state.h"
#include "../uobjcnt.h"
#include "../uobject.h"
#include "item.h"
#include "itemdesc.h"
#include "regions/resource.h"

namespace Pol
{
namespace Items
{
Item::Item( const ItemDesc& id, Core::UOBJ_CLASS uobj_class )
    : UObject( id.objtype, uobj_class ),
      container( nullptr ),
      _itemdesc( nullptr ),
      decayat_gameclock_( 0 ),
      amount_( 1 ),
      slot_index_( 0 ),
      layer( 0 ),
      hp_( id.maxhp )
{
  graphic = id.graphic;
  color = id.color;
  setfacing( id.facing );
  equip_script_ = id.equip_script;
  unequip_script_ = id.unequip_script;
  newbie( id.newbie );
  insured( id.insured );
  cursed( id.cursed );
  movable( id.default_movable() );
  inuse( false );
  invisible( id.invisible );
  no_drop( id.no_drop );
  saveonexit( id.save_on_exit );

  // quality is a dynprop that defaults to the itemdesc value. It won't be stored unless it differs
  // from that default. However, GetItemDescriptor() may be used to assign a new quality before the
  // item is created and we need to consider that here.
  quality( id.quality );

  ++Core::stateManager.uobjcount.uitem_count;

  // hmm, doesn't quite work right with items created on startup..
  decayat_gameclock_ = Core::read_gameclock() + id.decay_time * 60;
}

Item::~Item()
{
  --Core::stateManager.uobjcount.uitem_count;
  return_resources( objtype_, amount_ );
}

size_t Item::estimatedSize() const
{
  return base::estimatedSize() + sizeof( Core::UContainer* ) /* container*/
         + sizeof( int )                                     /* decayat_gameclock_*/
         + sizeof( u16 )                                     /* amount_*/
         + sizeof( u8 )                                      /* slot_index_*/
         + sizeof( const ItemDesc* )                         /* _itemdesc*/
         + sizeof( u8 )                                      /* layer*/
         + sizeof( u8 )                                      /* tile_layer*/
         + sizeof( unsigned short )                          /* hp_*/
         + sizeof( boost_utils::script_name_flystring )      /*on_use_script_*/
         + sizeof( boost_utils::script_name_flystring )      /*equip_script_*/
         + sizeof( boost_utils::script_name_flystring )      /*unequip_script_*/
         + sizeof( boost_utils::script_name_flystring );     /*snoop_script_*/
}
}  // namespace Items
}  // namespace Pol
