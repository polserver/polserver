/** @file
 *
 * @par History
 * - 2008/12/17 MuadDub:   Added item.tile_layer - returns layer entry from tiledata/tiles.cfg
 * - 2009/09/03 MuadDib:   Changes for account related source file relocation
 *                         Changes for multi related source file relocation
 * - 2009/12/02 Turley:    added config.max_tile_id - Tomi
 * - 2011/11/12 Tomi:      added extobj.port_plank and extobj.starboard_plank
 */


#include <stddef.h>
#include <string>

#include "../../../lib/format/format.h"
#include "../../clib/clib_endian.h"
#include "../../clib/logfacility.h"
#include "../../clib/passert.h"
#include "../../clib/rawtypes.h"
#include "../../plib/systemstate.h"
#include "../baseobject.h"
#include "../clidata.h"
#include "../containr.h"
#include "../door.h"
#include "../extobj.h"
#include "../globals/object_storage.h"
#include "../globals/settings.h"
#include "../globals/uvars.h"
#include "../lockable.h"
#include "../mobile/corpse.h"
#include "../module/uomod.h"
#include "../multi/boatcomp.h"
#include "../objtype.h"
#include "../polcfg.h"
#include "../resource.h"
#include "../scrdef.h"
#include "../scrsched.h"
#include "../spelbook.h"
#include "../ufunc.h"
#include "../umap.h"
#include "../uobject.h"
#include "armor.h"
#include "armrtmpl.h"
#include "item.h"
#include "itemdesc.h"
#include "weapon.h"
#include "wepntmpl.h"

namespace Pol
{
namespace Items
{
Item* Item::create( u32 objtype, u32 serial )
{
  const ItemDesc& id = find_itemdesc( objtype );
  if ( &id != Core::gamestate.empty_itemdesc.get() )
  {
    return create( id, serial );
  }
  else if ( objtype <= Plib::systemstate.config.max_tile_id )
  {
    Core::gamestate.temp_itemdesc->objtype = objtype;
    Core::gamestate.temp_itemdesc->graphic = static_cast<u16>( objtype );
    return create( *( Core::gamestate.temp_itemdesc.get() ), serial );
  }
  else
  {
    fmt::Writer message;
    message.Format( "Objtype not defined : 0x{:X}" ) << objtype;

    if ( !Plib::systemstate.config.ignore_load_errors )
      throw std::runtime_error( message.str() );
    else
    {
      ERROR_PRINT << message.str() << "\n";
      return NULL;
    }
  }
}

Item* Item::create( const ItemDesc& id, u32 serial )
{
  u32 objtype = id.objtype;
  unsigned short graphic = id.graphic;

  Item* item;
  // FIXME looks like a place for a bunch of function pointers if I ever saw one.
  if ( id.type == ItemDesc::DOORDESC )
  {
    item = new Core::UDoor( static_cast<const DoorDesc&>( id ) );
  }
  else if ( id.type == ItemDesc::BOATDESC )
  {
    // still created with create_multi
    return NULL;
  }
  else if ( id.type == ItemDesc::HOUSEDESC )
  {
    // still created with create_multi
    return NULL;
  }
  else if ( ( objtype >= Core::gamestate.spell_scroll_objtype_limits[0][0] &&
              objtype <= Core::gamestate.spell_scroll_objtype_limits[0][1] ) ||
            ( objtype >= Core::gamestate.spell_scroll_objtype_limits[1][0] &&
              objtype <= Core::gamestate.spell_scroll_objtype_limits[1][1] ) ||
            ( objtype >= Core::gamestate.spell_scroll_objtype_limits[2][0] &&
              objtype <= Core::gamestate.spell_scroll_objtype_limits[2][1] ) )
  {
    item = new Core::USpellScroll( id );
  }
  else if ( objtype == UOBJ_CORPSE )  // ITEMDESCTODO make new ItemDesc type
  {
    item = new Core::UCorpse( static_cast<const ContainerDesc&>( id ) );
  }
  else if ( id.type == ItemDesc::SPELLBOOKDESC )  // ITEMDESCTODO make new ItemDesc type
  {
    item = new Core::Spellbook( static_cast<const SpellbookDesc&>( id ) );
  }
  else if ( id.type == ItemDesc::CONTAINERDESC )
  {
    item = new Core::UContainer( static_cast<const ContainerDesc&>( id ) );
  }
  else if ( id.type == ItemDesc::WEAPONDESC )
  {
    // we call find_itemdesc here because the item descriptor passed in may not
    // be the "real" one - it may be a temporary descriptor.
    const WeaponDesc* permanent_descriptor =
        static_cast<const WeaponDesc*>( &find_itemdesc( objtype ) );
    item = new UWeapon( static_cast<const WeaponDesc&>( id ), permanent_descriptor );
  }
  else if ( id.type == ItemDesc::ARMORDESC )
  {
    const ArmorDesc* permanent_descriptor =
        static_cast<const ArmorDesc*>( &find_itemdesc( objtype ) );
    item = new UArmor( static_cast<const ArmorDesc&>( id ), permanent_descriptor );
  }
  else if ( id.type == ItemDesc::MAPDESC )  // (graphic >= UOBJ_MAP1 && graphic <= UOBJ_ROLLED_MAP2)
  {
    item = new Core::Map( static_cast<const MapDesc&>( id ) );
  }
  else if ( objtype == Core::settingsManager.extobj.port_plank ||
            objtype == Core::settingsManager.extobj
                           .starboard_plank )  // ITEMDESCTODO make new ItemDesc type
  {
    item = new Multi::UPlank( id );
  }
  else if ( objtype_is_lockable( objtype ) )
  {
    item = new Core::ULockable( id, Core::UOBJ_CLASS::CLASS_ITEM );
  }
  else
  {
    item = new Item( id, Core::UOBJ_CLASS::CLASS_ITEM );
  }

  // 12-17-2008 MuadDib added for reading the tilelayer at all times while retaining item.layer
  // useage.
  item->tile_layer = Core::tilelayer( graphic );

  // Have to be set after the item is created, because item graphic changes
  // Because items can have facing 0 as the lightsource we use as default 127 to check
  if ( item->facing == 127 )
    item->facing = item->tile_layer;

  if ( serial )
  {
    item->serial = Core::UseItemSerialNumber( serial );
    item->clear_dirty();
  }
  else if ( !find_itemdesc( objtype ).save_on_exit )  // get the real itemdesc
  {
    item->set_dirty();

    item->serial = Core::GetNewItemSerialNumber();
  }
  else  // creating something new
  {
    item->set_dirty();
    remove_resources( objtype, 1 );
    item->serial = Core::GetNewItemSerialNumber();
  }

  ////HASH
  Core::objStorageManager.objecthash.Insert( item );
  ////

  item->serial_ext = ctBEu32( item->serial );

  item->restart_decay_timer();

  item->graphic = graphic;

  // If item already has a serial (eg. an existing item loaded from a save),
  // then do not assign CProps from descriptor
  if ( !serial )
    item->copyprops( id.props );

#ifdef PERGON
  std::string value_self;
  if ( !item->getprop( "ct", value_self ) )  // Pergon: Check if Prop still exist - prevents
                                             // Overwrite on Server-Restart
    item->setprop(
        "ct",
        "i" + Clib::decint(
                  Core::read_gameclock() ) );  // Pergon: Init Property CreateTime for a new Item
#endif

  if ( !id.control_script.empty() )
  {
    passert( item->process() == nullptr );

    Module::UOExecutorModule* uoemod = start_script( id.control_script, item->make_ref() );
    if ( uoemod )
    {
      uoemod->attached_item_ = item;
      item->process( uoemod );
    }
    else
    {
      POLLOG << "Unable to start control script " << id.control_script.name() << " for "
             << id.objtype_description() << "\n";
    }
  }

  for ( unsigned element = 0; element <= Core::ELEMENTAL_TYPE_MAX; ++element )
  {
    switch ( element )
    {
    case Core::ELEMENTAL_FIRE:
      item->fire_resist( item->fire_resist().addToValue( id.element_resist.fire ) );
      item->fire_damage( item->fire_damage().addToValue( id.element_damage.fire ) );
      break;
    case Core::ELEMENTAL_COLD:
      item->cold_resist( item->cold_resist().addToValue( id.element_resist.cold ) );
      item->cold_damage( item->cold_damage().addToValue( id.element_damage.cold ) );
      break;
    case Core::ELEMENTAL_ENERGY:
      item->energy_resist( item->energy_resist().addToValue( id.element_resist.energy ) );
      item->energy_damage( item->energy_damage().addToValue( id.element_damage.energy ) );
      break;
    case Core::ELEMENTAL_POISON:
      item->poison_resist( item->poison_resist().addToValue( id.element_resist.poison ) );
      item->poison_damage( item->poison_damage().addToValue( id.element_damage.poison ) );
      break;
    case Core::ELEMENTAL_PHYSICAL:
      item->physical_resist( item->physical_resist().addToValue( id.element_resist.physical ) );
      item->physical_damage( item->physical_damage().addToValue( id.element_damage.physical ) );
      break;
    }
  }

  // if ItemDesc is a dynamic one desc could differ and would be lost
  const ItemDesc& origid = find_itemdesc( item->objtype_ );
  if ( id.desc != origid.desc )
    item->setname( id.desc );

  return item;
}
}
}
