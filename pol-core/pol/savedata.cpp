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
Core::SavePart datastore_part( Clib::StreamWriter& sw );
}  // namespace Module
namespace Core
{
void write_party( Clib::StreamWriter& sw );
void write_guilds( Clib::StreamWriter& sw );

std::shared_future<void> SaveContext::finished;
std::atomic<gameclock_t> SaveContext::last_worldsave_success = 0;

SaveContext::SaveContext()
    : pol( Plib::systemstate.config.world_data_path + "pol.ndt" ),
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
      parties( Plib::systemstate.config.world_data_path + "parties.ndt" )
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

  parties.comment( "" );
  parties.comment( " PARTIES.TXT: Party Data" );
  parties.comment( "\n" );
}

std::array<std::pair<std::string_view, Clib::StreamWriter*>, 12> SaveContext::files()
{
  return { { { "pol", &pol },
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
             { "parties", &parties } } };
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
  sw.add<"CoreVersion">( POL_VERSION_STR );
  sw.add<"CoreVersionString">( POL_VERSION_STR );
  sw.add<"CompileDateTime">( Clib::ProgramConfig::build_datetime() );
  sw.add<"LastItemSerialNumber">( GetCurrentItemSerialNumber() );
  sw.add<"LastCharSerialNumber">( GetCurrentCharSerialNumber() );
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
      sw.add<"Name">( realm->name() );
    }
    else
    {
      sw.add<"Name">( realm->shadowname );
      sw.add<"BaseRealm">( realm->baserealm->name() );
    }
    sw.add<"HasDecay">( realm->has_decay );
    sw.end();
  }
}

// Austin (Oct. 17, 2006)
/// pol.txt: the header a shard is identified by, plus the globals and the realm list.
void write_pol_file( Clib::StreamWriter& sw )
{
  sw.comment( "" );
  sw.comment( " Created by Version: {}", POL_VERSION_ID );
  sw.comment( " Mobiles: {}", get_mobile_count() );
  sw.comment( " Top-level Items: {}", get_toplevel_item_count() );
  sw.comment( "\n" );

  write_system_data( sw );
  write_global_properties( sw );
  write_realms( sw );
}

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
/// on a large shard it is a tree of millions of nodes; finding a few thousand mobiles used to walk
/// all of it three times over.
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

/// One mobile per piece, its worn items included, so a mobile and its equipment are never split
/// between two threads.
void write_mobile( Mobile::Character* chr, ChunkOut out )
{
  chr->printForSave( out.file, out.equip );
}

/// Every top-level item of every realm, flattened so that the file can be split. The filter is
/// applied here rather than per piece, so that the pieces hold only items that get written and
/// therefore carry comparable amounts of work.
std::vector<Items::Item*> collect_toplevel_items()
{
  std::vector<Items::Item*> items;
  items.reserve( get_toplevel_item_count() );
  for ( auto* realm : gamestate.Realms )
  {
    // The whole realm is one box, so the iterator's own range check is always true -- it is here
    // because walking the zone grid is what the core does to enumerate a realm's items.
    WorldIterator<ItemFilter>::InBox( realm->area(), realm,
                                      [&items]( Items::Item* item )
                                      {
                                        if ( item->itemdesc().save_on_exit && item->saveonexit() )
                                          items.push_back( item );
                                      } );
  }
  return items;
}

/// One top-level item per piece, its contents included: printOn walks a container's whole subtree,
/// so a parent and its children are never split between two threads.
void write_item( Items::Item* item, ChunkOut out )
{
  item->printOn( out.file );
  item->clear_dirty();
}

/// Items on a player's cursor, appended to items.txt after all the top-level ones as they always
/// have been.
///
/// Runs once the pool has finished rather than as a piece of the items part: it reads the mobiles
/// that the character part is writing, and that part clears a dirty flag on each of them as it
/// goes. There is at most one of these per player, so there is nothing here worth overlapping
/// anything with.
void write_gotten_items( Clib::StreamWriter& sw_items, const MobileSnapshot& mobiles )
{
  for ( Mobile::Character* chr : mobiles.pcs )
  {
    // Figure out where to save the 'gotten item' - Austin (Oct. 17, 2006)
    if ( chr->has_gotten_item() )
      WriteGottenItem( chr->gotten_item().item(), sw_items );
  }
}

/// Drop the working design of any custom house left mid-edit, which a shutdown has to do before
/// the save decides what there is to write.
///
/// Separate from writing multis.txt: the commit destroys the house's editable component items,
/// and those are ordinary top-level items, so collect_toplevel_items would otherwise hand one to a
/// worker that formats it after it has been freed. Doing it here, on this thread and before
/// anything is collected, is also what keeps destroy_item - which touches the object hash - off
/// the save's worker threads.
void settle_pending_house_commits()
{
  if ( !Clib::exit_signalled )
    return;
  for ( auto* realm : gamestate.Realms )
  {
    WorldIterator<MultiFilter>::InBox(
        realm->area(), realm,
        []( Multi::UMulti* multi )
        {
          Multi::UHouse* house = multi->as_house();
          if ( house != nullptr && house->IsCustom() && house->IsWaitingForAccept() )
            house->AcceptHouseCommit( nullptr, false );
        } );
  }
}

/// Every multi of every realm, flattened so that the file can be split.
std::vector<Multi::UMulti*> collect_multis()
{
  std::vector<Multi::UMulti*> multis;
  for ( auto* realm : gamestate.Realms )
  {
    WorldIterator<MultiFilter>::InBox(
        realm->area(), realm, [&multis]( Multi::UMulti* multi ) { multis.push_back( multi ); } );
  }
  return multis;
}

/// One multi per piece. A custom house writes its whole design here, which on a housing shard is
/// most of what multis.txt weighs.
void write_multi( Multi::UMulti* multi, ChunkOut out )
{
  multi->printOn( out.file );
  multi->clear_dirty();
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
                  "Worldsave {}: {} ms world stopped, {} ms writing and {} ms waiting for a file "
                  "summed over the threads, {} ms flushing, {} ms committing",
                  stats.success ? "ok" : "FAILED", stats.critical_ms, stats.write_ms, stats.wait_ms,
                  stats.flush_ms, stats.commit_ms );
  fmt::format_to( out, "\n  parts (work, summed over the threads that shared it):" );
  for ( const auto& task : tasks )
  {
    fmt::format_to( out, "\n    {:<10} {:>7} ms", task.name, task.elapsed_ms );
    if ( task.pieces != 0 )  // "scan" is not a part and was not cut into any
      fmt::format_to( out, " {:>10} {}", task.pieces, task.pieces == 1 ? "piece" : "pieces" );
    if ( task.biggest_piece != 0 )  // a part that writes its own file measures nothing
      fmt::format_to( out, ", biggest {} KB",
                      ( task.biggest_piece + 1023 ) / 1024 );  // rounded up, never a bare 0
  }
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
        bool result = true;
        SaveResult stats;
        try
        {
          SaveContext sc;
          try
          {
            // Working out what there is to write, before any of it is formatted. Building the
            // part list is the whole of it: each part is handed the objects it will write.
            Tools::Timer<> scan_timer;
            settle_pending_house_commits();  // destroys items, so before anything is collected
            auto mobiles = collect_mobiles();
            // The parts that are one indivisible piece lead, because a worker claims parts in
            // list order and the save cannot finish before its longest single piece does. They
            // are all small; accounts is the one that has to be a part of its own, having no
            // SaveContext writer to hand pieces to.
            auto parts = std::vector<SavePart>{
                whole_file_part( "accounts",
                                 [&]()
                                 {
                                   if ( Plib::systemstate.accounts_txt_dirty )
                                     Accounts::write_account_data();
                                 } ),
                whole_file_part( "pol", [&]() { write_pol_file( sc.pol ); } ),
                whole_file_part( "resource", [&]() { write_resources_dat( sc.resource ); } ),
                whole_file_part( "guilds", [&]() { write_guilds( sc.guilds ); } ),
                whole_file_part( "parties", [&]() { write_party( sc.parties ); } ),
                Module::datastore_part( sc.datastore ), gamestate.storage.save_part( sc.storage ),
                object_part( "multis", collect_multis(), &sc.multis, nullptr, write_multi ),
                object_part( "items", collect_toplevel_items(), &sc.items, nullptr, write_item ),
                // pcs is copied rather than moved: the gotten-item pass below needs it again.
                object_part( "character", mobiles.pcs, &sc.pcs, &sc.pcequip, write_mobile ),
                object_part( "npcs", std::move( mobiles.npcs ), &sc.npcs, &sc.npcequip,
                             write_mobile ) };
            const auto scan_ms = scan_timer.ellapsed();

            auto work = write_parallel( parts );
            write_gotten_items( sc.items, mobiles );
            // Atomically (hopefully) perform the switch. Retiring a datafile's previous
            // generation has to wait until every datafile has written its new one.
            Module::commit_datastore();

            stats.write_ms = work.write_ms;
            stats.wait_ms = work.wait_ms;
            stats.tasks = std::move( work.work );
            // Not a part: no pieces, and nothing of it passed through a buffer.
            stats.tasks.push_back( { .name = "scan", .elapsed_ms = scan_ms } );
          }
          catch ( const std::exception& error )
          {
            POLLOG_ERRORLN( "failed to store the world datafiles! {}\n{}", error.what(),
                            Clib::ExceptionParser::getTrace() );
            result = false;
          }
          catch ( ... )
          {
            POLLOG_ERRORLN( "failed to store the world datafiles!\n{}",
                            Clib::ExceptionParser::getTrace() );
            result = false;
          }

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
        // Handed to another thread rather than run here, because the callback takes the world
        // lock and SaveContext::finished must not span a lock acquisition: write_data is called
        // from the scripts thread with that lock held, and opens by waiting on finished. Running
        // the callback before finished is set would mean this thread waits for a lock the thread
        // waiting for us is holding - the shard wedges, and back-to-back saves are how you get
        // there. Nothing waits on the callback, so it is free to block for the lock.
        if ( callback )
          gamestate.save_callback_pool.push(
              [callback = std::move( callback ), stats = std::move( stats )]() mutable
              { callback( stats ); } );
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
