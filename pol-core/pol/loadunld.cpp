/** @file
 *
 * @par History
 * - 2009/1/24 MuadDib: Added read_bannedips_config() to reload_configuration.
 * - 2009/12/02 Turley: config.max_tile_id
 * - 2011/11/12 Tomi:   added extobj.secure_trade_container and extobj.wornitems_container
 */


#include "loadunld.h"

#include <string>

#include "clib/strutil.h"
#include "plib/pkg.h"
#include "plib/systemstate.h"
#include "plib/tiles.h"

#include "checkpnt.h"
#include "cmbtcfg.h"
#include "console.h"
#include "extobj.h"
#include "globals/multidefs.h"
#include "globals/settings.h"
#include "globals/uvars.h"
#include "item/equipmnt.h"
#include "item/itemdesc.h"
#include "landtile.h"
#include "mobile/attribute.h"
#include "multi/multidef.h"
#include "objtype.h"
#include "polcfg.h"

namespace Pol
{
namespace Items
{
void preload_test_scripts();
void unload_itemdesc();
void load_itemdesc();
}  // namespace Items
namespace Mobile
{
void unload_armor_zones();
void load_armor_zones();
}  // namespace Mobile
namespace Multi
{
void load_special_storedconfig( const std::string& cfgname );
void read_boat_cfg();
}  // namespace Multi
namespace Network
{
void read_bannedips_config( bool initial_load );
void initialize_client_interfaces();
}  // namespace Network
namespace Module
{
void load_fileaccess_cfg();
}
namespace Core
{
void load_stacking_cfg();
void load_repsys_cfg( bool reload );
void unload_repsys_cfg();

void load_npc_templates();
void load_resource_cfg();

void set_watch_vars();
void load_skill_scripts();
void load_anim_xlate_cfg( bool reload );
void load_spell_data();

void load_cmdlevels();
void load_package_cmdlevels();
void load_tips();


void load_intrinsic_weapons();

void read_justice_zones();
void read_nocast_zones();
void read_music_zones();
void read_light_zones();
void read_weather_zones();


void load_vitals_cfg();
void load_uoskills_cfg();
void load_uoclient_cfg();

void read_npc_templates();

void load_party_cfg( bool reload );

void load_movecost( bool reload );

void unload_party();
void read_npc_templates();

void check_config()
{
  // Check if secure trading is enabled and that the container for it is setup.
  if ( Plib::systemstate.config.enable_secure_trading )
  {
    const Items::ItemDesc& stid =
        Items::find_itemdesc( settingsManager.extobj.secure_trade_container );
    if ( stid.type != Items::ItemDesc::CONTAINERDESC )
      throw std::runtime_error( "Secure trade container (" +
                                Clib::hexint( settingsManager.extobj.secure_trade_container ) +
                                ") must be defined in itemdesc.cfg as a container." );
  }

  // Make sure backpack container is defined.
  const Items::ItemDesc& backpackid = Items::find_itemdesc( UOBJ_BACKPACK );
  if ( backpackid.type != Items::ItemDesc::CONTAINERDESC )
    throw std::runtime_error( "Backpack container (" + Clib::hexint( UOBJ_BACKPACK ) +
                              ") must be defined in itemdesc.cfg as a container." );

  // Make sure corpse container is defined.
  const Items::ItemDesc& corpseid = Items::find_itemdesc( UOBJ_CORPSE );
  if ( corpseid.type != Items::ItemDesc::CONTAINERDESC )
    throw std::runtime_error( "Corpse container (" + Clib::hexint( UOBJ_CORPSE ) +
                              ") must be defined in itemdesc.cfg as a container." );

  // Make sure the WornItems container is defined.
  const Items::ItemDesc& wic_id =
      Items::find_itemdesc( settingsManager.extobj.wornitems_container );
  if ( wic_id.type != Items::ItemDesc::CONTAINERDESC )
    throw std::runtime_error( "WornItems container (" +
                              Clib::hexint( settingsManager.extobj.wornitems_container ) +
                              ") must be defined in itemdesc.cfg as a container." );

  const Items::ContainerDesc& cd =
      Items::find_container_desc( settingsManager.extobj.wornitems_container );
  Items::getgraphic( cd.objtype );

  // Make sure the boat mount piece is SaveOnExit 0 and the correct graphic
  // (0x3E96) that the client uses to enter into "mouse piloting mode".
  const Items::ItemDesc& boatmountid = Items::find_itemdesc( settingsManager.extobj.boatmount );
  if ( boatmountid.save_on_exit || boatmountid.graphic != 0x3E96 )
    throw std::runtime_error( "Boat mountpiece " +
                              Clib::hexint( Core::settingsManager.extobj.boatmount ) +
                              " must specify SaveOnExit 0 and Graphic 0x3E96." );
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
  //  checkpoint( "read_translations" );
  //  read_translations();

  checkpoint( "load_cmdlevels" );
  load_cmdlevels();

  checkpoint( "read_combat_config" );
  CombatConfig::read_combat_config();

  checkpoint( "read_boat_cfg" );
  Multi::read_boat_cfg();

  checkpoint( "read_multidefs" );
  Multi::read_multidefs();
  gamestate.update_range_from_multis();

  checkpoint( "set_watch_vars" );
  set_watch_vars();

  checkpoint( "load_packages" );
  Plib::load_packages();

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
  Mobile::load_armor_zones();

  checkpoint( "load_attributes_cfg" );
  Mobile::load_attributes_cfg();

  checkpoint( "load_vitals_cfg" );
  load_vitals_cfg();

  checkpoint( "load_uoskills_cfg" );
  load_uoskills_cfg();
  Mobile::combine_attributes_skillid();

  checkpoint( "load_uoclient_cfg" );
  load_uoclient_cfg();

  checkpoint( "initialize_client_interfaces" );
  Network::initialize_client_interfaces();

  checkpoint( "load_tiles_cfg" );
  Plib::load_tiles_cfg();

  checkpoint( "load_landtile_cfg" );
  load_landtile_cfg();

  checkpoint( "load_itemdesc" );
  Items::load_itemdesc();

  checkpoint( "load_special_storedconfig: itemdesc" );
  Multi::load_special_storedconfig( "itemdesc" );

  checkpoint( "load_special_storedconfig: spells" );
  Multi::load_special_storedconfig( "spells" );

  checkpoint( "load_npc_intrinsic_equip" );
  Items::load_npc_intrinsic_equip();

  checkpoint( "load_npc_templates" );
  load_npc_templates();

  checkpoint( "preload_test_scripts" );
  Items::preload_test_scripts();

  checkpoint( "load_spell_data" );
  load_spell_data();

  checkpoint( "load_tips" );
  load_tips();

  checkpoint( "load stacking cfg" );  // dave 1/26/3
  load_stacking_cfg();

  load_config( false );

  read_npc_templates();


  // #ifdef _WIN32
  checkpoint( "load console commands" );
  ConsoleCommand::load_console_commands();
  // #endif

  Module::load_fileaccess_cfg();

  checkpoint( "check configuration" );
  check_config();
}

void reload_configuration()
{
  PolConfig::read_pol_config( false );
  ServSpecOpt::read_servspecopt();
  Network::read_bannedips_config( false );
  gamestate.unload_npc_templates();
  load_npc_templates();
  read_npc_templates();  // dave 1/12/3 npc template data wasn't actually being read, just names.
  ConsoleCommand::load_console_commands();
  Module::load_fileaccess_cfg();
}

void unload_data()
{
  Mobile::unload_armor_zones();

  unload_repsys_cfg();  // Any better place?

  Items::unload_itemdesc();

  unload_party();
}
}  // namespace Core
}  // namespace Pol
