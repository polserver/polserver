/*
History
=======
2005/01/23 Shinigami: for_all_mobiles, write_items & write_multis - Tokuno MapDimension doesn't fit blocks of 64x64 (WGRID_SIZE)
2007/06/17 Shinigami: added config.world_data_path
2009/09/03 MuadDib:   Relocation of account related cpp/h
                      Relocation of multi related cpp/h

Notes
=======

*/

#include "clib/stl_inc.h"
#ifdef _MSC_VER
#pragma warning( disable: 4786 )
#endif


#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "clib/cfgelem.h"
#include "clib/cfgfile.h"
#include "clib/endian.h"
#include "clib/fileutil.h"
#include "clib/logfile.h"
#include "clib/progver.h"
#include "clib/stlutil.h"
#include "clib/strutil.h"

#include "plib/polver.h"
#include "plib/realm.h"

#include "accounts/account.h"
#include "charactr.h"
#include "fnsearch.h"
#include "gflag.h"
#include "gprops.h"
#include "itemdesc.h"
#include "loaddata.h"
#include "objtype.h"
#include "npc.h"
#include "polcfg.h"
#include "polvar.h"
#include "realms.h"
#include "resource.h"
#include "savedata.h"
#include "servdesc.h"
#include "sockio.h"
#include "startloc.h"
#include "storage.h"
#include "stubdata.h"
#include "uvars.h"
#include "ufunc.h"
#include "uworld.h"
#include "multi/multi.h"

////HASH
#include "objecthash.h"
////

typedef std::vector<Item*> ContItemArr;
ContItemArr contained_items;

typedef std::vector<unsigned long> ContSerArr;
ContSerArr  container_serials;

void uox_place_contained_items( void )
{
	int any;
	unsigned idx;
	
	do {
		any = 0;
		for( idx = 0; idx < contained_items.size(); idx++ )
		{
			Item* item = contained_items[idx];
			if (!item) continue;

			u32 container_serial = container_serials[idx];

			if (IsCharacter( container_serial )) // it's equipped on a character
			{
				Character* chr = find_character( container_serial );
				if (chr)
				{
					if (!chr->equippable( item ))
					{
						throw runtime_error( "refusing to equip non-equippable item!" );
					}
					chr->equip( item );
					contained_items[idx] = NULL;
					container_serials[idx] = 0;
					any = 1;
				}
			}
			else
			{
				Item* cont_item = system_find_item( container_serial );

				if (cont_item)
				{
					if (cont_item->isa(UObject::CLASS_CONTAINER))
					{
						// FIXME : Add Grid Index Default Location Checks here.
						// Remember, if index fails, move to the ground.
						((UContainer* ) cont_item)->add( item );
						contained_items[idx] = NULL;
						container_serials[idx] = 0;
						any = 1;
					}
					else
					{
						printf( "Container type 0x%4.04x (%d) contains items, but is not a container class\n",cont_item->objtype_, cont_item->objtype_ );
					}
				}
			}
		}
		
		ContItemArr::iterator itr1 = contained_items.begin();
		ContSerArr::iterator itr2 = container_serials.begin();
		while (itr1 != contained_items.end())
		{
			if ((*itr1) == NULL)
			{
				(*itr1) = contained_items.back();
				(*itr2) = container_serials.back();
				contained_items.pop_back();
				container_serials.pop_back();
				itr1 = contained_items.begin();
				itr2 = container_serials.begin();
			}
			else
			{
				++itr1;
				++itr2;
			}
		}
	} while (any);
	
	if (contained_items.size())
	{
		cout << "Couldn't find a home for " << contained_items.size() << " items." << endl;
		cout << "Serials: ";
		for( idx = 0; idx < contained_items.size(); idx++ )
		{
			cout << contained_items[idx]->serial << " ";
		}
		cout << endl;

	}
	contained_items.clear();
	container_serials.clear();
}

void uox_read_character( ConfigElem& elem )
{
/*
	unsigned long account;
	if (!elem.remove_prop( "Account", &account ) ||
		account != 0xFFffFFffLu)
	{
		return;
	}

	Character* chr = new Character;
	elem.remove_prop( "NAME", chr->name );
	
	elem.remove_prop( "SERIAL", &chr->serial );
	chr->serial_ext = ctBEu32( chr->serial );
	chr->wornitems.serial = chr->serial;
	UseCharSerialNumber( chr->serial );

	elem.remove_prop( "XBODY", &chr->objtype );
	chr->objtype_ext = ctBEu16( chr->objtype );

	elem.remove_prop( "XSKIN", &chr->color );
	chr->color_ext = ctBEu16( chr->color );
	
	elem.remove_prop( "X", &chr->x );
	elem.remove_prop( "Y", &chr->y );
	unsigned short z;
	elem.remove_prop( "Z", &z );
	chr->z = (u8)z;

	unsigned short facing;
	elem.remove_prop( "DIR", &facing );
	chr->facing = static_cast<UFACING>(facing);

	if (chr->objtype == UOBJ_HUMAN_MALE)
		chr->gender = GENDER_MALE;
	else if (chr->objtype == UOBJ_HUMAN_FEMALE)
		chr->gender = GENDER_FEMALE;

	chr->position_changed();

	characters.push_back( chr );
	SetCharacterWorldPosition( chr );
*/
}

void uox_read_item( ConfigElem& elem )
{
	u32 serial;
	u16 objtype;
	if (elem.remove_prop( "SERIAL", &serial ) == 0 ||
		elem.remove_prop( "ID", &objtype ) == 0)
	{
		return;
	}
	if (objtype >= 0x4000)
	{
		return;
	}
	Item* item = Item::create( objtype, serial );

	elem.remove_prop( "X", &item->x );
	elem.remove_prop( "Y", &item->y );
	if (item->x > WORLD_MAX_X || item->y > WORLD_MAX_Y)
	{
		cerr << "Error importing impworld.wsc:" << endl
			 << "WorldItem " << serial << " position (" << item->x 
				  << "," << item->y << ") is out of bounds." << endl;
		throw runtime_error( "Data integrity error in impworld.wsc" );
	}
	unsigned short z;
	elem.remove_prop( "Z", &z );
	item->z = (u8)z;
	
	elem.remove_prop( "COLOR", &item->color );
	item->color_ext = ctBEu16( item->color );

	u32 container_serial = 0;
	elem.remove_prop( "CONT", &container_serial );
	
	unsigned short layer;
	elem.remove_prop( "LAYER", &layer );
	item->layer = (u8) layer;

	unsigned short amount;
	elem.remove_prop( "AMOUNT", &amount );
	item->setamount( amount );

	if (container_serial == 0xFFffFFffLu)
	{
		add_item_to_world( item );
	}
	else
	{
		contained_items.push_back( item );
		container_serials.push_back( container_serial );
	}
}

void import_wsc( void )
{
	string impworldfile = config.world_data_path + "impworld.wsc";

	if (FileExists( impworldfile ))
	{
		cout << "Found " << impworldfile << " - importing" << endl;
		//if (1)
		{
			ConfigFile cf( impworldfile );
			
			ConfigElem elem;
			while (cf.read( elem ))
			{
				// printf( "UOX3 data element: %s (%s)\n", elem.type(), elem.rest() );
				if (stricmp(elem.type(), "SECTION")==0)
				{
					if (strnicmp( elem.rest(), "CHARACTER", 9 ) == 0)
						uox_read_character( elem );
					else if (strnicmp( elem.rest(), "WORLDITEM", 9 ) == 0)
						uox_read_item( elem );
				}
			}

			uox_place_contained_items();
		}
		unlink( impworldfile.c_str() );
	}
}

/****************** POL Native Files *******************/
//Dave changed 3/8/3 to use objecthash
void read_character( ConfigElem& elem )
{
	// if this object is modified in a subsequent incremental save,
	// don't load it now. 
	pol_serial_t serial = 0;
	elem.get_prop( "SERIAL", &serial );
	if (get_save_index( serial ) > current_incremental_save)
		return;

	CharacterRef chr( new Character( elem.remove_ushort( "OBJTYPE" ) ) );

	try
	{
		// note chr->logged_in is true..
		chr->readProperties( elem );
		chr->clear_dirty();
		
		// readProperties gets the serial, so we can't add to the objecthash until now.
		objecthash.Insert(chr.get());
	}
	catch( exception& )
	{
		if (chr.get() != NULL)
			chr->destroy();
		throw;
	}
}

//Dave changed 3/8/3 to use objecthash
void read_npc( ConfigElem& elem )
{
	// if this object is modified in a subsequent incremental save,
	// don't load it now. 
	pol_serial_t serial = 0;
	elem.get_prop( "SERIAL", &serial );
	if (get_save_index( serial ) > current_incremental_save)
		return;

	NPCRef npc( new NPC( elem.remove_ushort( "OBJTYPE" ), elem ) );

	try
	{
		npc->readProperties( elem );

		SetCharacterWorldPosition( npc.get() );
		npc->clear_dirty();

		////HASH
		objecthash.Insert(npc.get());
		////
	}
	catch( exception& )
	{
		if (npc.get() != NULL)
			npc->destroy();
		throw;
	}
}

// hmm, an interesting idea, what if there was an
// Item::create( ConfigElem& elem ) member,
// which would do this?
// Item would need a constructor (ConfigElem&) also.
// Polymorphism would then take
// care of it, and move the logic into the derived
// classes.
Item* read_item( ConfigElem& elem )
{
	u32 serial;
	u16 objtype;
	if (elem.remove_prop( "SERIAL", &serial ) == false)
	{
		cerr << "Item element has no SERIAL property, omitting." << endl;
		return NULL;
	}

	if (config.check_integrity)
	{
		if (system_find_item( serial ))
		{
			cerr << "Duplicate item read from datafiles (Serial=0x"
				  << hex << serial << dec << ")"
				  << endl;
			throw runtime_error( "Data integrity error" );
		}
	}
	if (elem.remove_prop( "OBJTYPE", &objtype ) == false)
	{
		cerr << "Item (Serial 0x" << hex << serial << dec << ") has no OBJTYPE property, omitting." << endl;
		return NULL;
	}
	if (old_objtype_conversions.count( objtype ))
		objtype = old_objtype_conversions[ objtype ];

	Item* item = Item::create( objtype, serial );
	if (item == NULL)
	{
		cerr << "Unable to create item: objtype=" << hexint(objtype) << ", serial=0x" << hex << serial << dec << endl;
		if ( !config.ignore_load_errors )
			throw runtime_error( "Item::create failed!" );
		else
			return NULL;
	}
	item->realm = find_realm(string("britannia"));
		
	item->readProperties( elem );

	item->clear_dirty();

	return item;
}

#define USE_PARENT_CONTS 1

typedef std::stack<UContainer*> ContStack;
static ContStack parent_conts;

void read_global_item( ConfigElem& elem, int sysfind_flags )
{
	// if this object is modified in a subsequent incremental save,
	// don't load it now. 
	pol_serial_t serial = 0;
	elem.get_prop( "SERIAL", &serial );
	if (get_save_index( serial ) > current_incremental_save)
		return;


	u32 container_serial = 0;
 	elem.remove_prop( "CONTAINER", &container_serial );

	Item* item = read_item( elem );
	//dave added 1/15/3, protect against further crash if item is null. Should throw instead?
	if (item == NULL)
	{
		elem.warn_with_line("Error reading item SERIAL or OBJTYPE.");
		return;
	}

	ItemRef itemref(item); //dave 1/28/3 prevent item from being destroyed before function ends
	if (container_serial == 0)
	{
		add_item_to_world( item );
		if (item->isa( UObject::CLASS_CONTAINER ))
			parent_conts.push( static_cast<UContainer*>(item) );
	}
	else
	{
		if (IsCharacter( container_serial )) // it's equipped on a character
		{
			Character* chr = system_find_mobile( container_serial );
			if (chr != NULL)
			{
				equip_loaded_item( chr, item );
			}
			else
			{
				defer_item_insertion( item, container_serial );
			}
			return;
		}
		Item* cont_item = NULL;
		bool new_parent_cont = false;

		while (!parent_conts.empty())
		{
			UContainer* cont = parent_conts.top();
			if (cont->serial == container_serial)
			{
				cont_item = cont;
				break;
			}
			else
			{
				parent_conts.pop();
			}
		}

		if (cont_item == NULL)
		{
			cont_item = system_find_item( container_serial );
			new_parent_cont = true;
		}

		if (cont_item)
		{
			add_loaded_item( cont_item, item );
		}
		else
		{
			defer_item_insertion( item, container_serial );
		}
	}
}

void read_system_vars(ConfigElem& elem)
{
	polvar.DataWrittenBy = elem.remove_ushort( "CoreVersion" );
	stored_last_item_serial = elem.remove_ulong( "LastItemSerialNumber", ULONG_MAX ); //dave 3/9/3
	stored_last_char_serial = elem.remove_ulong( "LastCharSerialNumber", ULONG_MAX ); //dave 3/9/3
}

void read_shadow_realms(ConfigElem& elem)
{
	std::string name = elem.remove_string("Name");
	Realm* baserealm = find_realm(elem.remove_string("BaseRealm"));
	if ( !baserealm )
		elem.warn_with_line("BaseRealm not found.");
	if ( defined_realm(name) )
		elem.warn_with_line("Realmname already defined");
	add_realm(name, baserealm);
	cout << endl << "Shadowrealm " << name;
}

void read_multi( ConfigElem& elem )
{
	// if this object is modified in a subsequent incremental save,
	// don't load it now. 
	pol_serial_t serial = 0;
	elem.get_prop( "SERIAL", &serial );
	if (get_save_index( serial ) > current_incremental_save)
		return;

	u16 objtype;
	if (elem.remove_prop( "SERIAL", &serial ) == false)
	{
		cerr << "A Multi has no SERIAL property." << endl;
		throw runtime_error( "Config File error." );
	}
	if (system_find_multi( serial ) || system_find_item( serial ))
	{
		cerr << "Duplicate item read from datafiles (Serial=0x"
			  << hex << serial << dec << ")"
			  << endl;
		throw runtime_error( "Data integrity error" );
	}
	if (elem.remove_prop( "OBJTYPE", &objtype ) == false)
	{
		cerr << "Multi (Serial 0x" << hex << serial << dec << ") has no OBJTYPE property, omitting." << endl;
		return;
	}
	if (old_objtype_conversions.count( objtype ))
		objtype = old_objtype_conversions[ objtype ];

	UMulti* multi = UMulti::create( find_itemdesc(objtype), serial );
	if (multi == NULL)
	{
		cerr << "Unable to create multi: objtype=" << hexint(objtype) << ", serial=" << hexint(serial) << endl;
		throw runtime_error( "Multi::create failed!" );
	}
	multi->readProperties( elem );

	add_multi_to_world( multi );
}

string elapsed( clock_t start, clock_t end )
{
	long ms = static_cast<long>((end-start) * 1000.0 / CLOCKS_PER_SEC);
	return decint( ms ) + " ms";
}

void slurp( const char* filename, const char* tags, int sysfind_flags )
{
	static int num_until_dot = 1000;
	unsigned long nobjects = 0;

	if (FileExists( filename ))
	{
		cout << "  " << filename << ":";
		ConfigFile cf( filename, tags );
		ConfigElem elem;

		wallclock_t start = wallclock();

		while (cf.read( elem ))
		{
			if (--num_until_dot == 0)
			{
				cout << ".";
				num_until_dot = 1000;
			}
			try {
				if (stricmp( elem.type(), "CHARACTER" ) == 0)
					read_character( elem );
				else if (stricmp( elem.type(), "NPC" ) == 0)
					read_npc( elem );
				else if (stricmp( elem.type(), "ITEM" ) == 0)
					read_global_item( elem, sysfind_flags );
				else if (stricmp( elem.type(), "GLOBALPROPERTIES" ) == 0)
					global_properties.readProperties( elem );
				else if (elem.type_is( "SYSTEM" ))
					read_system_vars( elem );
				else if (elem.type_is( "MULTI" ))
					read_multi( elem );
				else if (elem.type_is( "STORAGEAREA" ))
				{
					StorageArea* storage_area = storage.create_area( elem );
					// this will be followed by an item
					cf.read( elem );
					storage_area->load_item( elem );
				}
				else if (elem.type_is( "REALM" ))
					read_shadow_realms( elem );

			}
			catch( std::exception& )
			{
				if (!config.ignore_load_errors)
					throw;
			}
			++nobjects;
		}

		wallclock_t finish = wallclock();

		cout << " " << nobjects << " elements in " << wallclock_diff_ms(start,finish) << " ms." << endl;
	}
}

void read_pol_dat()
{
	string polfile = config.world_data_path + "pol.txt";

	slurp( polfile.c_str(), "GLOBALPROPERTIES SYSTEM REALM" );

	if (polvar.DataWrittenBy == 0)
	{
		cerr << "CoreVersion not found in " << polfile << endl << endl;
		cerr << polfile << " must contain a section similar to: " << endl;
		cerr << "System" << endl
			 << "{" << endl
			 << "	CoreVersion 93" << endl
			 << "}" << endl
			 << endl;
		cerr << "Ensure that the CoreVersion matches the version that created your data files!" << endl;
		throw runtime_error( "Data file error" );
	}
}

void read_objects_dat()
{
	slurp( (config.world_data_path + "objects.txt").c_str(), "CHARACTER NPC ITEM GLOBALPROPERTIES" );
}

void read_pcs_dat()
{
	slurp( (config.world_data_path + "pcs.txt").c_str(), "CHARACTER ITEM", SYSFIND_SKIP_WORLD );
}

void read_pcequip_dat()
{
	slurp( (config.world_data_path + "pcequip.txt").c_str(), "ITEM", SYSFIND_SKIP_WORLD );
}

void read_npcs_dat()
{
	slurp( (config.world_data_path + "npcs.txt").c_str(), "NPC ITEM", SYSFIND_SKIP_WORLD );
}

void read_npcequip_dat()
{
	slurp( (config.world_data_path + "npcequip.txt").c_str(), "ITEM", SYSFIND_SKIP_WORLD );
}
	
void read_items_dat()
{
	slurp( (config.world_data_path + "items.txt").c_str(), "ITEM" );
}

void read_multis_dat()
{
	slurp( (config.world_data_path + "multis.txt").c_str(), "MULTI" );
//	string multisfile = config.world_data_path + "multis.txt";
//	if (FileExists( multisfile ))
//	{
//		cout << multisfile << ":";
//		ConfigFile cf( multisfile, "MULTI" );
//		ConfigElem elem;
//		while( cf.read( elem ))
//		{
//			UMulti* multi = read_multi( elem );
//			if (multi == NULL) throw runtime_error( "multi creation returned NULL!" );
//
//			add_multi_to_world( multi );
//		}
//	}
}

void read_storage_dat()
{
	string storagefile = config.world_data_path + "storage.txt";

	if (FileExists( storagefile ))
	{
		cout << "  " << storagefile << ":";
		ConfigFile cf2( storagefile );
		storage.read( cf2 );
	}
}

Item* find_existing_item( u16 objtype, u16 x, u16 y, s8 z, Realm* realm )
{
	unsigned short wx, wy;
	zone_convert( x, y, wx, wy, realm );
	ZoneItems& witem = realm->zone[wx][wy].items;
	for( ZoneItems::iterator itr = witem.begin(), end = witem.end(); itr != end; ++itr )
	{
		Item* item = *itr;

				// FIXME won't find doors which have been perturbed
		if (item->objtype_ == objtype &&
			item->x == x &&
			item->y == y &&
			item->z == z)
		{
			return item;
		}
	}
	return NULL;
}

int import_count;
int dupe_count;

void import( ConfigElem& elem )
{
	u16 objtype;
	objtype = elem.remove_ushort( "OBJTYPE" );
	if (objtype > UOBJ_ITEM__HIGHEST)
	{
		cerr << "Importing file: " << hex << objtype << dec << " is out of range." << endl;
		throw runtime_error( "Error while importing file." );
	}

	Item* item = Item::create( objtype, 0x40000000 ); // dummy serial
	
	if (item == NULL)
	{
		cerr << "Unable to import item: objtype=" << objtype << endl;
		throw runtime_error( "Item::create failed!" );
	}
	
	item->readProperties( elem );
	
	if (find_existing_item( item->objtype_, item->x, item->y, item->z, item->realm ))
	{
		item->destroy();
		++dupe_count;
	}
	else
	{
		item->serial = GetNewItemSerialNumber();
		
		item->serial_ext = ctBEu32( item->serial );

		add_item_to_world(item);
		register_with_supporting_multi( item );
		++import_count;
	}

}

void import_new_data()
{
	string importfile = config.world_data_path + "import.txt";

	if (FileExists( importfile ))
	{
//		if (1)
		{
			ConfigFile cf( importfile, "ITEM" );
			ConfigElem elem;
			while (cf.read( elem ))
			{
				import( elem );
			}
		}
		unlink( importfile.c_str() );
		cout << "Import Results: " << import_count << " imported, " << dupe_count << " duplicates." << endl;
	}
}

void rndat( const string& basename )
{
	string datname = config.world_data_path + basename + ".dat";
	string txtname = config.world_data_path + basename + ".txt";

	if (FileExists( datname.c_str() ))
	{
		rename( datname.c_str(), txtname.c_str() );
	}
}

void rename_dat_files()
{
	rndat( "pol" );
	rndat( "objects" );
	rndat( "pcs" );
	rndat( "pcequip" );
	rndat( "npcs" );
	rndat( "npcequip" );
	rndat( "items" );
	rndat( "multis" );
	rndat( "storage" );
	rndat( "resource" );
	rndat( "guilds" );
	rndat( "parties" );
}

void read_guilds_dat();
void write_guilds( ostream& os );

void read_datastore_dat();
void write_datastore( ostream& os );
void commit_datastore();

void read_party_dat();
void write_party( ostream& os );

void for_all_mobiles( void (*f)(Character* chr) )
{
	vector<Realm*>::iterator itr;
	Realm* realm;
	
	for( itr = Realms->begin(); itr != Realms->end(); ++itr)
	{
		realm = *itr;
		unsigned wgridx = realm->width() / WGRID_SIZE;
		unsigned wgridy = realm->height() / WGRID_SIZE;

	// Tokuno-Fix
	if (wgridx * WGRID_SIZE < realm->width())
	  wgridx++;
	if (wgridy * WGRID_SIZE < realm->height())
	  wgridy++;
	
		for( unsigned wx = 0; wx < wgridx; ++wx )
		{
			for( unsigned wy = 0; wy < wgridy; ++wy )
			{
				ZoneCharacters& wchr = realm->zone[wx][wy].characters;
				for( ZoneCharacters::iterator itr = wchr.begin(), end = wchr.end(); itr != end; ++itr )
				{
					Character* chr = *itr;

					(*f)(chr);
				}
			}
		}
	}
}

int read_data()
{
	string objectsndtfile = config.world_data_path + "objects.ndt";
	string storagendtfile = config.world_data_path + "storage.ndt";

	gflag_in_system_load = true;
	if (FileExists( objectsndtfile ))
	{
		// Display reads "Reading data files..."
		cerr << "Error!" << endl
			 << "'" << objectsndtfile << " exists.  This probably means the system" 
			 << endl
			 << "exited while writing its state.  To avoid loss of data," 
			 << endl
			 << "forcing human intervention."
			 << endl;
		throw runtime_error( "Human intervention required." );
	}
	if (FileExists( storagendtfile ))
	{
		cerr << "Error!" << endl
			 << "'" << storagendtfile << " exists.  This probably means the system" 
			 << endl
			 << "exited while writing its state.  To avoid loss of data," 
			 << endl
			 << "forcing human intervention."
			 << endl;
		throw runtime_error( "Human intervention required." );
	}

	rename_dat_files();

	load_incremental_indexes();

	read_pol_dat();

			// POL clock should be paused at this point.
	start_gameclock();

	read_objects_dat();
	read_pcs_dat();
	read_pcequip_dat();
	read_npcs_dat();
	read_npcequip_dat();
	read_items_dat();
	read_multis_dat();
	read_storage_dat();
	read_resources_dat();
	read_guilds_dat();
	read_datastore_dat();
	read_party_dat();

	read_incremental_saves();
	insert_deferred_items();

	register_deleted_serials();
	clear_save_index();

	import_new_data();
	import_wsc();

	//dave 3/9/3
	if( stored_last_item_serial < GetCurrentItemSerialNumber() )
		SetCurrentItemSerialNumber( stored_last_item_serial );
	if( stored_last_char_serial < GetCurrentCharSerialNumber() )
		SetCurrentCharSerialNumber( stored_last_char_serial );

	while (!parent_conts.empty())
		parent_conts.pop();

	for( ObjectHash::hs::const_iterator citr = objecthash.begin(); citr != objecthash.end(); ++citr )
	{
		UObject* obj = (*citr).second.get();
		if (obj->ismobile())
		{
			Character* chr = static_cast<Character*>(obj);

			if (chr->acct != NULL)
				chr->logged_in = false;
		}
	}

	gflag_in_system_load = false;
	return 0;
}

class SaveContext
{
public:

	SaveContext();

	ofstream ofs_pol;
	ofstream ofs_objects;
	ofstream ofs_pcs;
	ofstream ofs_pcequip;
	ofstream ofs_npcs;
	ofstream ofs_npcequip;
	ofstream ofs_items;
	ofstream ofs_multis;
	ofstream ofs_storage;
	ofstream ofs_resource;
	ofstream ofs_guilds;
	ofstream ofs_datastore;
	ofstream ofs_party;
};

SaveContext::SaveContext() :
	ofs_pol(),
	ofs_objects(),
	ofs_pcs(),
	ofs_pcequip(),
	ofs_npcs(),
	ofs_npcequip(),
	ofs_items(),
	ofs_multis(),
	ofs_storage(),
	ofs_resource(),
	ofs_guilds(),
	ofs_datastore(),
	ofs_party()
{
	ofs_pol.exceptions( std::ios_base::failbit | std::ios_base::badbit );
	ofs_objects.exceptions( std::ios_base::failbit | std::ios_base::badbit );
	ofs_pcs.exceptions( std::ios_base::failbit | std::ios_base::badbit );
	ofs_pcequip.exceptions( std::ios_base::failbit | std::ios_base::badbit );
	ofs_npcs.exceptions( std::ios_base::failbit | std::ios_base::badbit );
	ofs_npcequip.exceptions( std::ios_base::failbit | std::ios_base::badbit );
	ofs_items.exceptions( std::ios_base::failbit | std::ios_base::badbit );
	ofs_multis.exceptions( std::ios_base::failbit | std::ios_base::badbit );
	ofs_storage.exceptions( std::ios_base::failbit | std::ios_base::badbit );
	ofs_resource.exceptions( std::ios_base::failbit | std::ios_base::badbit );
	ofs_guilds.exceptions( std::ios_base::failbit | std::ios_base::badbit );
	ofs_datastore.exceptions( std::ios_base::failbit | std::ios_base::badbit );
	ofs_party.exceptions( std::ios_base::failbit | std::ios_base::badbit );

	ofs_pol.open(		(config.world_data_path + "pol.ndt").c_str(),		ios::out );
	ofs_objects.open(	(config.world_data_path + "objects.ndt").c_str(),	ios::out );
	ofs_pcs.open(		(config.world_data_path + "pcs.ndt").c_str(),		ios::out );
	ofs_pcequip.open(	(config.world_data_path + "pcequip.ndt").c_str(),	ios::out );
	ofs_npcs.open(	    (config.world_data_path + "npcs.ndt").c_str(),	    ios::out );
	ofs_npcequip.open(  (config.world_data_path + "npcequip.ndt").c_str(),  ios::out );
	ofs_items.open(	    (config.world_data_path + "items.ndt").c_str(),	    ios::out );
	ofs_multis.open(	(config.world_data_path + "multis.ndt").c_str(),	ios::out );
	ofs_storage.open(	(config.world_data_path + "storage.ndt").c_str(),	ios::out );
	ofs_resource.open(  (config.world_data_path + "resource.ndt").c_str(),  ios::out );
	ofs_guilds.open(	(config.world_data_path + "guilds.ndt").c_str(),	ios::out );
	ofs_datastore.open( (config.world_data_path + "datastore.ndt").c_str(), ios::out );
	ofs_party.open( (config.world_data_path + "parties.ndt").c_str(), ios::out );

	ofs_pcs << "#" << endl
			<< "#  PCS.TXT: Player-Character Data" << endl
			<< "#" << endl
			<< "#  In addition to PC data, this also contains hair, beards, death shrouds," << endl
			<< "#  and backpacks, but not the contents of each backpack." << endl
			<< "#" << endl
			<< endl;

	ofs_pcequip 
			<< "#" << endl
			<< "#  PCEQUIP.TXT: Player-Character Equipment Data" << endl
			<< "#" << endl
			<< "#  This file can be deleted to wipe all items held/equipped by characters" << endl
			<< "#  Note that hair, beards, empty backpacks, and death shrouds are in PCS.TXT." << endl
			<< "#" << endl
			<< endl;

	ofs_npcs
			<< "#" << endl
			<< "#  NPCS.TXT: Nonplayer-Character Data" << endl
			<< "#" << endl
			<< "#  If you delete this file to perform an NPC wipe," << endl
			<< "#  be sure to also delete NPCEQUIP.TXT" << endl
			<< "#" << endl
			<< endl;

	ofs_npcequip
			<< "#" << endl
			<< "#  NPCEQUIP.TXT: Nonplayer-Character Equipment Data" << endl
			<< "#" << endl
			<< "#  Delete this file along with NPCS.TXT to perform an NPC wipe" << endl
			<< "#" << endl
			<< endl;

	ofs_items
			<< "#" << endl
			<< "#  ITEMS.TXT: Item data" << endl
			<< "#" << endl
			<< "#  This file also contains ship and house components (doors, planks etc)" << endl
			<< "#" << endl
			<< endl;

	ofs_multis
			<< "#" << endl
			<< "#  MULTIS.TXT: Ship and House data" << endl
			<< "#" << endl
			<< "#  Deleting this file will not properly wipe houses and ships," << endl
			<< "#  because doors, planks, and tillermen will be left in the world." << endl
			<< "#" << endl
			<< endl;

	ofs_storage
			<< "#" << endl
			<< "#  STORAGE.TXT: Contains bank boxes, vendor inventories, and other data." << endl
			<< "#" << endl
			<< "#  This file can safely be deleted to wipe bank boxes and vendor inventories." << endl
			<< "#  Note that scripts may use this for other types of storage as well" << endl
			<< "#" << endl
			<< endl;

	ofs_resource
			<< "#" << endl
			<< "#  RESOURCE.TXT: Resource System Data" << endl
			<< "#" << endl
			<< endl;

	ofs_guilds
			<< "#" << endl
			<< "#  GUILDS.TXT: Guild Data" << endl
			<< "#" << endl
			<< endl;

	ofs_datastore
			<< "#" << endl
			<< "#  DATASTORE.TXT: DataStore Data" << endl
			<< "#" << endl
			<< endl;
	ofs_party
			<< "#" << endl
			<< "#  PARTIES.TXT: Party Data" << endl
			<< "#" << endl
			<< endl;
}



void write_global_properties( ostream& ofs )
{
	ofs << "GlobalProperties" << endl
		<< "{" << endl;
	global_properties.printProperties( ofs );
	ofs << "}" << endl
		<< endl;
}

void write_system_data( ostream& ofs )
{
	ofs << "System" << endl
		<< "{" << endl
		<< "\tCoreVersion\t" << progver << endl
		<< "\tCoreVersionString\t" << polverstr << endl
		<< "\tCompileDate\t" << compiledate << endl
		<< "\tCompileTime\t" << compiletime << endl
		<< "\tLastItemSerialNumber\t" << GetCurrentItemSerialNumber() << endl //dave 3/9/3
		<< "\tLastCharSerialNumber\t" << GetCurrentCharSerialNumber() << endl //dave 3/9/3
		<< "}" << endl
		<< endl;
}

void write_shadow_realms( ostream& ofs )
{
	vector<Realm*>::iterator itr;
	for(itr = Realms->begin(); itr != Realms->end(); ++itr)
	{
		if( (*itr)->is_shadowrealm )
		{
			ofs << "Realm" << endl
			<< "{" << endl;
			ofs << "\tName\t" << (*itr)->shadowname << pf_endl;
			ofs << "\tBaseRealm\t" << (*itr)->baserealm->name() << pf_endl;
			ofs << "}" << endl
			<< endl;
		}
	}
}

// Austin (Oct. 17, 2006)
// Added to handle gotten item saving.
inline void WriteGottenItem(Character* chr, SaveContext& sc)
{
	Item* item = chr->gotten_item;
	// For now, it just saves the item in items.txt 
	item->x = chr->x;
	item->y = chr->y;
	item->z = chr->z;
	item->realm = chr->realm;

	item->printOn(sc.ofs_items);

	item->x = item->y = item->z = 0;
}

void write_characters( SaveContext& sc )
{
	for( ObjectHash::hs::const_iterator citr = objecthash.begin(); citr != objecthash.end(); ++citr )
	{
		UObject* obj = (*citr).second.get();
		if (obj->ismobile() && !obj->orphan())
		{
			Character* chr = static_cast<Character*>(obj);
			if (chr->isa( UObject::CLASS_NPC ))

			{
				if (chr->saveonexit())
				{
					chr->printOn( sc.ofs_npcs );
					chr->clear_dirty();
					chr->printWornItems( sc.ofs_npcs, sc.ofs_npcequip );
				}
			}
			else
			{
				chr->printOn( sc.ofs_pcs );
				chr->clear_dirty();
				chr->printWornItems( sc.ofs_pcs, sc.ofs_pcequip );

				// Figure out where to save the 'gotten item' - Austin (Oct. 17, 2006)
				if ( chr->gotten_item && !chr->gotten_item->orphan() )
					WriteGottenItem(chr, sc);
			}
		}
	}
}

void write_items( ostream& ofs_items )
{
	vector<Realm*>::iterator itr;
	Realm* realm;
	for( itr = Realms->begin(); itr != Realms->end(); ++itr)
	{
		realm = *itr;
		unsigned wgridx = realm->width() / WGRID_SIZE;
		unsigned wgridy = realm->height() / WGRID_SIZE;

	// Tokuno-Fix
	if (wgridx * WGRID_SIZE < realm->width())
	  wgridx++;
	if (wgridy * WGRID_SIZE < realm->height())
	  wgridy++;
	
		for( unsigned wx = 0; wx < wgridx; ++wx )
		{
			for( unsigned wy = 0; wy < wgridy; ++wy )
			{
				ZoneItems& witem = realm->zone[wx][wy].items;
				for( ZoneItems::iterator itr = witem.begin(), end = witem.end(); itr != end; ++itr )
				{
					Item* item = *itr;

					if (!dont_save_itemtype[item->objtype_] && item->saveonexit())
					{
						ofs_items << *item;
						item->clear_dirty();
					}
				}
			}
		}
	}
}

void write_multis( ostream& ofs )
{
	vector<Realm*>::iterator itr;
	Realm* realm;
	
	for( itr = Realms->begin(); itr != Realms->end(); ++itr)
	{
		realm = *itr;
		unsigned wgridx = realm->width() / WGRID_SIZE;
		unsigned wgridy = realm->height() / WGRID_SIZE;

	// Tokuno-Fix
	if (wgridx * WGRID_SIZE < realm->width())
	  wgridx++;
	if (wgridy * WGRID_SIZE < realm->height())
	  wgridy++;
	
		for( unsigned wx = 0; wx < wgridx; ++wx )
		{
			for( unsigned wy = 0; wy < wgridy; ++wy )
			{
				ZoneMultis& wmulti = realm->zone[wx][wy].multis;
				for( ZoneMultis::iterator itr = wmulti.begin(), end = wmulti.end(); itr != end; ++itr )
				{
					UMulti* multi = *itr;

					ofs << *multi;
					multi->clear_dirty();
				}
			}
		}
	}
}

bool commit( const string& basename )
{
	string bakfile = config.world_data_path + basename + ".bak";
	string datfile = config.world_data_path + basename + ".txt";
	string ndtfile = config.world_data_path + basename + ".ndt";
	
	bool any = false;

	if (FileExists( bakfile ))
	{
		any = true;
		if (unlink( bakfile.c_str() ))
		{
			int err = errno;
			Log2( "Unable to remove %s: %s (%d)\n", bakfile.c_str(), strerror(err), err );
		}
	}
	if (FileExists( datfile ))
	{
		any = true;
		if (rename( datfile.c_str(), bakfile.c_str() ))
		{
			int err = errno;
			Log2( "Unable to rename %s to %s: %s (%d)\n", datfile.c_str(), bakfile.c_str(), strerror(err), err );
		}
	}
	if (FileExists( ndtfile ))
	{
		any = true;
		if (rename( ndtfile.c_str(), datfile.c_str() ))
		{
			int err = errno;
			Log2( "Unable to rename %s to %s: %s (%d)\n", ndtfile.c_str(), datfile.c_str(), strerror(err), err );
		}
	}

	return any;
}

bool should_write_data()
{
	if (config.inhibit_saves)
		return false;
	if (passert_shutdown_due_to_assertion && passert_nosave)
		return false;

	return true;
}

int write_data( unsigned long& dirty_writes, unsigned long& clean_writes, unsigned long& elapsed_ms )
{
	if (!should_write_data())
	{
		dirty_writes = clean_writes = elapsed_ms = 0;
		return -1;
	}

	UObject::dirty_writes = 0;
	UObject::clean_writes = 0;

	vector<wallclock_t> tx;

	{
		SaveContext sc;

		tx.push_back(wallclock());

		sc.ofs_pol << "#" << endl;
		sc.ofs_pol << "#  Created by Version: " << polverstr << endl;
		sc.ofs_pol << "#  Mobiles:		 " << get_mobile_count() << endl;
		sc.ofs_pol << "#  Top-level Items: " << get_toplevel_item_count() << endl;
		sc.ofs_pol << "#" << endl;
		sc.ofs_pol << endl;


		tx.push_back(wallclock());
		write_system_data( sc.ofs_pol );
		tx.push_back(wallclock());
		write_global_properties( sc.ofs_pol );
		tx.push_back(wallclock());
		write_shadow_realms( sc.ofs_pol );

		tx.push_back(wallclock());
		write_characters( sc );
		tx.push_back(wallclock());

		write_items( sc.ofs_items );
		tx.push_back(wallclock());

		write_multis( sc.ofs_multis );
		tx.push_back(wallclock());

		sc.ofs_storage << storage;
		tx.push_back(wallclock());

		write_resources_dat( sc.ofs_resource );
		tx.push_back(wallclock());

		write_guilds( sc.ofs_guilds );
		tx.push_back(wallclock());

		write_datastore( sc.ofs_datastore );
		tx.push_back(wallclock());

		write_party( sc.ofs_party );
		tx.push_back(wallclock());

	}

	// Atomically (hopefully) perform the switch.
	commit_datastore();

	tx.push_back(wallclock());

	commit( "pol" );
	commit( "objects" );
	commit( "pcs" );
	commit( "pcequip" );
	commit( "npcs" );
	commit( "npcequip" );
	commit( "items" );
	commit( "multis" );
	commit( "storage" );
	commit( "resource" );
	commit( "guilds" );
	commit( "datastore" );
	commit( "parties" );

	commit_incremental_saves();
	incremental_save_count = 0;

	tx.push_back(wallclock());

	objecthash.ClearDeleted();

	//for( unsigned i = 1; i < tx.size(); ++i )
	//	cout << "t" << i << "=" << elapsed(tx[i-1],tx[i]);
	//cout << endl;
	//cout << "Clean: " << UObject::clean_writes << " Dirty: " << UObject::dirty_writes << endl;
	clean_writes = UObject::clean_writes;
	dirty_writes = UObject::dirty_writes;
	elapsed_ms = wallclock_diff_ms( tx.front(),tx.back() );

	incremental_saves_disabled = false;
	return 0;
}


void read_starting_locations()
{
	ConfigFile cf( "config/startloc.cfg" );

	ConfigElem elem;
	while (cf.read( elem ))
	{
		if (stricmp( elem.type(), "StartingLocation" ) != 0)
		{
			cerr << "Unknown element type in startloc.cfg: " << elem.type() << endl;
			throw runtime_error( "Error in configuration file." );
		}

		auto_ptr<StartingLocation> loc( new StartingLocation );
		loc->city = elem.remove_string( "CITY" );
		loc->desc = elem.remove_string( "DESCRIPTION" );

		string coord;
		while( elem.remove_prop( "Coordinate", &coord ))
		{
			int x, y, z;
			if (sscanf( coord.c_str(), "%d,%d,%d", &x, &y, &z ) == 3)
			{
				loc->coords.push_back( Coordinate(x,y,z) );
			}
			else
			{
				cerr << "Poorly formed coordinate in startloc.cfg: '"
					 << coord
					 << "' for city "
					 << loc->city
					 << ", description "
					 << loc->desc
					 << endl;
				throw runtime_error( "Configuration file error in startloc.cfg." );
			}
		}
		if (loc->coords.size() == 0)
		{
			cerr << "STARTLOC.CFG: StartingLocation ("
				 << loc->city 
				 << ","
				 << loc->desc
				 << ") has no Coordinate properties."
				 << endl;
			throw runtime_error( "Configuration file error." );
		}
		startlocations.push_back( loc.release() );
	}

	if (startlocations.size() == 0)
		throw runtime_error( "STARTLOC.CFG: No starting locations found.  Clients will crash on character creation." );
}

ServerDescription::ServerDescription() :
	name(""),
	port(0),
	hostname("")
{
	memset( ip, 0, sizeof ip );
}

void read_gameservers()
{
	string accttext;

	ConfigFile cf( "config/servers.cfg" );

	ConfigElem elem;
	while (cf.read( elem ))
	{
		if (!elem.type_is( "GameServer" ))
			continue;

		auto_ptr<ServerDescription> svr( new ServerDescription );
		
		svr->name = elem.remove_string( "NAME" );
		
		string iptext;
		int ip0, ip1, ip2, ip3;
		iptext = elem.remove_string( "IP" );
		if (iptext == "--ip--")
		{
			iptext = ipaddr_str;
			if (iptext == "")
			{
				cout << "Skipping server " << svr->name << " because there is no Internet IP address." << endl;
				continue;
			}
		}
		else if (iptext == "--lan--")
		{
			iptext = lanaddr_str;
			if (iptext == "")
			{
				cout << "Skipping server " << svr->name << " because there is no LAN IP address." << endl;
				continue;
			}
		}
		
		if (isdigit(iptext[0]))
		{
			if (sscanf( iptext.c_str(), "%d.%d.%d.%d", &ip0, &ip1, &ip2, &ip3 ) != 4)
			{
				cerr << "SERVERS.CFG: Poorly formed IP ("
						<< iptext 
						<< ") for GameServer '"
						<< svr->name 
						<< "'." 
						<< endl;
				throw runtime_error( "Configuration file error." );
			}
			svr->ip[0] = ip3;
			svr->ip[1] = ip2;
			svr->ip[2] = ip1;
			svr->ip[3] = ip0;
		}
		else
		{
			svr->hostname = iptext;

#ifdef __linux__
			/* try to look up */
			struct hostent host_ret;
			struct hostent* host_result = NULL;
			char tmp_buf[ 1024 ];
			int my_h_errno;
			int res = gethostbyname_r( svr->hostname.c_str(), &host_ret, tmp_buf, sizeof tmp_buf, &host_result, &my_h_errno );
			if (res == 0 && host_result && host_result->h_addr_list[0])
			{
				char* addr = host_result->h_addr_list[0];
				svr->ip[0] = addr[3];
				svr->ip[1] = addr[2];
				svr->ip[2] = addr[1];
				svr->ip[3] = addr[0];
/*
				struct sockaddr_in saddr;
				memcpy( &saddr.sin_addr, he->h_addr_list[0], he->h_length);
				server->ip[0] = saddr.sin_addr.S_un.S_un_b.s_b1;
				server->ip[1] = saddr.sin_addr.S_un.S_un_b.s_b2;
				server->ip[2] = saddr.sin_addr.S_un.S_un_b.s_b3;
				server->ip[3] = saddr.sin_addr.S_un.S_un_b.s_b4;
*/
			}
			else
			{
				Log2( "Warning: gethostbyname_r failed for server %s (%s): %d\n",
						svr->name.c_str(), svr->hostname.c_str(), my_h_errno );
			}
#endif

		}
		
		svr->port = elem.remove_ushort( "PORT" );

		while (elem.remove_prop( "IPMATCH", &iptext ))
		{
			string::size_type delim = iptext.find_first_of( "/" );
			if (delim != string::npos)
			{
				string ipaddr_str = iptext.substr( 0, delim );
				string ipmask_str = iptext.substr( delim+1 );
				unsigned long ipaddr = inet_addr( ipaddr_str.c_str() );
				unsigned long ipmask = inet_addr( ipmask_str.c_str() );
				svr->ip_match.push_back( ipaddr );
				svr->ip_match_mask.push_back( ipmask );
			}
			else
			{
				unsigned long ipaddr = inet_addr( iptext.c_str() );
				svr->ip_match.push_back( ipaddr );
				svr->ip_match_mask.push_back( 0xFFffFFffLu );
			}
		}

		while (elem.remove_prop( "ACCTMATCH", &accttext ))
		{
			svr->acct_match.push_back(accttext);
		}

		servers.push_back( svr.release() );
	}
	if (servers.size() == 0)
		throw runtime_error( "There must be at least one GameServer in SERVERS.CFG." );
}
