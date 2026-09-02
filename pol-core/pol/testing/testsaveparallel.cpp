/** @file
 *
 * @par History
 */

#include <algorithm>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "clib/rawtypes.h"
#include "clib/streamsaver.h"
#include "pol/containr.h"
#include "pol/globals/uvars.h"
#include "pol/item/item.h"
#include "pol/saveparallel.h"
#include "pol/storage.h"
#include "pol/testing/testenv.h"

namespace Pol::Testing
{
namespace
{
constexpr u32 CONTAINER_OBJTYPE = 0xe75;  // a backpack, per the test shard's itemdesc.cfg
constexpr u32 ITEM_OBJTYPE = 0x0eed;      // gold

/// Enough root items that the file is cut into far more pieces than there are threads, so that
/// piece boundaries land wherever they can land.
constexpr unsigned ROOTS = 1024;
constexpr unsigned ITEMS_PER_ROOT = 3;
constexpr unsigned CPROPS = 2;

/// The text a writer produced, which for a detached writer is everything it was given.
std::string written( const Clib::StreamWriter& sw )
{
  const auto view = sw.buffer();
  return std::string( view.data(), view.size() );
}

/// The `Type [Value]\n{\n..\n}` elements of a data file, in the order they appear.
std::vector<std::string> blocks( const std::string& text )
{
  std::vector<std::string> out;
  for ( size_t at = 0; at < text.size(); )
  {
    const size_t end = text.find( "}\n\n", at );
    if ( end == std::string::npos )
      break;
    out.push_back( text.substr( at, end + 1 - at ) );
    at = end + 3;
  }
  return out;
}

/// Every item block paired with the storage area it would be loaded into, which is the one thing
/// the order of a storage file decides: Storage::read files each Item under the last StorageArea
/// element it saw.
std::vector<std::pair<std::string, std::string>> items_by_area( const std::string& text )
{
  std::vector<std::pair<std::string, std::string>> out;
  std::string area;
  for ( const auto& block : blocks( text ) )
  {
    if ( block.starts_with( "StorageArea" ) )
    {
      const size_t at = block.find( "\tName\t" );
      area = at == std::string::npos ? std::string() : block.substr( at + 6 );
      area = area.substr( 0, area.find( '\n' ) );
    }
    else
      out.emplace_back( area, block );
  }
  return out;
}

size_t count_blocks( const std::string& text, std::string_view type )
{
  const auto all = blocks( text );
  return std::count_if( all.begin(), all.end(),
                        [type]( const std::string& b ) { return b.starts_with( type ); } );
}

void add_cprops( Core::UObject* obj, unsigned seed )
{
  for ( unsigned i = 0; i < CPROPS; ++i )
    obj->setprop( fmt::format( "scriptprop_number_{}", i ),
                  fmt::format( "i{}", 1000000 + ( seed + i ) * 7919 ) );
}
}  // namespace

/// A save splits storage.txt across the task pool, and the pieces reach the file in whatever
/// order the threads finish them. What has to survive that is not the byte order but the filing:
/// Storage::read puts every Item under the last StorageArea element it read, so a piece that
/// lands between the wrong header and its items would be loaded into the wrong area - silently.
/// Hold the split against the unsplit on a world laid out the way a real shard's storage is: one
/// area of many root containers.
void storage_print_test()
{
  // Two areas, so that the piece boundaries fall inside an area as well as between them.
  const std::vector<std::string> area_names{ "aaa_small_area", "bbb_bank" };
  for ( const auto& area_name : area_names )
  {
    auto* area = Core::gamestate.storage.create_area( area_name );
    const unsigned roots = ( area_name == "bbb_bank" ) ? ROOTS : 3;
    for ( unsigned r = 0; r < roots; ++r )
    {
      auto* root = Items::Item::create( CONTAINER_OBJTYPE );
      root->setname( fmt::format( "box_{}_{}", area_name, r ) );
      add_cprops( root, r );
      auto* container = static_cast<Core::UContainer*>( root );
      for ( unsigned i = 0; i < ITEMS_PER_ROOT; ++i )
      {
        auto* item = Items::Item::create( ITEM_OBJTYPE );
        item->setname( fmt::format( "item_{}_{}", r, i ) );
        add_cprops( item, r + i );
        container->add( item );
      }
      area->insert_root_item( root );
    }
  }

  // The same part, formatted in one call on this thread, is the reference for what the pool has
  // to produce. Detached writers, so the text stays in memory to be compared.
  Clib::StreamWriter reference;
  Clib::StreamWriter unused;
  const auto serial_part = Core::gamestate.storage.save_part( reference );
  // One thread, one block: only the first piece opens it.
  for ( size_t piece = 0; piece < serial_part.count; ++piece )
    serial_part.format( piece, Core::ChunkOut{ reference, unused, piece == 0 } );
  const auto expected = written( reference );

  Clib::StreamWriter parallel;
  Core::write_parallel( { Core::gamestate.storage.save_part( parallel ) } );
  const auto actual = written( parallel );

  auto expected_items = items_by_area( expected );
  auto actual_items = items_by_area( actual );
  std::sort( expected_items.begin(), expected_items.end() );
  std::sort( actual_items.begin(), actual_items.end() );

  UnitTest( [&]() { return expected.size() > 0x8000; }, true,
            "the test world is big enough to be split" );
  // One run of pieces is one header, so a split file repeats them - which Storage::create_area,
  // being find-or-create, reads as the same area every time.
  UnitTest( [&]() { return count_blocks( expected, "StorageArea" ); }, area_names.size(),
            "an unsplit storage file opens each area once" );
  UnitTest( [&]() { return count_blocks( actual, "StorageArea" ) >= area_names.size(); }, true,
            "and a split one opens each area at least once" );
  UnitTest( [&]() { return actual_items.size(); }, expected_items.size(),
            "a split storage file holds as many items as an unsplit one" );
  UnitTest( [&]() { return actual_items == expected_items; }, true,
            "every one of them the same item, filed under the same area" );

  // What a worker's buffer holds is the runs it claimed, which are not adjacent pieces: a run of
  // one area can follow a run of another with nothing handed to the file in between. So only the
  // piece that opens a run may read anything into the piece before it in the vector - anywhere
  // else, what precedes it in the file is the last piece of some other run entirely. The pieces
  // are laid out area by area, aaa_small_area's three roots first, so a run inside bbb_bank
  // followed by a run inside aaa_small_area is that case.
  std::map<std::string, std::string> area_of;
  for ( const auto& [area, block] : expected_items )
    area_of[block] = area;

  Clib::StreamWriter mixed;
  for ( const auto& [from, to] : { std::pair<size_t, size_t>{ 10, 13 }, { 1, 4 } } )
  {
    for ( size_t piece = from; piece < to; ++piece )
      serial_part.format( piece, Core::ChunkOut{ mixed, unused, piece == from } );
  }
  const auto mixed_items = items_by_area( written( mixed ) );
  // Three roots per run, each writing itself and its contents.
  UnitTest( [&]() { return mixed_items.size(); }, size_t( 2 * 3 * ( 1 + ITEMS_PER_ROOT ) ),
            "two runs of three root items each" );
  UnitTest(
      [&]()
      {
        return std::all_of( mixed_items.begin(), mixed_items.end(), [&area_of]( const auto& entry )
                            { return area_of[entry.second] == entry.first; } );
      },
      true, "a run of one area following a run of another is still filed under its own" );
}

/// write_parallel cuts several files into pieces and hands them to arbitrary threads. Nothing may
/// be lost, duplicated or crossed between files, and the two-file case - pcs.txt with pcequip.txt,
/// npcs.txt with npcequip.txt - has to get both halves of every piece out. A save also mixes in
/// the files that are one piece and write themselves - accounts.txt, having no writer to hand
/// pieces through, and the small ones - and those must come out whole and *in order*, since one
/// thread writes them start to finish. Build parts whose output says which piece it came from,
/// and check it against the same parts run straight through.
void save_parallel_test()
{
  // Many more pieces than there are threads, so every thread takes several runs of the same part.
  constexpr size_t MANY = 4000;
  constexpr size_t FEW = 37;

  // A piece's text is long enough that the buffers are handed over several times per part, which
  // is where a mistake in what a handover carries would show up.
  auto one_file = []( const char* tag, size_t count )
  {
    return [tag, count]( size_t i, Core::ChunkOut out )
    {
      out.file.begin( tag );
      out.file.add( "Index", i );
      out.file.add( "Total", count );
      out.file.end();
    };
  };
  auto two_files = []( const char* tag )
  {
    return [tag]( size_t i, Core::ChunkOut out )
    {
      out.file.begin( tag );
      out.file.add( "Index", i );
      out.file.end();
      // Two entries in the second file per piece, so a swap between the files cannot pass.
      for ( int half = 0; half < 2; ++half )
      {
        out.equip.begin( "Equipment" );
        out.equip.add( "Owner", i );
        out.equip.add( "Half", half );
        out.equip.end();
      }
    };
  };

  // One writer per file, detached so the text stays in memory to be compared.
  constexpr size_t FILES = 6;
  constexpr size_t WHOLE_FILE = 5;  // written by one thread, so this one does come out in order
  auto run = [&]( bool parallel, std::vector<std::string>& text )
  {
    std::vector<std::unique_ptr<Clib::StreamWriter>> writers;
    for ( size_t f = 0; f < FILES; ++f )
      writers.push_back( std::make_unique<Clib::StreamWriter>() );

    std::vector<Core::SavePart> parts = {
        { .name = "big",
          .count = MANY,
          .file = writers[0].get(),
          .format = one_file( "Big", MANY ) },
        { .name = "paired",
          .count = MANY,
          .file = writers[1].get(),
          .equip = writers[2].get(),
          .format = two_files( "Paired" ) },
        { .name = "small",
          .count = FEW,
          .file = writers[3].get(),
          .format = one_file( "Small", FEW ) },
        // A file with nothing to write still has to come out empty rather than upset the rest.
        { .name = "empty", .count = 0, .file = writers[4].get(), .format = one_file( "Empty", 0 ) },
        // A part that writes its own file and hands nothing back. Its text must arrive whole and
        // in order, and its null `file` must neither be appended to nor disturb the parts that
        // do have one.
        Core::whole_file_part( "whole",
                               [&]()
                               {
                                 for ( size_t i = 0; i < FEW; ++i )
                                 {
                                   writers[WHOLE_FILE]->begin( "Whole" );
                                   writers[WHOLE_FILE]->add( "Index", i );
                                   writers[WHOLE_FILE]->end();
                                 }
                               } ) };

    if ( parallel )
    {
      Core::write_parallel( parts );
    }
    else
    {
      Clib::StreamWriter unused;
      for ( const auto& part : parts )
        for ( size_t piece = 0; piece < part.count; ++piece )
          part.format( piece, Core::ChunkOut{ part.file ? *part.file : unused,
                                              part.equip ? *part.equip : unused, piece == 0 } );
    }
    for ( const auto& writer : writers )
      text.push_back( written( *writer ) );
  };

  std::vector<std::string> expected;
  std::vector<std::string> actual;
  run( false, expected );
  run( true, actual );

  UnitTest( [&]() { return expected[0].size() > 0x8000; }, true,
            "the test parts are big enough to be split" );
  for ( size_t f = 0; f < FILES; ++f )
  {
    auto want = blocks( expected[f] );
    auto got = blocks( actual[f] );
    UnitTest( [&]() { return actual[f].size(); }, expected[f].size(),
              fmt::format( "split file {} is the same size as an unsplit one", f ) );
    UnitTest( [&]() { return got.size(); }, want.size(),
              fmt::format( "and file {} holds the same number of elements", f ) );
    std::sort( want.begin(), want.end() );
    std::sort( got.begin(), got.end() );
    UnitTest( [&]() { return got == want; }, true,
              fmt::format( "and file {} holds exactly the same elements", f ) );
  }
  UnitTest( [&]() { return actual[WHOLE_FILE] == expected[WHOLE_FILE]; }, true,
            "a whole-file part comes out in the order it wrote" );
}

/// A piece that fails - a full disk is the real case - has to bring the save down and reach the
/// caller, without leaving a thread behind. The piece that fails here is the first one, so every
/// other thread is somewhere in the middle of the same file when it happens. If a worker could be
/// left waiting on something the failed one owed it, the save would hang with the world stopped,
/// so this test hanging is the failure.
void save_parallel_failure_test()
{
  constexpr size_t MANY = 4000;
  Clib::StreamWriter file;
  const Core::SavePart part{ .name = "fails",
                             .count = MANY,
                             .file = &file,
                             .format = []( size_t i, Core::ChunkOut out )
                             {
                               if ( i == 0 )
                                 throw std::runtime_error( "no space left on device" );
                               out.file.begin( "Late" );
                               out.file.add( "Index", i );
                               out.file.end();
                             } };

  std::string reported;
  try
  {
    Core::write_parallel( { part } );
  }
  catch ( const std::exception& error )
  {
    reported = error.what();
  }
  UnitTest( [&]() { return reported; }, std::string( "no space left on device" ),
            "a piece that throws takes the save down and says why" );
}
}  // namespace Pol::Testing
