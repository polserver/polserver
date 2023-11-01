/** @file
 *
 * @par History
 * - 2005/01/23 Shinigami: cleanup_vars - Tokuno MapDimension doesn't fit blocks of 64x64
 * (WGRID_SIZE)
 * - 2008/02/09 Shinigami: removed hard coded MAX_CHARS from cleanup_vars()
 * - 2009/01/18 Nando:     Realms was not being freed when exiting POL
 *                         vitals too. Multidefs too. Oh well...
 *                         Moved objecthash.ClearCharacterAccountReferences() out of the realms'
 * loop
 *                         Added clean_multidefs()
 * - 2009/09/03 MuadDib:   Relocation of account related cpp/h
 *                         Changes for multi related source file relocation
 */

#include "uvars.h"

#include <algorithm>
#include <string.h>

#include "../../bscript/bobject.h"
#include "../../clib/boostutils.h"
#include "../../clib/clib_MD5.h"
#include "../../clib/logfacility.h"
#include "../../clib/stlutil.h"
#include "../../plib/systemstate.h"
#include "../accounts/account.h"
#include "../accounts/accounts.h"
#include "../checkpnt.h"
#include "../console.h"
#include "../guilds.h"
#include "../item/equipmnt.h"
#include "../item/itemdesc.h"
#include "../item/weapon.h"
#include "../listenpt.h"
#include "../loadunld.h"
#include "../mobile/attribute.h"
#include "../multi/boat.h"
#include "../multi/multidef.h"
#include "../npctmpl.h"
#include "../objecthash.h"
#include "../party.h"
#include "../polcfg.h"
#include "../polsem.h"
#include "../scrstore.h"
#include "../spells.h"
#include "../startloc.h"
#include "../storage.h"
#include "../syshook.h"
#include "../syshookscript.h"
#include "../uoskills.h"
#include "../uworld.h"
#include "../vital.h"
#include "multidefs.h"
#include "network.h"
#include "object_storage.h"
#include "realms/realm.h"
#include "regions/guardrgn.h"
#include "regions/miscrgn.h"
#include "regions/musicrgn.h"
#include "regions/resource.h"
#include "script_internals.h"
#include "ucfg.h"

#ifdef _MSC_VER
#pragma warning( \
    disable : 4505 )  // '...': unreferenced local function has been removed (because of region.h)
#endif

namespace Pol
{
namespace Core
{
// See comment in boost_utils::flyweight_initializers
boost_utils::flyweight_initializers fw_inits;

GameState gamestate;


GameState::GameState()
    : cmdlevels(),
      npc_templates(),
      npc_template_elems(),
      // Using force allocate because this is inited before reading global CProp setting
      global_properties( new Core::PropertyList( CPropProfiler::Type::GLOBAL, true ) ),
      accounts(),
      startlocations(),
      wrestling_weapon( nullptr ),
      justicedef( nullptr ),
      nocastdef( nullptr ),
      lightdef( nullptr ),
      weatherdef( nullptr ),
      musicdef( nullptr ),
      menus(),
      storage(),
      parties(),
      guilds(),
      nextguildid( 1 ),
      main_realm( nullptr ),
      Realms(),
      shadowrealms_by_id(),
      baserealm_count( 0 ),
      shadowrealm_count( 0 ),

      update_rpm_task( new PeriodicTask( update_rpm, 60, "RPM" ) ),
      regen_stats_task( new PeriodicTask( regen_stats, 5, "Regen" ) ),
      regen_resources_task( new PeriodicTask( regen_resources, 10, 60 * 10, "RsrcRegen" ) ),
      reload_accounts_task( new PeriodicTask( Accounts::reload_account_data, 30, "LOADACCT" ) ),
      write_account_task( new PeriodicTask( Accounts::write_account_data_task, 60, "WRITEACCT" ) ),
      update_sysload_task( new PeriodicTask( update_sysload, 1, "SYSLOAD" ) ),
      reload_pol_cfg_task( new PeriodicTask( PolConfig::reload_pol_cfg, 30, "LOADPOLCFG" ) ),

      attributes(),
      numAttributes( 0 ),
      attributes_byname(),
      pAttrStrength( nullptr ),
      pAttrIntelligence( nullptr ),
      pAttrDexterity( nullptr ),
      pAttrParry( nullptr ),
      pAttrTactics( nullptr ),

      // Magery is repeated at array entry 3, because as of right now, NO spellbook
      // on OSI uses the 301+ spellrange that we can find. 5/30/06 - MuadDib
      // We use Mysticism at array entry 3 because Mysticism spellids are 678 -> 693 and this slot
      // is free.
      spell_scroll_objtype_limits( { { // TODO: Comment those objtypes :D
                                       { { 0x1F2D, 0x1F6C } },
                                       { { 0x2260, 0x226F } },
                                       { { 0x2270, 0x227C } },
                                       { { 0x2D9E, 0x2DAD } },
                                       { { 0x238D, 0x2392 } },
                                       { { 0x23A1, 0x23A8 } },
                                       { { 0x2D51, 0x2D60 } },
                                       { { 0x574B, 0x5750 } } } } ),
      spells(),
      spellcircles(),
      export_scripts(),
      system_hooks(),
      tipfilenames(),
      armorzones(),
      armor_zone_chance_sum( 0 ),
      vitals(),
      numVitals( 0 ),
      pVitalLife( nullptr ),
      pVitalStamina( nullptr ),
      pVitalMana( nullptr ),
      vitals_byname(),
      desctable(),
      old_objtype_conversions(),
      dynamic_item_descriptors(),
      objtype_byname(),
      // The temp_itemdesc is used when something is created by graphic.
      // another option is to create such ItemDesc objects on demand as needed, and keep them
      // around.
      empty_itemdesc( new Items::ItemDesc( Items::ItemDesc::ITEMDESC ) ),
      temp_itemdesc( new Items::ItemDesc( Items::ItemDesc::ITEMDESC ) ),
      resourcedefs(),

      intrinsic_equipments(),
      boatshapes(),

      animation_translates(),
      console_commands(),
      landtiles(),
      landtiles_loaded( false ),
      listen_points(),
      wwwroot_pkg( nullptr ),
      mime_types(),
      task_queue(),
      Global_Ignore_CProps(),
      target_cursors(),
      textcmds(),
      paramtextcmds(),
      uo_skills(),
      task_thread_pool(),
      max_update_range( Plib::RANGE_VISUAL ),
      max_update_range_multi( 0 ),
      max_update_range_client( Plib::RANGE_VISUAL )

{
  memset( &mount_action_xlate, 0, sizeof( mount_action_xlate ) );
}
GameState::~GameState()
{
  // FIXME: since deconstruction of externs has a more or less random order
  // everything should be cleared before.
  // or make sure that the globals get deconstructed before eg the flyweight string container
}


void GameState::update_range_from_multis()
{
  for ( const auto& m_pair : Multi::multidef_buffer.multidefs_by_multiid )
  {
    if ( m_pair.second == nullptr )
      continue;
    auto* mdef = m_pair.second;
    u16 maxrel = (u16)std::max( { std::abs( mdef->minrxyz.x() ), std::abs( mdef->minrxyz.y() ),
                                  std::abs( mdef->maxrxyz.x() ), std::abs( mdef->maxrxyz.y() ) } ) +
                 1;
    if ( maxrel > max_update_range_multi )
      max_update_range_multi = maxrel;
  }
  max_update_range = max_update_range_multi + max_update_range_client;
}
void GameState::update_range_from_client( u16 range )
{
  if ( range > max_update_range_client )
  {
    max_update_range_client = range;
    max_update_range = max_update_range_multi + max_update_range_client;
  }
}

void display_leftover_objects();

void GameState::deinitialize()
{
  INFO_PRINT << "Initiating POL Cleanup....\n";

  networkManager.deinialize();
  deinit_ipc_vars();

  if ( Plib::systemstate.config.log_script_cycles )
    log_all_script_cycle_counts( false );

  checkpoint( "cleaning up vars" );
  cleanup_vars();
  checkpoint( "cleaning up scripts" );
  cleanup_scripts();

  // scripts remove their listening points when they exit..
  // so there should be no listening points to clean up.
  checkpoint( "cleaning listen points" );
  clear_listen_points();


  // unload_other_objects
  unload_intrinsic_weapons();
  unload_intrinsic_templates();

  // unload_itemdesc_scripts
  for ( auto& elem : desctable )
  {
    elem.second->unload_scripts();
  }

  system_hooks.unload_system_hooks();

  configurationbuffer.deinitialize();

  unload_npc_templates();  // quick and nasty fix until npcdesc usage is rewritten Turley
                           // 2012-08-27: moved before objecthash due to npc-method_script cleanup

  Bscript::UninitObject::ReleaseSharedInstance();
  objStorageManager.deinitialize();
  display_leftover_objects();

  Multi::multidef_buffer.deinitialize();
  Plib::systemstate.deinitialize();

  checkpoint( "unloading data" );
  unload_data();
  guilds.clear();

  Clib::MD5_Cleanup();

  checkpoint( "misc cleanup" );

  global_properties->clear();
  menus.clear();

  textcmds.clear();
  paramtextcmds.clear();
  Global_Ignore_CProps.clear();
  mime_types.clear();
  console_commands.clear();
  animation_translates.clear();

  tipfilenames.clear();

  task_thread_pool.deinit_pool();

  checkpoint( "end of xmain2" );

#ifdef __linux__
  unlink( ( Plib::systemstate.config.pidfile_path + "pol.pid" ).c_str() );
#endif
}


void GameState::cleanup_vars()
{
  // dave added 9/27/03: accounts and player characters have a mutual reference that prevents them
  // getting cleaned up
  //  properly. So clear the references now.
  for ( auto& account : accounts )
  {
    for ( int i = 0; i < Plib::systemstate.config.character_slots; i++ )
      account->clear_character( i );
  }

  for ( auto& realm : Realms )
  {
    for ( const auto& p : realm->gridarea() )
    {
      for ( auto& item : realm->getzone_grid( p ).items )
      {
        item->destroy();
      }
      realm->getzone_grid( p ).items.clear();
    }

    for ( const auto& p : realm->gridarea() )
    {
      for ( auto& chr : realm->getzone_grid( p ).characters )
      {
        chr->acct.clear();  // dave added 9/27/03, see above comment re: mutual references
        chr->destroy();
      }
      realm->getzone_grid( p ).characters.clear();
      for ( auto& chr : realm->getzone_grid( p ).npcs )
      {
        chr->acct.clear();  // dave added 9/27/03, see above comment re: mutual references
        chr->destroy();
      }
      realm->getzone_grid( p ).npcs.clear();
    }

    for ( const auto& p : realm->gridarea() )
    {
      for ( auto& multi : realm->getzone_grid( p ).multis )
      {
        multi->destroy();
      }
      realm->getzone_grid( p ).multis.clear();
    }
  }

  // dave renamed this 9/27/03, so we only have to traverse the objhash once, to clear out account
  // references and delete.
  // and Nando placed it outside the Realms' loop in 2009-01-18.
  objStorageManager.objecthash.ClearCharacterAccountReferences();

  accounts.clear();
  Clib::delete_all( startlocations );

  storage.clear();

  // RegionGroup cleanup _before_ Realm cleanup
  delete justicedef;
  delete lightdef;
  delete nocastdef;
  delete weatherdef;
  delete musicdef;
  clean_resources();

  Clib::delete_all( Realms );
  shadowrealms_by_id.clear();
  main_realm = nullptr;

  // delete_all(vitals);
  clean_vitals();
  Multi::clean_boatshapes();

  Mobile::clean_attributes();
  cmdlevels.clear();
  clean_spells();
  clean_skills();
}

// Note, when the program exits, each executor in these queues
// will be deleted by cleanup_scripts()
// Therefore, any object that owns an executor must be destroyed
// before cleanup_scripts() is called.

void GameState::cleanup_scripts()
{
  scriptScheduler.deinitialize();
}

void GameState::clear_listen_points()
{
  for ( auto& lp_pair : listen_points )
  {
    ListenPoint* lp = lp_pair.second;
    delete lp;
    lp_pair.second = nullptr;
  }
  listen_points.clear();
}

void GameState::unload_intrinsic_weapons()
{
  if ( wrestling_weapon != nullptr )
  {
    wrestling_weapon->destroy();
    wrestling_weapon = nullptr;
  }
}

void GameState::unload_intrinsic_templates()
{
  for ( auto it = intrinsic_equipments.begin(); it != intrinsic_equipments.end(); ++it )
  {
    if ( it->second != nullptr )
    {
      it->second->destroy();
      it->second = nullptr;
    }
  }
  intrinsic_equipments.clear();
}

// quick and nasty fix until npcdesc usage is rewritten
void GameState::unload_npc_templates()
{
  npc_templates.clear();

  npc_template_elems.clear();
}


GameState::Memory GameState::estimateSize() const
{
  Memory usage;
  memset( &usage, 0, sizeof( usage ) );

  usage.misc = sizeof( GameState );
  for ( const auto& ele : cmdlevels )
    usage.misc += ele.estimateSize();

  for ( const auto& ele : npc_templates )
  {
    usage.misc += ele.first.capacity() + sizeof( NpcTemplate* ) + ( sizeof( void* ) * 3 + 1 ) / 2;
    if ( ele.second )
      usage.misc += ele.second->estimateSize();
  }

  for ( const auto& ele : npc_template_elems )
  {
    usage.misc +=
        ele.first.capacity() + ele.second.estimateSize() + ( sizeof( void* ) * 3 + 1 ) / 2;
  }

  usage.misc += sizeof( std::unique_ptr<Core::PropertyList> ) + global_properties->estimatedSize();


  usage.account_count = accounts.size();
  usage.account_size += 3 * sizeof( AccountRef* ) + accounts.capacity() * sizeof( AccountRef );
  for ( const auto& acc : accounts )
  {
    if ( acc.get() != nullptr )
      usage.account_size += acc->estimatedSize();
  }

  for ( const auto& loc : startlocations )
  {
    if ( loc != nullptr )
      usage.misc += loc->estimateSize();
  }

  if ( justicedef != nullptr )
    usage.misc += justicedef->estimateSize();
  if ( nocastdef != nullptr )
    usage.misc += nocastdef->estimateSize();
  if ( lightdef != nullptr )
    usage.misc += lightdef->estimateSize();
  if ( weatherdef != nullptr )
    usage.misc += weatherdef->estimateSize();
  if ( musicdef != nullptr )
    usage.misc += musicdef->estimateSize();

  for ( const auto& menu : menus )
    usage.misc += menu.estimateSize();

  usage.misc += storage.estimateSize();

  for ( const auto& party : parties )
    if ( party.get() != nullptr )
      usage.misc += party->estimateSize();

  for ( const auto& guild : guilds )
    if ( guild.second.get() != nullptr )
      usage.misc +=
          sizeof( unsigned int ) + guild.second->estimateSize() + ( sizeof( void* ) * 3 + 1 ) / 2;

  for ( const auto& realm : Realms )
  {
    if ( realm != nullptr )
      usage.realm_size += realm->sizeEstimate();
  }
  usage.realm_size +=
      ( sizeof( int ) + sizeof( Realms::Realm* ) + ( sizeof( void* ) * 3 + 1 ) / 2 ) *
      gamestate.shadowrealms_by_id.size();

  for ( const auto& attr : attributes )
  {
    if ( attr != nullptr )
      usage.misc += attr->estimateSize();
  }

  for ( const auto& attr : attributes_byname )
  {
    usage.misc +=
        attr.first.capacity() + sizeof( Mobile::Attribute* ) + ( sizeof( void* ) * 3 + 1 ) / 2;
  }

  usage.misc += 3 * sizeof( USpell** ) + spells.capacity() * sizeof( USpell* );
  for ( const auto& spell : spells )
  {
    if ( spell != nullptr )
      usage.misc += spell->estimateSize();
  }
  usage.misc += 3 * sizeof( SpellCircle** ) +
                spellcircles.capacity() * ( sizeof( SpellCircle* ) + sizeof( SpellCircle ) );

  usage.misc += 3 * sizeof( ExportScript** ) + export_scripts.capacity() * sizeof( ExportScript* );
  for ( const auto& script : export_scripts )
  {
    if ( script != nullptr )
      usage.misc += script->estimateSize();
  }

  for ( const auto& name : tipfilenames )
  {
    usage.misc += name.capacity();
  }

  for ( const auto& zone : armorzones )
  {
    usage.misc += zone.name.capacity() + sizeof( double ) + 3 * sizeof( unsigned short* ) +
                  zone.layers.capacity() * sizeof( unsigned short );
  }

  usage.misc += 3 * sizeof( Vital** ) + vitals.capacity() * sizeof( Vital* );
  for ( const auto& vital : vitals )
  {
    if ( vital != nullptr )
      usage.misc += vital->estimateSize();
  }
  for ( const auto& vital : vitals_byname )
    usage.misc += vital.first.capacity() + sizeof( Vital* ) + ( sizeof( void* ) * 3 + 1 ) / 2;


  usage.misc += ( sizeof( u32 ) + sizeof( Items::ItemDesc* ) + ( sizeof( void* ) * 3 + 1 ) / 2 ) *
                desctable.size();
  for ( const auto& elem : desctable )
  {
    if ( elem.second != nullptr )
      usage.misc += elem.second->estimatedSize();
  }
  for ( const auto& elem : dynamic_item_descriptors )
  {
    if ( elem != nullptr )
      usage.misc += elem->estimatedSize();
  }
  usage.misc +=
      ( sizeof( unsigned int ) + sizeof( unsigned int ) + ( sizeof( void* ) * 3 + 1 ) / 2 ) *
      old_objtype_conversions.size();

  for ( const auto& elem : objtype_byname )
    usage.misc += elem.first.capacity() + sizeof( u32 ) + ( sizeof( void* ) * 3 + 1 ) / 2;

  for ( const auto& elem : resourcedefs )
  {
    usage.misc += elem.first.capacity() + sizeof( ResourceDef* ) + ( sizeof( void* ) * 3 + 1 ) / 2;
    if ( elem.second != nullptr )
      usage.misc += elem.second->estimateSize();
  }
  for ( const auto& elem : intrinsic_equipments )
  {
    usage.misc += elem.first.first.capacity() + sizeof( u8 ) + sizeof( Items::Equipment* ) +
                  ( sizeof( void* ) * 3 + 1 ) / 2;
  }
  for ( const auto& elem : boatshapes )
  {
    usage.misc += sizeof( u16 ) + sizeof( Multi::BoatShape* ) + ( sizeof( void* ) * 3 + 1 ) / 2;
    if ( elem.second != nullptr )
      usage.misc += elem.second->estimateSize();
  }

  for ( const auto& elem : animation_translates )
  {
    usage.misc +=
        elem.first.capacity() + elem.second.estimateSize() + ( sizeof( void* ) * 3 + 1 ) / 2;
  }

  for ( const auto& elem : console_commands )
  {
    usage.misc += elem.estimateSize();
  }
  usage.misc += ( sizeof( UOExecutor* ) + sizeof( ListenPoint* ) + sizeof( ListenPoint ) +
                  ( sizeof( void* ) * 3 + 1 ) / 2 ) *
                listen_points.size();
  for ( const auto& elem : mime_types )
  {
    usage.misc += elem.first.capacity() + elem.second.capacity() + ( sizeof( void* ) * 3 + 1 ) / 2;
  }

  usage.misc += task_queue.size() * ( sizeof( ScheduledTask* ) + sizeof( ScheduledTask ) );

  for ( const auto& elem : Global_Ignore_CProps )
    usage.misc += elem.capacity() + 3 * sizeof( void* );

  for ( const auto& elem : textcmds )
  {
    usage.misc += elem.first.capacity() + sizeof( elem.second ) + ( sizeof( void* ) * 3 + 1 ) / 2;
  }
  for ( const auto& elem : paramtextcmds )
  {
    usage.misc += elem.first.capacity() + sizeof( elem.second ) + ( sizeof( void* ) * 3 + 1 ) / 2;
  }

  for ( const auto& elem : uo_skills )
  {
    usage.misc += elem.estimateSize();
  }
  return usage;
}
}  // namespace Core
}  // namespace Pol
