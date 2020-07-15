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
#include "../resource.h"
#include "../uobjcnt.h"
#include "../uobject.h"
#include "item.h"
#include "itemdesc.h"

namespace Pol
{
namespace Items
{
Item::Item( const ItemDesc& id, Core::UOBJ_CLASS uobj_class )
    : UObject( id.objtype, uobj_class ),
      container( nullptr ),
      decayat_gameclock_( 0 ),
      amount_( 1 ),
      slot_index_( 0 ),
      _itemdesc( nullptr ),
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
  movable( id.default_movable() );
  inuse( false );
  invisible( id.invisible );

  // quality is a dynprop in Items and defaults to the itemdesc value.
  // However, if this item was created from an itemdescriptor, we need to set its quality.
  // Unfortunately we need to use setQuality() because Equipment is somehow too special to use the dynprop "quality" directly...
  // TODO: remove _quality member from equipment?
  setQuality( id.quality ); 

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
         + sizeof( boost_utils::script_name_flystring );     /*unequip_script_*/
}
}  // namespace Items
}  // namespace Pol
