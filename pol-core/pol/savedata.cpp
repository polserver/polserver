/** @file
 *
 * @par History
 * - 2007/06/17 Shinigami: added config.world_data_path
 */


#include "pol/savedata.h"

#include <algorithm>
#include <cerrno>
#include <exception>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <thread>

#include "clib/Debugging/ExceptionParser.h"
#include "clib/Program/ProgramConfig.h"
#include "clib/clib_endian.h"
#include "clib/esignal.h"
#include "clib/fileutil.h"
#include "clib/iohelp.h"
#include "clib/logfacility.h"
#include "clib/rawtypes.h"
#include "clib/streamsaver.h"
#include "clib/strutil.h"
#include "clib/timer.h"
#include "plib/systemstate.h"
#include "pol/accounts/accounts.h"
#include "pol/globals/object_storage.h"
#include "pol/globals/state.h"
#include "pol/globals/uvars.h"
#include "pol/item/item.h"
#include "pol/item/itemdesc.h"
#include "pol/mobile/charactr.h"
#include "pol/mobile/npc.h"
#include "pol/multi/house.h"
#include "pol/multi/multi.h"
#include "pol/objecthash.h"
#include "pol/polsem.h"
#include "pol/realms/realm.h"
#include "pol/regions/resource.h"
#include "pol/saveparallel.h"
#include "pol/storage.h"
#include "pol/ufunc.h"
#include "pol/uobject.h"
#include "pol/uworld.h"

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
    : pol( Plib::systemstate.config.world_data_path + "pol.ndt" ),
      objects( Plib::systemstate.config.world_data_path + "objects.ndt" ),
      pcs( Plib::systemstate.config.world_data_path + "pcs.ndt" ),
      pcequip( Plib::systemstate.config.world_data_path + "pcequip.ndt" ),
      npcs( Plib::systemstate.config.world_data_path + "npcs.ndt" ),
      npcequip( Plib::systemstate.config.world_data_path + "npcequip.ndt" ),
      items( Plib::systemstate.config.world_data_path + "items.ndt" ),
      multis( Plib::systemstate.config.world_data_path + "multis.ndt" ),
      storage( Plib::systemstate.config.world_data_path + "storage.ndt" ),
      resource( Plib::systemstate.config.world_data_path + "resource.ndt" ),
      guilds( Plib::systemstate.config.world_data_path + "guilds.ndt" ),
      datastore( Plib::systemstate.config.world_data_path + "datastore.ndt" ),
      party( Plib::systemstate.config.world_data_path + "parties.ndt" )
{
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

std::array<std::pair<std::string_view, Clib::StreamWriter*>, 13> SaveContext::files()
{
  // Note that `party` commits as "parties".
  return { { { "pol", &pol },
             { "objects", &objects },
             { "pcs", &pcs },
             { "pcequip", &pcequip },
             { "npcs", &npcs },
             { "npcequip", &npcequip },
             { "items", &items },
             { "multis", &multis },
             { "storage", &storage },
             { "resource", &resource },
             { "guilds", &guilds },
             { "datastore", &datastore },
             { "parties", &party } } };
}

SaveContext::~SaveContext() noexcept( false )
{
  auto stack_unwinding = std::uncaught_exceptions();
  try
  {
    // A save that got this far has already flushed and closed everything itself, so that the i/o
    // could be timed and kept out of the critical part. This is the cleanup path for the rest.
    for ( auto& file : files() )
      file.second->flush_close();
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
void WriteGottenItem( Items::Item* item, Clib::StreamWriter& sw )
{
  if ( item == nullptr || item->orphan() )
    return;
  // Saved among the ordinary items, at the position local_position() gives it -- which for
  // something on a cursor is wherever its holder stands. This used to move the item there and back
  // around the write, because printOn had no other way to be told.
  item->printOn( sw );
}

/// The mobiles a save has to write, in the order the object hash lists them.
///
/// Collected once and shared. The hash holds every item on the shard as well as every mobile, so
/// on a large shard it is a tree of millions of nodes; write_characters, write_npcs and the
/// gotten-item pass of write_items used to each walk all of it to find a few thousand mobiles.
struct MobileSnapshot
{
  std::vector<Mobile::Character*> pcs;
  std::vector<Mobile::Character*> npcs;  // only the ones that get saved
};

MobileSnapshot collect_mobiles()
{
  MobileSnapshot mobiles;
  // An NPC counts as a character too, so the players are the characters left over. Both are an
  // upper bound - orphans and NPCs that are not saved drop out below - so the vectors are sized
  // once instead of growing a pointer at a time.
  const int npc_count = stateManager.uobjcount.npc_count;
  mobiles.pcs.reserve( std::max( 0, stateManager.uobjcount.ucharacter_count - npc_count ) );
  mobiles.npcs.reserve( npc_count );
  objStorageManager.objecthash.for_each_character(
      [&mobiles]( UObject* obj )
      {
        if ( !obj->ismobile() || obj->orphan() )
          return;
        Mobile::Character* chr = static_cast<Mobile::Character*>( obj );
        if ( !chr->isa( UOBJ_CLASS::CLASS_NPC ) )
          mobiles.pcs.push_back( chr );
        else if ( chr->saveonexit() )
          mobiles.npcs.push_back( chr );
      } );
  return mobiles;
}

/// Every top-level item of every realm, flattened so that the file can be split. The filter is
/// applied here rather than per piece, so that the pieces hold only items that get written and
/// therefore carry comparable amounts of work.
std::vector<Items::Item*> collect_toplevel_items()
{
  std::vector<Items::Item*> items;
  items.reserve( get_toplevel_item_count() );
  for ( const auto& realm : gamestate.Realms )
  {
    for ( const auto& p : realm->gridarea() )
    {
      for ( const auto& item : realm->getzone_grid( p ).items )
      {
        if ( item->itemdesc().save_on_exit && item->saveonexit() )
          items.push_back( item );
      }
    }
  }
  return items;
}

SavePart items_part( const std::vector<Items::Item*>& items, Clib::StreamWriter& sw )
{
  return {
      .name = "items",
      .count = items.size(),
      .writers = { &sw },
      .format = [&items]( size_t begin, size_t end, const std::vector<Clib::StreamWriter*>& out )
      {
        for ( size_t i = begin; i < end; ++i )
        {
          items[i]->printOn( *out[0] );
          items[i]->clear_dirty();
        }
      } };
}

/// One mobile per piece, its worn items included. Both files it writes to are split the same way,
/// so each keeps the order a single thread would have given it.
SavePart mobiles_part( std::string name, const std::vector<Mobile::Character*>& mobiles,
                       Clib::StreamWriter& sw_mobiles, Clib::StreamWriter& sw_equip )
{
  return {
      .name = std::move( name ),
      .count = mobiles.size(),
      .writers = { &sw_mobiles, &sw_equip },
      .format = [&mobiles]( size_t begin, size_t end, const std::vector<Clib::StreamWriter*>& out )
      {
        for ( size_t i = begin; i < end; ++i )
          mobiles[i]->printForSave( *out[0], *out[1] );
      } };
}

/// Items on a player's cursor, written after all the top-level ones as they always have been.
/// There is one at most per player, so there is nothing here worth splitting.
void write_gotten_items( Clib::StreamWriter& sw_items, const MobileSnapshot& mobiles )
{
  for ( Mobile::Character* chr : mobiles.pcs )
  {
    // Figure out where to save the 'gotten item' - Austin (Oct. 17, 2006)
    if ( chr->has_gotten_item() )
      WriteGottenItem( chr->gotten_item().item(), sw_items );
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

namespace
{
/// Break a finished save down into where its time and its bytes went, slowest and biggest first.
void log_save_details( const SaveResult& stats )
{
  auto tasks = stats.tasks;
  std::sort( tasks.begin(), tasks.end(),
             []( const auto& a, const auto& b ) { return a.elapsed_ms > b.elapsed_ms; } );
  auto files = stats.files;
  std::sort( files.begin(), files.end(),
             []( const auto& a, const auto& b ) { return a.bytes > b.bytes; } );

  fmt::memory_buffer buffer;
  auto out = std::back_inserter( buffer );
  fmt::format_to( out,
                  "Worldsave {}: {} ms world stopped ({} ms of it writing), {} ms flushing, {} ms "
                  "committing",
                  stats.success ? "ok" : "FAILED", stats.critical_ms, stats.write_ms,
                  stats.flush_ms, stats.commit_ms );
  fmt::format_to( out, "\n  parts (work, summed over the threads that shared it):" );
  for ( const auto& task : tasks )
    fmt::format_to( out, "\n    {:<10} {:>7} ms", task.name, task.elapsed_ms );
  fmt::format_to( out, "\n  files:" );
  for ( const auto& file : files )
    fmt::format_to( out, "\n    {:<10} {:>13} bytes", file.name, file.bytes );
  POLLOG_INFOLN( "{}", std::string_view( buffer.data(), buffer.size() ) );
}
}  // namespace

std::optional<bool> write_data( std::function<void( const SaveResult& )> callback,
                                SaveResult* critical_result )
{
  SaveContext::ready();  // allow only one active
  if ( !should_write_data() )
    return {};

  UObject::dirty_writes = 0;
  UObject::clean_writes = 0;

  // launch complete save as seperate thread
  // but wait till the first critical part is finished
  // which means all objects got written and the world is free to run again
  // the remaining operations are only pure buffered i/o
  auto critical_promise = std::promise<SaveResult>();
  auto critical_future = critical_promise.get_future();
  auto set_promise = []( auto& promise, SaveResult result )
  {
    try  // guard to be able to try to set it twice (exceptions)
    {
      promise.set_value( std::move( result ) );
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
        SaveResult stats;
        std::mutex stats_mutex;  // guards stats.tasks, appended once per finished task
        try
        {
          SaveContext sc;
          std::vector<std::future<bool>> critical_parts;
          // Wrap one part of the save so that a failure is logged and recorded rather than
          // escaping, and so that its duration lands in the report.
          auto timed = [&]( auto func, std::string name )
          {
            return [&, name, func = std::move( func )]() mutable
            {
              Tools::Timer<> task_timer;
              try
              {
                func();
              }
              catch ( const std::exception& error )
              {
                POLLOG_ERRORLN( "failed to store {} datafile! {}\n{}", name, error.what(),
                                Clib::ExceptionParser::getTrace() );
                result = false;
              }
              catch ( ... )
              {
                POLLOG_ERRORLN( "failed to store {} datafile!\n{}", name,
                                Clib::ExceptionParser::getTrace() );
                result = false;
              }
              std::lock_guard<std::mutex> lock( stats_mutex );
              stats.tasks.push_back( { std::move( name ), task_timer.ellapsed() } );
            };
          };
          auto save = [&]( auto func, std::string name )
          {
            critical_parts.push_back( gamestate.task_thread_pool.checked_push(
                timed( std::move( func ), std::move( name ) ) ) );
          };

          // The small files, one thread each. Submitted first so that they are picked up while
          // the big ones below are still being cut up.
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

          // The four files that are actually big, cut into pieces and spread over the pool as one
          // pool of work. Giving each of them a thread of its own would make the save wait for
          // storage, which on a large shard is most of it. Run on this thread rather than as a
          // pool task: write_parallel waits on pool work, and a pool thread waiting on the pool
          // can leave nothing free to make progress with.
          try
          {
            // Working out what there is to write, before any of it is formatted.
            Tools::Timer<> scan_timer;
            const auto mobiles = collect_mobiles();
            const auto toplevel_items = collect_toplevel_items();
            const auto scan_ms = scan_timer.ellapsed();
            auto work = write_parallel(
                { gamestate.storage.save_part( sc.storage ), items_part( toplevel_items, sc.items ),
                  mobiles_part( "character", mobiles.pcs, sc.pcs, sc.pcequip ),
                  mobiles_part( "npcs", mobiles.npcs, sc.npcs, sc.npcequip ) } );
            write_gotten_items( sc.items, mobiles );

            std::lock_guard<std::mutex> lock( stats_mutex );
            stats.write_ms = work.write_ms;
            stats.tasks.push_back( { "scan", scan_ms } );
            for ( const auto& part : work.work )
              stats.tasks.push_back( { part.first, part.second } );
          }
          catch ( const std::exception& error )
          {
            POLLOG_ERRORLN( "failed to store the object datafiles! {}\n{}", error.what(),
                            Clib::ExceptionParser::getTrace() );
            result = false;
          }
          catch ( ... )
          {
            POLLOG_ERRORLN( "failed to store the object datafiles!\n{}",
                            Clib::ExceptionParser::getTrace() );
            result = false;
          }

          for ( auto& task : critical_parts )
            task.wait();

          // Counted before the final flush: bytes_written() includes the buffered tail, so this is
          // the whole payload each file received while the world was stopped.
          for ( const auto& file : sc.files() )
            stats.files.push_back( { file.first, file.second->bytes_written() } );

          blocking_timer.stop();
          stats.critical_ms = blocking_timer.ellapsed();
          stats.success = result;
          set_promise( critical_promise, stats );  // critical part end

          // The world runs again from here on. Draining the buffers and closing the files no
          // longer touches any object.
          Tools::Timer<> flush_timer;
          for ( auto& file : sc.files() )
            file.second->flush_close();
          stats.flush_ms = flush_timer.ellapsed();
        }  // deconstructor of the SaveContext closes whatever is left open
        catch ( std::ios_base::failure& e )
        {
          POLLOG_ERRORLN( "failed to save datafiles! {}:{}\n{}", e.what(), std::strerror( errno ),
                          Clib::ExceptionParser::getTrace() );

          result = false;
          stats.success = false;
          set_promise( critical_promise, stats );
        }
        catch ( ... )
        {
          POLLOG_ERRORLN( "failed to save datafiles!\n{}", Clib::ExceptionParser::getTrace() );
          result = false;
          stats.success = false;
          set_promise( critical_promise, stats );
        }
        if ( result )
        {
          Tools::Timer<> commit_timer;
          result = std::all_of( stats.files.begin(), stats.files.end(), []( const auto& file )
                                { return commit( std::string( file.name ) ); } );
          stats.commit_ms = commit_timer.ellapsed();
          if ( result )
            SaveContext::last_worldsave_success = read_gameclock();
        }
        stats.success = result;
        stats.clean_writes = UObject::clean_writes;
        stats.dirty_writes = UObject::dirty_writes;
        if ( Plib::systemstate.config.log_worldsave_details )
          log_save_details( stats );
        if ( callback )
          callback( stats );
      } );
  auto res = critical_future.get();  // wait for end of critical part

  objStorageManager.objecthash.ClearDeleted();

  res.clean_writes = UObject::clean_writes;
  res.dirty_writes = UObject::dirty_writes;
  const bool success = res.success;
  if ( critical_result )
    *critical_result = std::move( res );
  return success;
}

}  // namespace Core
}  // namespace Pol
