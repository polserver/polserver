/*
History
=======
2009/1/24 MuadDib: Added read_bannedips_config() to reload_configuration.

Notes
=======

*/


#include "clib/stl_inc.h"
#ifdef _MSC_VER
#pragma warning( disable: 4786 )
#endif


#include "clib/cfgelem.h"
#include "clib/cfgfile.h"
#include "clib/fileutil.h"
#include "clib/stlutil.h"
#include "clib/strutil.h"

#include "armor.h"
#include "checkpnt.h"
#include "cmbtcfg.h"
#include "objtype.h"
#include "polcfg.h"
#include "stackcfg.h" //dave 1/26/3
#include "ufunc.h"
#include "uvars.h"
#include "weapon.h"
#include "wepntmpl.h"

#include "loadunld.h"
#include "objecthash.h"

#include "plib/pkg.h"

void initialize_client_interfaces();

void load_repsys_cfg( bool reload );
void unload_repsys_cfg();

void load_npc_templates();
void load_resource_cfg();
void load_gpdata();
void load_itemdesc();
void set_watch_vars();
void load_skill_scripts();
void load_anim_xlate_cfg( bool reload );
void load_spell_data();
void load_armor_zones();
void load_cmdlevels();
void load_package_cmdlevels();
void load_console_commands();
void unload_armor_zones();
void load_tips();
void read_multidefs();
void read_boat_cfg();
void load_intrinsic_weapons();
void unload_intrinsic_weapons();
void read_justice_zones();
void read_nocast_zones();
void read_music_zones();
void read_light_zones();
void read_weather_zones();
void load_packages();
void load_special_storedconfig( string cfgname );
void load_attributes_cfg();
void load_vitals_cfg();
void load_uoskills_cfg();
void load_uoclient_cfg();
void preload_test_scripts();
void read_npc_templates();
void load_fileaccess_cfg();
void load_tiles_cfg();
void load_landtile_cfg();
void load_party_cfg( bool reload );

void load_movecost( bool reload );

void unload_npc_templates();

void check_config()
{
	// Check if secure trading is enabled and that the container for it is setup.
	if ( config.enable_secure_trading )
	{
		const ItemDesc& stid = find_itemdesc(EXTOBJ_SECURE_TRADE_CONTAINER);
		if ( stid.type != ItemDesc::CONTAINERDESC )
			throw runtime_error("Secure trade container ("+hexint(EXTOBJ_SECURE_TRADE_CONTAINER)+") must be defined in itemdesc.cfg as a container.");
	}

	// Make sure backpack container is defined.
	const ItemDesc& backpackid = find_itemdesc(UOBJ_BACKPACK);
	if ( backpackid.type != ItemDesc::CONTAINERDESC )
		throw runtime_error("Backpack container ("+hexint(UOBJ_BACKPACK)+") must be defined in itemdesc.cfg as a container.");

	// Make sure corpse container is defined.
	const ItemDesc& corpseid = find_itemdesc(UOBJ_CORPSE);
	if ( corpseid.type != ItemDesc::CONTAINERDESC )
		throw runtime_error("Corpse container ("+hexint(UOBJ_CORPSE)+") must be defined in itemdesc.cfg as a container.");
    
	// Make sure the WornItems container is defined.
	const ItemDesc& wic_id = find_itemdesc( EXTOBJ_WORNITEMS_CONTAINER );
	if ( wic_id.type != ItemDesc::CONTAINERDESC )
		throw runtime_error("WornItems container ("+hexint(EXTOBJ_WORNITEMS_CONTAINER)+") must be defined in itemdesc.cfg as a container.");

	const ContainerDesc& cd = find_container_desc( EXTOBJ_WORNITEMS_CONTAINER );
	getgraphic( cd.objtype );
}

void load_config( bool reload )
{
	checkpoint( "load movement cost" );
	load_movecost( reload );

	checkpoint( "load animation translations" );
	load_anim_xlate_cfg( reload );

	checkpoint( "load repsys config" );
	load_repsys_cfg( reload );

	checkpoint( "load party config" );
	load_party_cfg( reload );
}

void load_data()
{
//	checkpoint( "read_translations" );
//	read_translations();

	checkpoint( "load_cmdlevels" );
	load_cmdlevels();

	checkpoint( "read_combat_config" );
	read_combat_config();
	
	checkpoint( "read_boat_cfg" );
	read_boat_cfg();

	checkpoint( "read_multidefs" );
	read_multidefs();
		
	checkpoint( "set_watch_vars" );
	set_watch_vars();

	checkpoint( "load_packages" );
	load_packages();

	checkpoint( "load_package_cmdlevels" );
	load_package_cmdlevels();

	checkpoint( "load_resource_cfg" );
	load_resource_cfg();
	
	checkpoint( "read_justice_zones" );
	read_justice_zones();
	
	checkpoint( "read_music_zones" );
	read_music_zones();
	
	checkpoint( "read_nocast_zones" );
	read_nocast_zones();
	
	checkpoint( "read_light_zones" );
	read_light_zones();

	checkpoint( "read_weather_zones" );
	read_weather_zones();

	checkpoint( "load_armor_zones" );
	load_armor_zones();
	
	checkpoint( "load_attributes_cfg" );
	load_attributes_cfg();

	checkpoint( "load_vitals_cfg" );
	load_vitals_cfg();

	checkpoint( "load_uoskills_cfg" );
	load_uoskills_cfg();

	checkpoint( "load_uoclient_cfg" );
	load_uoclient_cfg();

	checkpoint( "initialize_client_interfaces" );
	initialize_client_interfaces();

	checkpoint( "load_tiles_cfg" );
	load_tiles_cfg();

	checkpoint( "load_landtile_cfg" );
	load_landtile_cfg();

	checkpoint( "load_itemdesc" );
	load_itemdesc();

	checkpoint( "load_special_storedconfig: itemdesc" );
	load_special_storedconfig( "itemdesc" );

	checkpoint( "load_special_storedconfig: spells" );
    load_special_storedconfig( "spells" );

	checkpoint( "load_weapon_templates" );
	load_weapon_templates();
	
	checkpoint( "load_npc_templates" );
	load_npc_templates();

	checkpoint( "preload_test_scripts" );
	preload_test_scripts();

	checkpoint( "load_spell_data" );
	load_spell_data();

	checkpoint( "load_tips" );
	load_tips();

	checkpoint( "load stacking cfg" ); //dave 1/26/3
	load_stacking_cfg();

	load_config( false );

	read_npc_templates();


//#ifdef _WIN32
	checkpoint( "load console commands" );
	load_console_commands();
//#endif

	load_fileaccess_cfg();

	checkpoint( "check configuration" );
	check_config();

}

void read_pol_config( bool initial_load );
void read_npc_templates(); 
void read_bannedips_config( bool initial_load );
void reload_configuration()
{
	read_pol_config( false );
	read_bannedips_config( false );
	load_npc_templates();
	read_npc_templates(); //dave 1/12/3 npc template data wasn't actually being read, just names.
	load_console_commands();
	load_fileaccess_cfg();
}


void UnloadAllConfigFiles();
void unload_itemdesc();
void unload_system_hooks();
void unload_party();
void unload_datastore();

void unload_other_objects()
{
	unload_intrinsic_weapons();
	unload_weapon_templates();
}

void unload_data()
{
	unload_armor_zones();

	unload_npc_templates();  //quick and nasty fix until npcdesc usage is rewritten

	unload_repsys_cfg(); // Any better place?

	unload_itemdesc();

	unload_party();

	unload_datastore();
}
