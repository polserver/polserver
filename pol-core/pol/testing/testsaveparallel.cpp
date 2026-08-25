/** @file
 *
 * @par History
 */

#include <cstdio>
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

/// Enough root items that Storage::print takes its parallel path: it wants more pieces than
/// PARALLEL_CHUNKS_PER_THREAD per worker thread before it bothers splitting the work.
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

/// Storage::print splits a big file across the task pool and appends the pieces in order. The
/// output has to stay byte for byte what one thread would have written, or every existing
/// storage.txt becomes unreadable, so hold the two paths against each other on a world laid out
/// the way a real shard's storage is: one area of many root containers.
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

  Clib::StreamWriter reference;
  Core::gamestate.storage.print_single_threaded( reference );
  const auto expected = written( reference );

  Clib::StreamWriter parallel;
  Core::gamestate.storage.print( parallel );
  const auto actual = written( parallel );

  UnitTest( [&]() { return expected.size() > 0x8000; }, true,
            "the test world is big enough to be split" );
  UnitTest( [&]() { return actual.size(); }, expected.size(),
            "a split storage file is the same size as an unsplit one" );
  UnitTest( [&]() { return actual == expected; }, true, "and byte for byte the same text" );
}

/// write_parallel cuts several files into pieces, hands them to arbitrary threads and appends the
/// results. Getting the order wrong would corrupt every save file at once, and the two-file case
/// -- pcs.txt with pcequip.txt, npcs.txt with npcequip.txt -- has to keep both in step. Build
/// parts whose output says which piece it came from, and check it against the same parts run
/// straight through.
void save_parallel_test()
{
  // Well past the point where write_parallel stops formatting on one thread.
  constexpr size_t MANY = 4000;
  constexpr size_t FEW = 37;

  // A piece's text is long enough that the chunks overflow their buffers, which is where an
  // ordering mistake between a chunk's own flushes would show up.
  auto one_file = []( const char* tag, size_t count )
  {
    return [tag, count]( size_t begin, size_t end, const std::vector<Clib::StreamWriter*>& out )
    {
      for ( size_t i = begin; i < end; ++i )
      {
        out[0]->begin( tag );
        out[0]->add( "Index", i );
        out[0]->add( "Total", count );
        out[0]->end();
      }
    };
  };
  auto two_files = []( const char* tag )
  {
    return [tag]( size_t begin, size_t end, const std::vector<Clib::StreamWriter*>& out )
    {
      for ( size_t i = begin; i < end; ++i )
      {
        out[0]->begin( tag );
        out[0]->add( "Index", i );
        out[0]->end();
        // Two entries in the second file per piece, so a swap between the files cannot pass.
        for ( int half = 0; half < 2; ++half )
        {
          out[1]->begin( "Equipment" );
          out[1]->add( "Owner", i );
          out[1]->add( "Half", half );
          out[1]->end();
        }
      }
    };
  };

  // One writer per file, detached so the text stays in memory to be compared.
  constexpr size_t FILES = 5;
  auto run = [&]( bool parallel, std::vector<std::string>& text )
  {
    std::vector<std::unique_ptr<Clib::StreamWriter>> writers;
    for ( size_t f = 0; f < FILES; ++f )
      writers.push_back( std::make_unique<Clib::StreamWriter>() );

    std::vector<Core::SavePart> parts = {
        { "big", MANY, { writers[0].get() }, one_file( "Big", MANY ) },
        { "paired", MANY, { writers[1].get(), writers[2].get() }, two_files( "Paired" ) },
        { "small", FEW, { writers[3].get() }, one_file( "Small", FEW ) },
        // A file with nothing to write still has to come out empty rather than upset the rest.
        { "empty", 0, { writers[4].get() }, one_file( "Empty", 0 ) } };

    if ( parallel )
    {
      Core::write_parallel( parts );
    }
    else
    {
      for ( const auto& part : parts )
        part.format( 0, part.count, part.writers );
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
}  // namespace Pol::Testing
