/** @file
 *
 * @par History
 */

#include <cstdio>
#include <stdexcept>
#include <string>
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

void add_cprops( Core::UObject* obj, unsigned seed )
{
  for ( unsigned i = 0; i < CPROPS; ++i )
    obj->setprop( fmt::format( "scriptprop_number_{}", i ),
                  fmt::format( "i{}", 1000000 + ( seed + i ) * 7919 ) );
}
}  // namespace

/// A save splits storage.txt across the task pool and appends the pieces in order. The output has
/// to stay byte for byte what one thread would have written, or every existing storage.txt becomes
/// unreadable, so hold the split against the unsplit on a world laid out the way a real shard's
/// storage is: one area of many root containers.
void storage_print_test()
{
  // Two areas, so that the piece boundaries fall inside an area as well as between them.
  for ( const auto& area_name : { std::string( "aaa_small_area" ), std::string( "bbb_bank" ) } )
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
  serial_part.format( 0, serial_part.count, Core::ChunkOut{ reference, unused } );
  const auto expected = written( reference );

  Clib::StreamWriter parallel;
  Core::write_parallel( { Core::gamestate.storage.save_part( parallel ) } );
  const auto actual = written( parallel );

  UnitTest( [&]() { return expected.size() > 0x8000; }, true,
            "the test world is big enough to be split" );
  UnitTest( [&]() { return actual.size(); }, expected.size(),
            "a split storage file is the same size as an unsplit one" );
  UnitTest( [&]() { return actual == expected; }, true, "and byte for byte the same text" );
}

/// write_parallel cuts several files into pieces, hands them to arbitrary threads and appends the
/// results. Getting the order wrong would corrupt every save file at once, and the two-file case
/// -- pcs.txt with pcequip.txt, npcs.txt with npcequip.txt -- has to keep both in step. A save
/// also mixes in the files that are one piece and write themselves, which must come out whole
/// without disturbing the rest. Build parts whose output says which piece it came from, and check
/// it against the same parts run straight through.
void save_parallel_test()
{
  // Many more pieces than there are threads, so every thread takes several chunks of the same
  // part and has to wait for its turn to append more than once.
  constexpr size_t MANY = 4000;
  constexpr size_t FEW = 37;

  // A piece's text is long enough that the chunks overflow their buffers, which is where an
  // ordering mistake between a chunk's own flushes would show up.
  auto one_file = []( const char* tag, size_t count )
  {
    return [tag, count]( size_t begin, size_t end, Core::ChunkOut out )
    {
      for ( size_t i = begin; i < end; ++i )
      {
        out.file.begin( tag );
        out.file.add( "Index", i );
        out.file.add( "Total", count );
        out.file.end();
      }
    };
  };
  auto two_files = []( const char* tag )
  {
    return [tag]( size_t begin, size_t end, Core::ChunkOut out )
    {
      for ( size_t i = begin; i < end; ++i )
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
      }
    };
  };

  // One writer per file, detached so the text stays in memory to be compared.
  constexpr size_t FILES = 6;
  auto run = [&]( bool parallel, std::vector<std::string>& text )
  {
    std::vector<std::unique_ptr<Clib::StreamWriter>> writers;
    for ( size_t f = 0; f < FILES; ++f )
      writers.push_back( std::make_unique<Clib::StreamWriter>() );

    std::vector<Core::SavePart> parts = {
        { "big", MANY, writers[0].get(), nullptr, one_file( "Big", MANY ) },
        { "paired", MANY, writers[1].get(), writers[2].get(), two_files( "Paired" ) },
        { "small", FEW, writers[3].get(), nullptr, one_file( "Small", FEW ) },
        // A file with nothing to write still has to come out empty rather than upset the rest.
        { "empty", 0, writers[4].get(), nullptr, one_file( "Empty", 0 ) },
        // A part that writes its own file and hands nothing back. Its text must arrive whole and
        // its null `file` must neither be appended to nor disturb the parts that do have one.
        Core::whole_file_part( "whole",
                               [&]()
                               {
                                 for ( size_t i = 0; i < FEW; ++i )
                                 {
                                   writers[5]->begin( "Whole" );
                                   writers[5]->add( "Index", i );
                                   writers[5]->end();
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
        part.format(
            0, part.count,
            Core::ChunkOut{ part.file ? *part.file : unused, part.equip ? *part.equip : unused } );
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
    UnitTest( [&]() { return actual[f].size(); }, expected[f].size(),
              fmt::format( "split file {} is the same size as an unsplit one", f ) );
    UnitTest( [&]() { return actual[f] == expected[f]; }, true,
              fmt::format( "and file {} is byte for byte the same text", f ) );
  }
}

/// A piece that fails - a full disk is the real case - has to bring the save down and reach the
/// caller. The piece that fails here is the first one, which is the shape that would strand every
/// other thread: they are all holding a later piece of the same file, waiting for a turn that is
/// never going to be bumped. If that release goes wrong the save hangs with the world stopped, so
/// this test hanging is the failure.
void save_parallel_failure_test()
{
  constexpr size_t MANY = 4000;
  Clib::StreamWriter file;
  const Core::SavePart part{ .name = "fails",
                             .count = MANY,
                             .file = &file,
                             .format = []( size_t begin, size_t end, Core::ChunkOut out )
                             {
                               if ( begin == 0 )
                                 throw std::runtime_error( "no space left on device" );
                               for ( size_t i = begin; i < end; ++i )
                               {
                                 out.file.begin( "Late" );
                                 out.file.add( "Index", i );
                                 out.file.end();
                               }
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
