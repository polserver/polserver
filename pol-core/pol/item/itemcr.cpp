/*
History
=======
2008/12/17 MuadDub:   Added item.tile_layer - returns layer entry from tiledata/tiles.cfg
2009/09/03 MuadDib:	  Changes for account related source file relocation
                      Changes for multi related source file relocation
2009/12/02 Turley:    added config.max_tile_id - Tomi

Notes
=======

*/

#include "../../clib/stl_inc.h"

#ifdef _MSC_VER
#	pragma warning( disable: 4786 )
#endif

#include "../../clib/cfgfile.h"
#include "../../clib/endian.h"
#include "../../clib/logfile.h"
#include "../../clib/strutil.h"

#include "armor.h"
#include "armrtmpl.h"
#include "../multi/boat.h"
#include "../multi/boatcomp.h"
#include "../mobile/charactr.h"
#include "../network/client.h"
#include "../containr.h"
#include "../door.h"
#include "../dye.h"
#include "../multi/house.h"
#include "item.h"
#include "itemdesc.h"
#include "../lockable.h"
#include "../umap.h"
#include "../objecthash.h"
#include "../objtype.h"
#include "../polcfg.h"
#include "../resource.h"
#include "../scrsched.h"
#include "../scrstore.h"
#include "../spells.h"
#include "../spelbook.h"
#include "../tiles.h"
#include "../ufunc.h"
#include "../uofile.h"
#include "../uoscrobj.h"
#include "weapon.h"
#include "wepntmpl.h"


Item* Item::create( u32 objtype, u32 serial )
{
	const ItemDesc& id = find_itemdesc( objtype );
	if (&id != &empty_itemdesc)
	{
		return create( id, serial );
	}
	else if (objtype <= config.max_tile_id)
	{
		temp_itemdesc.objtype = objtype;
		temp_itemdesc.graphic = objtype;
		return create( temp_itemdesc, serial );
	}
	else
	{
		string message = "Objtype not defined: " + hexint( objtype );
		
		if ( !config.ignore_load_errors )
			throw runtime_error( message );
		else
		{		
			cerr << message << endl;
			return NULL;
		}
	}
}

Item* Item::create( const ItemDesc& id, u32 serial)
{
	u32 objtype = id.objtype;
	unsigned short graphic = id.graphic;
	
	Item* item;
	// FIXME looks like a place for a bunch of function pointers if I ever saw one.
	if (id.type == ItemDesc::DOORDESC)
	{
		item = new UDoor( static_cast<const DoorDesc&>(id) );
	}
	else if (id.type == ItemDesc::BOATDESC)
	{
		// still created with create_multi
		return NULL;
	}
	else if (id.type == ItemDesc::HOUSEDESC)
	{
		// still created with create_multi
		return NULL;
	}
	else if ( (objtype >= spell_scroll_objtype_limits[0][0] &&
			   objtype <= spell_scroll_objtype_limits[0][1]) ||
			  (objtype >= spell_scroll_objtype_limits[1][0] &&
			   objtype <= spell_scroll_objtype_limits[1][1]) ||
			  (objtype >= spell_scroll_objtype_limits[2][0] &&
			   objtype <= spell_scroll_objtype_limits[2][1]) )
	{
		item = new USpellScroll( id );
	}
	else if (objtype == UOBJ_CORPSE) // ITEMDESCTODO make new ItemDesc type
	{
		item = new UCorpse( static_cast<const ContainerDesc&>(id) );
	}
	else if (id.type == ItemDesc::SPELLBOOKDESC) // ITEMDESCTODO make new ItemDesc type
	{
		item = new Spellbook( static_cast<const SpellbookDesc&>(id) );
	}
	else if (id.type == ItemDesc::CONTAINERDESC)
	{
		item = new UContainer( static_cast<const ContainerDesc&>(id) );
	}
	else if (id.type == ItemDesc::WEAPONDESC)
	{
		// we call find_itemdesc here because the item descriptor passed in may not
		// be the "real" one - it may be a temporary descriptor.
		const WeaponDesc* permanent_descriptor = static_cast<const WeaponDesc*>(&find_itemdesc(objtype));
		item = new UWeapon( static_cast<const WeaponDesc&>(id), permanent_descriptor );
	}
	else if (id.type == ItemDesc::ARMORDESC)
	{
		const ArmorDesc* permanent_descriptor = static_cast<const ArmorDesc*>(&find_itemdesc(objtype));
		item = new UArmor( static_cast<const ArmorDesc&>(id), permanent_descriptor );
	}
	else if (id.type == ItemDesc::MAPDESC)  // (graphic >= UOBJ_MAP1 && graphic <= UOBJ_ROLLED_MAP2)
	{
		item = new Map( static_cast<const MapDesc&>(id) );
	}
	else if (objtype == EXTOBJ_PORT_PLANK || objtype == EXTOBJ_STARBOARD_PLANK)// ITEMDESCTODO make new ItemDesc type
	{
		item = new UPlank( id );
	}
	else if (objtype_is_lockable(objtype))
	{
		item = new ULockable( id, CLASS_ITEM );
	}
	else
	{
		item = new Item( id, CLASS_ITEM );
	}

	// 12-17-2008 MuadDib added for reading the tilelayer at all times while retaining item.layer useage.
	item->tile_layer = tilelayer( graphic );

	if (serial)
	{
		item->serial = UseItemSerialNumber(serial);
		item->clear_dirty();
	}
	else if (dont_save_itemtype[objtype])
	{
		item->set_dirty();

		item->serial = GetNewItemSerialNumber();
	}
	else // creating something new
	{
		item->set_dirty();
		remove_resources( objtype, 1 );
		item->serial = GetNewItemSerialNumber();
	}
	
	////HASH
	objecthash.Insert(item);
	////

	item->serial_ext = ctBEu32( item->serial );

	item->restart_decay_timer();

	item->graphic = graphic;
	item->graphic_ext = ctBEu16( item->graphic );

	item->copyprops( id.props );

	#ifdef PERGON
	string value_self;
	if (!item->getprop( "ct", value_self )) // Pergon: Check if Prop still exist - prevents Overwrite on Server-Restart
	  item->setprop("ct", "i" + decint( read_gameclock() )); // Pergon: Init Property CreateTime for a new Item
	#endif

	if (!id.control_script.empty())
	{
		UOExecutorModule* uoemod = start_script( id.control_script, item->make_ref() );
		if (!uoemod)
		{
			Log( "Unable to start control script %s for %s\n",
					   id.control_script.name().c_str(),
					   id.objtype_description().c_str() );
		}
	}

	for( unsigned element = 0; element <= ELEMENTAL_TYPE_MAX; ++element )
	{
		switch(element)
		{
			case ELEMENTAL_FIRE: item->element_resist.fire = id.element_resist.fire;
								 item->element_damage.fire = id.element_damage.fire;break;
			case ELEMENTAL_COLD: item->element_resist.cold = id.element_resist.cold;
								 item->element_damage.cold = id.element_damage.cold;break;
			case ELEMENTAL_ENERGY: item->element_resist.energy = id.element_resist.energy;
								 item->element_damage.energy = id.element_damage.energy; break;
			case ELEMENTAL_POISON: item->element_resist.poison = id.element_resist.poison;
								   item->element_damage.poison = id.element_damage.poison; break;
			case ELEMENTAL_PHYSICAL: item->element_resist.physical = id.element_resist.physical;
									 item->element_damage.physical = id.element_damage.physical; break;
		}
	}

	// if ItemDesc is a dynamic one desc could differ and would be lost
	const ItemDesc& origid = find_itemdesc( item->objtype_ );
    if (id.desc != origid.desc)
		item->setname( id.desc);

	return item;
}


