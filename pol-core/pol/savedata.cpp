/** @file
 *
 * @par History
 * - 2007/06/17 Shinigami: added config.world_data_path
 */


#include "savedata.h"

#include <cerrno>
#include <exception>
#include <filesystem>
#include <fstream>
#include <thread>

#include "../clib/Debugging/ExceptionParser.h"
#include "../clib/Program/ProgramConfig.h"
#include "../clib/clib_endian.h"
#include "../clib/esignal.h"
#include "../clib/fileutil.h"
#include "../clib/iohelp.h"
#include "../clib/logfacility.h"
#include "../clib/rawtypes.h"
#include "../clib/streamsaver.h"
#include "../clib/strutil.h"
#include "../clib/timer.h"
#include "../plib/systemstate.h"
#include "accounts/accounts.h"
#include "globals/object_storage.h"
#include "globals/uvars.h"
#include "item/item.h"
#include "item/itemdesc.h"
#include "mobile/charactr.h"
#include "mobile/npc.h"
#include "multi/house.h"
#include "multi/multi.h"
#include "objecthash.h"
#include "polsem.h"
#include "realms/realm.h"
#include "regions/resource.h"
#include "storage.h"
#include "ufunc.h"
#include "uobject.h"
#include "uworld.h"

namespace fs = std::filesystem;

namespace Pol
{
namespace Module
{
void commit_datastore();
void write_datastore( Clib::StreamWriter& sw );
}  // namespace Module
namespace Core
{
void write_party( Clib::StreamWriter& sw );
void write_guilds( Clib::StreamWriter& sw );

std::shared_future<void> SaveContext::finished;
std::atomic<gameclock_t> SaveContext::last_worldsave_success = 0;

SaveContext::SaveContext()
    : _pol(),
      _objects(),
      _pcs(),
      _pcequip(),
      _npcs(),
      _npcequip(),
      _items(),
      _multis(),
      _storage(),
      _resource(),
      _guilds(),
      _datastore(),
      _party(),
      pol( _pol ),
      objects( _objects ),
      pcs( _pcs ),
      pcequip( _pcequip ),
      npcs( _npcs ),
      npcequip( _npcequip ),
      items( _items ),
      multis( _multis ),
      storage( _storage ),
      resource( _resource ),
      guilds( _guilds ),
      datastore( _datastore ),
      party( _party )
{
  pol.open_fstream( Plib::systemstate.config.world_data_path + "pol.ndt", _pol );
  objects.open_fstream( Plib::systemstate.config.world_data_path + "objects.ndt", _objects );
  pcs.open_fstream( Plib::systemstate.config.world_data_path + "pcs.ndt", _pcs );
  pcequip.open_fstream( Plib::systemstate.config.world_data_path + "pcequip.ndt", _pcequip );
  npcs.open_fstream( Plib::systemstate.config.world_data_path + "npcs.ndt", _npcs );
  npcequip.open_fstream( Plib::systemstate.config.world_data_path + "npcequip.ndt", _npcequip );
  items.open_fstream( Plib::systemstate.config.world_data_path + "items.ndt", _items );
  multis.open_fstream( Plib::systemstate.config.world_data_path + "multis.ndt", _multis );
  storage.open_fstream( Plib::systemstate.config.world_data_path + "storage.ndt", _storage );
  resource.open_fstream( Plib::systemstate.config.world_data_path + "resource.ndt", _resource );
  guilds.open_fstream( Plib::systemstate.config.world_data_path + "guilds.ndt", _guilds );
  datastore.open_fstream( Plib::systemstate.config.world_data_path + "datastore.ndt", _datastore );
  party.open_fstream( Plib::systemstate.config.world_data_path + "parties.ndt", _party );

  pcs.comment( "" );
  pcs.comment( " PCS.TXT: Player-Character Data" );
  pcs.comment( "" );
  pcs.comment( " In addition to PC data, this also contains hair, beards, death shrouds," );
  pcs.comment( " and backpacks, but not the contents of each backpack." );
  pcs.comment( "\n" );

  pcequip.comment( "" );
  pcequip.comment( " PCEQUIP.TXT: Player-Character Equipment Data" );
  pcequip.comment( "" );
  pcequip.comment( " This file can be deleted to wipe all items held/equipped by characters" );
  pcequip.comment( " Note that hair, beards, empty backpacks, and death shrouds are in PCS.TXT." );
  pcequip.comment( "\n" );

  npcs.comment( "" );
  npcs.comment( " NPCS.TXT: Nonplayer-Character Data" );
  npcs.comment( "" );
  npcs.comment( " If you delete this file to perform an NPC wipe," );
  npcs.comment( " be sure to also delete NPCEQUIP.TXT" );
  npcs.comment( "\n" );

  npcequip.comment( "" );
  npcequip.comment( " NPCEQUIP.TXT: Nonplayer-Character Equipment Data" );
  npcequip.comment( "" );
  npcequip.comment( " Delete this file along with NPCS.TXT to perform an NPC wipe" );

  npcequip.comment( "\n" );

  items.comment( "" );
  items.comment( " ITEMS.TXT: Item data" );
  items.comment( "" );
  items.comment( " This file also contains ship and house components (doors, planks etc)" );
  items.comment( "\n" );

  multis.comment( "" );
  multis.comment( " MULTIS.TXT: Ship and House data" );
  multis.comment( "" );
  multis.comment( " Deleting this file will not properly wipe houses and ships," );
  multis.comment( " because doors, planks, and tillermen will be left in the world." );
  multis.comment( "\n" );

  storage.comment( "" );
  storage.comment( " STORAGE.TXT: Contains bank boxes, vendor inventories, and other data." );
  storage.comment( "" );
  storage.comment( " This file can safely be deleted to wipe bank boxes and vendor inventories." );
  storage.comment( " Note that scripts may use this for other types of storage as well" );
  storage.comment( "\n" );

  resource.comment( "" );
  resource.comment( " RESOURCE.TXT: Resource System Data" );
  resource.comment( "\n" );

  guilds.comment( "" );
  guilds.comment( " GUILDS.TXT: Guild Data" );
  guilds.comment( "\n" );

  datastore.comment( "" );
  datastore.comment( " DATASTORE.TXT: DataStore Data" );
  datastore.comment( "\n" );

  party.comment( "" );
  party.comment( " PARTIES.TXT: Party Data" );
  party.comment( "\n" );
}

SaveContext::~SaveContext() noexcept( false )
{
  auto stack_unwinding = std::uncaught_exceptions();
  try
  {
    pol.flush_file();
    objects.flush_file();
    pcs.flush_file();
    pcequip.flush_file();
    npcs.flush_file();
    npcequip.flush_file();
    items.flush_file();
    multis.flush_file();
    storage.flush_file();
    resource.flush_file();
    guilds.flush_file();
    datastore.flush_file();
    party.flush_file();
  }
  catch ( ... )
  {
    // during stack unwinding an exception would terminate
    if ( !stack_unwinding )
      throw;
  }
}

/// blocks till possible last commit finishes
void SaveContext::ready()
{
  if ( SaveContext::finished.valid() )
  {
    // Tools::Timer<Tools::DebugT> t("future");
    SaveContext::finished.wait();
  }
}

void write_system_data( Clib::StreamWriter& sw )
{
  sw.begin( "System" );
  sw.add( "CoreVersion", POL_VERSION_STR );
  sw.add( "CoreVersionString", POL_VERSION_STR );
  sw.add( "CompileDateTime", Clib::ProgramConfig::build_datetime() );
  sw.add( "LastItemSerialNumber", GetCurrentItemSerialNumber() );
  sw.add( "LastCharSerialNumber", GetCurrentCharSerialNumber() );
  sw.end();
}

void write_global_properties( Clib::StreamWriter& sw )
{
  sw.begin( "GlobalProperties" );
  gamestate.global_properties->printProperties( sw );
  sw.end();
}

void write_realms( Clib::StreamWriter& sw )
{
  for ( const auto& realm : gamestate.Realms )
  {
    sw.begin( "Realm" );
    if ( !realm->is_shadowrealm )
    {
      sw.add( "Name", realm->name() );
    }
    else
    {
      sw.add( "Name", realm->shadowname );
      sw.add( "BaseRealm", realm->baserealm->name() );
    }
    sw.add( "HasDecay", realm->has_decay );
    sw.end();
  }
}

// Austin (Oct. 17, 2006)
// Added to handle gotten item saving.
void WriteGottenItem( Mobile::Character* chr, Items::Item* item, Clib::StreamWriter& sw )
{
  if ( item == nullptr || item->orphan() )
    return;
  // For now, it just saves the item in items.txt
  item->setposition( chr->pos() );

  item->printOn( sw );

  item->setposition(
      Pos4d( 0, 0, 0,
             item->realm() ) );  // TODO POS position should have no meaning remove this completely
}

void write_characters( Core::SaveContext& sc )
{
  for ( const auto& objitr : objStorageManager.objecthash )
  {
    UObject* obj = objitr.second.get();
    if ( obj->ismobile() && !obj->orphan() )
    {
      Mobile::Character* chr = static_cast<Mobile::Character*>( obj );
      if ( !chr->isa( UOBJ_CLASS::CLASS_NPC ) )
      {
        chr->printOn( sc.pcs );
        chr->clear_dirty();
        chr->printWornItems( sc.pcs, sc.pcequip );
      }
    }
  }
}

void write_npcs( Core::SaveContext& sc )
{
  for ( const auto& objitr : objStorageManager.objecthash )
  {
    UObject* obj = objitr.second.get();
    if ( obj->ismobile() && !obj->orphan() )
    {
      Mobile::Character* chr = static_cast<Mobile::Character*>( obj );
      if ( chr->isa( UOBJ_CLASS::CLASS_NPC ) )
      {
        if ( chr->saveonexit() )
        {
          chr->printOn( sc.npcs );
          chr->clear_dirty();
          chr->printWornItems( sc.npcs, sc.npcequip );
        }
      }
    }
  }
}

void write_items( Clib::StreamWriter& sw_items )
{
  for ( const auto& realm : gamestate.Realms )
  {
    for ( const auto& p : realm->gridarea() )
    {
      for ( const auto& item : realm->getzone_grid( p ).items )
      {
        if ( item->itemdesc().save_on_exit && item->saveonexit() )
        {
          item->printOn( sw_items );
          item->clear_dirty();
        }
      }
    }
  }

  for ( const auto& objitr : objStorageManager.objecthash )
  {
    UObject* obj = objitr.second.get();
    if ( obj->ismobile() && !obj->orphan() )
    {
      Mobile::Character* chr = static_cast<Mobile::Character*>( obj );
      if ( !chr->isa( UOBJ_CLASS::CLASS_NPC ) )
      {
        // Figure out where to save the 'gotten item' - Austin (Oct. 17, 2006)
        if ( chr->has_gotten_item() )
          WriteGottenItem( chr, chr->gotten_item().item(), sw_items );
      }
    }
  }
}

void write_multis( Clib::StreamWriter& ofs )
{
  for ( const auto& realm : gamestate.Realms )
  {
    for ( const auto& p : realm->gridarea() )
    {
      for ( auto& multi : realm->getzone_grid( p ).multis )
      {
        if ( Clib::exit_signalled )  // drop waiting commit on shutdown
        {
          Multi::UHouse* house = multi->as_house();
          if ( house != nullptr )
          {
            if ( house->IsCustom() )
            {
              if ( house->IsWaitingForAccept() )
                house->AcceptHouseCommit( nullptr, false );
            }
          }
        }
        multi->printOn( ofs );
        multi->clear_dirty();
      }
    }
  }
}
bool should_write_data()
{
  if ( Plib::systemstate.config.inhibit_saves )
    return false;
  if ( Clib::passert_shutdown_due_to_assertion && Clib::passert_nosave )
    return false;

  return true;
}

bool commit( const std::string& basename )
{
  auto bakfile = fs::path( Plib::systemstate.config.world_data_path ) / ( basename + ".bak" );
  auto datfile = fs::path( Plib::systemstate.config.world_data_path ) / ( basename + ".txt" );
  auto ndtfile = fs::path( Plib::systemstate.config.world_data_path ) / ( basename + ".ndt" );

  try
  {
    fs::remove( bakfile );  // does not throw if not existing
    if ( fs::exists( datfile ) )
      fs::rename( datfile, bakfile );
    if ( fs::exists( ndtfile ) )
      fs::rename( ndtfile, datfile );
  }
  catch ( const fs::filesystem_error& error )
  {
    POLLOG_ERRORLN( "Unable to commit worldsave: {}\n{}", error.what(),
                    Clib::ExceptionParser::getTrace() );
    return false;
  }
  return true;
}

std::optional<bool> write_data( std::function<void( bool, u32, u32, s64 )> callback,
                                u32* dirty_writes, u32* clean_writes, s64* elapsed_ms )
{
  SaveContext::ready();  // allow only one active
  if ( !should_write_data() )
    return {};

  UObject::dirty_writes = 0;
  UObject::clean_writes = 0;

  Tools::Timer<> timer;
  // launch complete save as seperate thread
  // but wait till the first critical part is finished
  // which means all objects got written into a format object
  // the remaining operations are only pure buffered i/o
  auto critical_promise = std::promise<bool>();
  auto critical_future = critical_promise.get_future();
  auto set_promise = []( auto& promise, bool result )
  {
    try  // guard to be able to try to set it twice (exceptions)
    {
      promise.set_value( result );
    }
    catch ( ... )
    {
    }
  };
  SaveContext::finished = std::async(
      std::launch::async,
      [&, critical_promise = std::move( critical_promise ),
       callback = std::move( callback )]() mutable
      {
        Tools::Timer<> blocking_timer;
        std::atomic<bool> result( true );
        try
        {
          SaveContext sc;
          std::vector<std::future<bool>> critical_parts;
          auto save = [&]( auto func, std::string name )
          {
            critical_parts.push_back( gamestate.task_thread_pool.checked_push(
                [&, name, func = std::move( func )]() mutable
                {
                  Tools::Timer<> swtimer;
                  INFO_PRINTLN( "STARTING {}", name );
                  try
                  {
                    func();
                  }
                  catch ( ... )
                  {
                    POLLOG_ERRORLN( "failed to store {} datafile!\n{}", name,
                                    Clib::ExceptionParser::getTrace() );
                    result = false;
                  }
                  INFO_PRINTLN( "{} -> {}ms thread_id{}", name, swtimer.ellapsed(),
                                std::this_thread::get_id() );
                } ) );
            INFO_PRINTLN( "PUSHED {}", name );
          };

          // ordered roughly by "usual" size, so that the biggest files will be written first
          save( [&]() { write_items( sc.items ); }, "items" );
          save( [&]() { gamestate.storage.print( sc.storage ); }, "storage" );
          save( [&]() { write_characters( sc ); }, "character" );
          save( [&]() { write_npcs( sc ); }, "npcs" );
          save(
              [&]()
              {
                if ( Plib::systemstate.accounts_txt_dirty )
                  Accounts::write_account_data();
              },
              "accounts" );
          save(
              [&]()
              {
                sc.pol.comment( "" );
                sc.pol.comment( " Created by Version: {}", POL_VERSION_ID );
                sc.pol.comment( " Mobiles: {}", get_mobile_count() );
                sc.pol.comment( " Top-level Items: {}", get_toplevel_item_count() );
                sc.pol.comment( "\n" );

                write_system_data( sc.pol );
                write_global_properties( sc.pol );
                write_realms( sc.pol );
              },
              "pol" );
          save( [&]() { write_multis( sc.multis ); }, "multis" );
          save( [&]() { write_resources_dat( sc.resource ); }, "resource" );
          save( [&]() { write_guilds( sc.guilds ); }, "guilds" );
          save(
              [&]()
              {
                Module::write_datastore( sc.datastore );
                // Atomically (hopefully) perform the switch.
                Module::commit_datastore();
              },
              "datastore" );
          save( [&]() { write_party( sc.party ); }, "party" );

          for ( auto& task : critical_parts )
            task.wait();

          set_promise( critical_promise, result );  // critical part end
          blocking_timer.stop();
        }  // deconstructor of the SaveContext flushes and joins the queues
        catch ( std::ios_base::failure& e )
        {
          POLLOG_ERRORLN( "failed to save datafiles! {}:{}\n{}", e.what(), std::strerror( errno ),
                          Clib::ExceptionParser::getTrace() );

          result = false;
          set_promise( critical_promise, result );
        }
        catch ( ... )
        {
          POLLOG_ERRORLN( "failed to save datafiles!\n{}", Clib::ExceptionParser::getTrace() );
          result = false;
          set_promise( critical_promise, result );
        }
        if ( result )
        {
          auto files = { "pol",      "objects",   "pcs",    "pcequip", "npcs",
                         "npcequip", "items",     "multis", "storage", "resource",
                         "guilds",   "datastore", "parties" };
          result =
              std::all_of( files.begin(), files.end(), []( auto file ) { return commit( file ); } );
          if ( result )
            SaveContext::last_worldsave_success = read_gameclock();
        }
        if ( callback )
          callback( result.load(), UObject::clean_writes, UObject::dirty_writes,
                    blocking_timer.ellapsed() );
      } );
  auto res = critical_future.get();  // wait for end of critical part

  objStorageManager.objecthash.ClearDeleted();

  if ( clean_writes )
    *clean_writes = UObject::clean_writes;
  if ( dirty_writes )
    *dirty_writes = UObject::dirty_writes;
  if ( elapsed_ms )
    *elapsed_ms = timer.ellapsed();
  return res;
}

}  // namespace Core
}  // namespace Pol
