/*
History
=======
2007/06/17 Shinigami: added config.world_data_path
2008/12/17 MuadDib:   Added item.tile_layer - returns layer entry from tiledata/tiles.cfg
2009/09/14 MuadDib:   Added slot support to equip_loaded_item() and add_loaded_item()
2009/09/18 MuadDib:   Spellbook rewrite to deal with only bits, not scrolls inside them.

Notes
=======

*/

#include "../clib/stl_inc.h"

#include "../clib/cfgfile.h"
#include "../clib/cfgelem.h"
#include "../clib/fileutil.h"
#include "../clib/strutil.h"
#include "../clib/wallclock.h"

#include "mobile/charactr.h"
#include "clidata.h"
#include "gflag.h"
#include "fnsearch.h"
#include "item/item.h"
#include "objecthash.h"
#include "polcfg.h"
#include "polclass.h"
#include "poltype.h"
#include "loaddata.h"
#include "spelbook.h"
#include "uobject.h"

#include <climits>

unsigned incremental_save_count = 0;
unsigned current_incremental_save = 0;

// if index is UINT_MAX, has been deleted
typedef hash_map<pol_serial_t, unsigned> SerialIndexMap;
SerialIndexMap incremental_serial_index;

void load_incremental_indexes() // indices is such a stupid word
{
	for(;;)
	{
		unsigned next_incremental_counter = incremental_save_count+1;
		string filename = config.world_data_path + "incr-index-" + decint(next_incremental_counter) + ".txt";
		if (!FileExists( filename ))
			break;

		++incremental_save_count;
		ConfigFile cf( filename, "MODIFIED DELETED" );
		ConfigElem elem;
		while (cf.read( elem ))
		{
			unsigned index;
			if (elem.type_is( "Modified" ))
				index = incremental_save_count;
			else
				index = UINT_MAX;

			string name, value;
			while (elem.remove_first_prop( &name, &value ))
			{
				pol_serial_t serial = strtoul( name.c_str(), NULL, 0 );
				incremental_serial_index[ serial ] = index;
			}
		}
	}
}

unsigned get_save_index( pol_serial_t serial )
{
	SerialIndexMap::iterator itr = incremental_serial_index.find( serial );
	if (itr != incremental_serial_index.end())
		return (*itr).second;
	else
		return 0;
}

void read_incremental_saves()
{
	for( unsigned i = 1; i <= incremental_save_count; ++i )
	{
		string filename = config.world_data_path + "incr-data-" + decint(i) + ".txt";
		current_incremental_save = i;

		slurp( filename.c_str(), "CHARACTER NPC ITEM GLOBALPROPERTIES SYSTEM MULTI STORAGEAREA" );

	}
}

void register_deleted_serials()
{
	// we have to store the deleted serials as 'clean deletes' so that we don't start
	// creating objects with the same serials.
	// after a full save, these will be cleared.
	for( SerialIndexMap::const_iterator citr = incremental_serial_index.begin(); citr != incremental_serial_index.end(); ++citr )
	{
		pol_serial_t serial = (*citr).first;
		unsigned index = (*citr).second;
		if (index == UINT_MAX)
			objecthash.RegisterCleanDeletedSerial( serial );
	}
}

void clear_save_index()
{
	incremental_serial_index.clear();
}

typedef std::multimap<pol_serial_t, UObject*> DeferList;
DeferList deferred_insertions;
void defer_item_insertion( Item* item, pol_serial_t container_serial )
{
	deferred_insertions.insert( make_pair( container_serial, item ) );
}

void insert_deferred_items()
{
	if (deferred_insertions.empty())
		return;

	int num_until_dot = 1000;
	unsigned long nobjects = 0;
	wallclock_t start = wallclock();

	cout << "  deferred inserts:";

	for( DeferList::iterator itr = deferred_insertions.begin(); itr != deferred_insertions.end(); ++itr )
	{
		if (--num_until_dot == 0)
		{
			cout << ".";
			num_until_dot = 1000;
		}

		pol_serial_t container_serial = (*itr).first;
		UObject* obj = (*itr).second;

		if (IsCharacter( container_serial ))
		{
			Character* chr = system_find_mobile( container_serial );
			Item* item = static_cast<Item*>(obj);
			if (chr != NULL)
			{
				equip_loaded_item( chr, item );
			}
			else
			{
				cerr << "Item " << hexint(item->serial)
					<< " is supposed to be on Character "  << hexint(container_serial)
					<< ", but that character cannot be found."
					<< endl;

				// Austin - Aug. 10, 2006
				// Removes the object if ignore_load_errors is enabled and the character can't be found.
				if ( !config.ignore_load_errors )
					throw runtime_error( "Data file integrity error" );
				else
				{
					cerr << "Ignore load errors enabled. Removing object." << endl;
					obj->destroy();
				}

			}
		}
		else
		{
			Item* cont_item = system_find_item( container_serial );
			Item* item = static_cast<Item*>(obj);
			if (cont_item != NULL)
			{
				add_loaded_item( cont_item, item );
			}
			else
			{
				cerr << "Item " << hexint(item->serial)
					<< " is supposed to be in container " << hexint(container_serial)
					<< ", but that container cannot be found."
					<< endl;

				// Austin - Aug. 10, 2006
				// Removes the object if ignore_load_errors is enabled and the character can't be found.
				if ( !config.ignore_load_errors )
					throw runtime_error( "Data file integrity error" );
				else
				{
					cerr << "Ignore load errors enabled. Removing object." << endl;
					obj->destroy();
				}
			}
		}
		++nobjects;
	}
	wallclock_t finish = wallclock();

	cout << " " << nobjects << " elements in " << wallclock_diff_ms(start,finish) << " ms." << endl;

	deferred_insertions.clear();
}

void equip_loaded_item( Character* chr, Item* item )
{
	item->layer = tilelayer( item->graphic ); // adjust for tiledata changes
	item->tile_layer = item->layer; // adjust for tiledata changes

	if (chr->equippable( item ) && 
		item->check_equiptest_scripts( chr, true ) && 
		item->check_equip_script( chr, true ) &&
		!item->orphan())//dave added 1/28/3, item might be destroyed in RTC script
	{
		chr->equip( item );
		item->clear_dirty(); // equipping sets dirty
		return;
	}
	else
	{
		cerr << "Item " << hexint(item->serial)
				<< " is supposed to be equipped on Character "
				<< hexint(chr->serial)
				<< ", but is not 'equippable' on that character."
				<< endl;
		UContainer* bp = chr->backpack();
		if (bp)
		{
			gflag_enforce_container_limits = false;
			bool canadd = bp->can_add( *item );
			u8 slotIndex = item->slot_index();
			bool add_to_slot = bp->can_add_to_slot(slotIndex);
			if ( canadd && add_to_slot && item->slot_index(slotIndex) )
			{
				bp->add_at_random_location(item);
				// leaving dirty
				gflag_enforce_container_limits = true;
				cerr << "I'm so cool, I put it in the character's backpack!" << endl;
				return;
			}
			else
			{
				gflag_enforce_container_limits = true;
				cerr << "Tried to put it in the character's backpack, "
						<< "but it wouldn't fit." << endl;
			}
		}
		else
		{
			cerr << "Tried to put it in the character's backpack, "
					<< "but there isn't one.  That's naughty..." << endl;
		}
		throw runtime_error( "Data file integrity error" );
	}
}

extern u16 spell_scroll_objtype_limits[7][2];

void add_loaded_item( Item* cont_item, Item* item )
{
	if (cont_item->isa(UObject::CLASS_CONTAINER))
	{
		UContainer* cont = static_cast<UContainer*>(cont_item);

		// Convert spellbook to use bitwise system, not scrolls.
		if ( cont->script_isa(POLCLASS_SPELLBOOK) )
		{
			// if can't add, means spell already there.
			if ( !cont->can_add( *item ) )
			{
				item->destroy();
				return;
			}
			// this is an oldschool book, oldschool contents. We need to create the bitwise
			// and handle for the first time before destroying the scrolls.
			Spellbook* book = static_cast<Spellbook*>(cont);

			u16 spellnum = USpellScroll::convert_objtype_to_spellnum(item->objtype_, book->spell_school);
			u8  spellslot = spellnum & 7;
			if(spellslot == 0) spellslot = 8;
			book->bitwise_contents[ (spellnum-1) >> 3 ] |= 1 << (spellslot-1);
			item->destroy();
			return;
		}
		
		gflag_enforce_container_limits = false;
		bool canadd = cont->can_add( *item );
		u8 slotIndex = item->slot_index();
		bool add_to_slot = cont->can_add_to_slot(slotIndex);
		if ( !canadd )
		{
			cerr << "Can't add Item " << hexint(item->serial)
					<< " to container " << hexint(cont->serial) << endl;
			throw runtime_error( "Data file error" );
		}

		if ( !add_to_slot || !item->slot_index(slotIndex) )
		{
			cerr << "Can't add Item " << hexint(item->serial)
				<< " to container " << hexint(cont->serial)
				<< " at slot " << hexint(slotIndex) << endl;
			throw runtime_error( "Data file error" );
		}

		cont->add( item );
		item->clear_dirty(); // adding sets dirty

		gflag_enforce_container_limits = true;

		//if (new_parent_cont)
		//	parent_conts.push( cont );
		//if (item->isa( UObject::CLASS_CONTAINER ))
		//	parent_conts.push( static_cast<UContainer*>(item) );
	}
	else
	{
		cout << "Container type " << hexint( cont_item->objtype_ )
			<< " contains items, but is not a container class" << endl;
		throw runtime_error( "Config file error" );
	}
}


