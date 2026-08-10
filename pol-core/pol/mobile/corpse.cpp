/** @file
 *
 * @par History
 * - 2009/08/07 MuadDib:   Added layer_list_ and functions like WornItems to corpse class. Used to
 * handle showing equippable items
 *                         on a corpse.
 */


#include "pol/mobile/corpse.h"

#include <stddef.h>

#include "bscript/executor.h"
#include "clib/cfgelem.h"
#include "clib/passert.h"
#include "clib/streamsaver.h"
#include "pol/containr.h"
#include "pol/globals/uvars.h"
#include "pol/layers.h"
#include "pol/syshookscript.h"
#include "pol/ufunc.h"
#include "pol/uobject.h"


namespace Pol::Core
{
UCorpse::UCorpse( const Items::ContainerDesc& descriptor )
    : UContainer( descriptor ), corpsetype( 0 ), ownerserial( 0 )
{
  movable( false );
}

UCorpse::LayerView UCorpse::layer_view() const
{
  LayerView view{};
  for ( const auto& item : contents_ )
  {
    if ( item == nullptr )
      continue;
    const Items::Location loc = item->location();
    const auto* on_corpse = loc.get_if<Items::OnCorpse>();
    // The layer is checked rather than assumed: relocate cannot produce anything else, but this
    // indexes an array with it, so it does not want to depend on a guarantee made elsewhere.
    if ( on_corpse != nullptr && on_corpse->corpse == this &&
         Items::valid_equip_layer( on_corpse->layer ) )
      view[on_corpse->layer] = item;
  }
  return view;
}

bool UCorpse::take_contents_to_grave() const
{
  return flags_.get( OBJ_FLAGS::CONTENT_TO_GRAVE );
}

void UCorpse::take_contents_to_grave( bool newvalue )
{
  flags_.change( OBJ_FLAGS::CONTENT_TO_GRAVE, newvalue );
}

void UCorpse::add_rendered_item( Item* item, const Pos2d& pos )
{
  set_dirty();  // what the corpse looks like changed, not just what it holds
  base::add( item, pos );
}

void UCorpse::remove( iterator itr )
{
  // Still OnCorpse at this point: the base call below is what detaches it.
  if ( ( *itr )->location().holds<Items::OnCorpse>() )
    set_dirty();  // one of the items the corpse renders is leaving it

  base::remove( itr );
}


u16 UCorpse::get_senditem_amount() const
{
  return corpsetype;
}

void UCorpse::spill_contents()
{
  bool any;
  do
  {
    any = false;
    for ( auto item : *this )
    {
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
    base::spill_contents();
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
                + sizeof( u32 );                      /*ownerserial*/
  return size;
}

void UCorpse::on_insert_add_item( Mobile::Character* mob, MoveType move, Items::Item* new_item )
{
  // The item is already where it is going by the time this runs, so it can say for itself whether
  // the corpse now renders it -- in which case everyone in range is looking at a stale corpse.
  if ( new_item->location().holds<Items::OnCorpse>() )
    send_corpse_equip_inrange( this );

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
}  // namespace Pol::Core
