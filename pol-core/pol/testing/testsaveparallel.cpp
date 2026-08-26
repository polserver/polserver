/** @file
 *
 * @par History
 */

#include <algorithm>
#include <cstdio>
#include <set>
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
/// CHUNKS_PER_THREAD per worker thread before it bothers splitting the work.
constexpr unsigned ROOTS = 1024;
constexpr unsigned ITEMS_PER_ROOT = 3;
constexpr unsigned CPROPS = 2;

/// The text a writer produced, which for a detached writer is everything it was given.
std::string written( const Clib::StreamWriter& sw )
{
  const auto view = sw.buffer();
  return std::string( view.data(), view.size() );
}

/// That text cut into the blocks begin()/end() wrote it as. Written in pieces by several threads,
/// a file holds the same blocks every time but not in the same order, so this is the unit any
/// comparison has to be made in.
std::vector<std::string> blocks( const Clib::StreamWriter& sw )
{
  const std::string text = written( sw );
  const std::string sep = "}\n\n";
  std::vector<std::string> out;
  size_t pos = 0;
  for ( size_t at = text.find( sep, pos ); at != std::string::npos; at = text.find( sep, pos ) )
  {
    out.push_back( text.substr( pos, at + sep.size() - pos ) );
    pos = at + sep.size();
  }
  if ( pos < text.size() )
    out.push_back( text.substr( pos ) );  // anything trailing that is not a block at all
  return out;
}

bool is_area( const std::string& block )
{
  return block.compare( 0, 12, "StorageArea\n" ) == 0;
}

/// The value of one key inside a block, or "" if it has no such key. Blocks are "\tKey\tValue\n".
std::string value_of( const std::string& block, const std::string& key )
{
  const std::string needle = "\t" + key + "\t";
  const size_t at = block.find( needle );
  if ( at == std::string::npos )
    return {};
  const size_t from = at + needle.size();
  return block.substr( from, block.find( '\n', from ) - from );
}

void add_cprops( Core::UObject* obj, unsigned seed )
{
  for ( unsigned i = 0; i < CPROPS; ++i )
    obj->setprop( fmt::format( "scriptprop_number_{}", i ),
                  fmt::format( "i{}", 1000000 + ( seed + i ) * 7919 ) );
}
}  // namespace

/// Storage::print splits a big file across the task pool and appends the pieces as they finish, so
/// the blocks come out in no particular order. Two things have to hold anyway, and between them
/// they are what makes the file loadable: every block that a single thread would have written is
/// there exactly once, and every item is preceded by the StorageArea element that puts it in the
/// right area - Storage::read files an item under the last one it saw. Checked on a world laid out
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
  const auto expected = blocks( reference );

  Clib::StreamWriter parallel;
  Core::gamestate.storage.print( parallel );
  const auto actual = blocks( parallel );

  UnitTest( [&]() { return written( reference ).size() > 0x8000; }, true,
            "the test world is big enough to be split" );

  // The items, as a multiset. Area elements are left out: a piece opens its own area, so the
  // parallel file legitimately carries more of them - one per piece rather than one per area.
  auto items_only = []( const std::vector<std::string>& all )
  {
    std::vector<std::string> items;
    for ( const auto& block : all )
      if ( !is_area( block ) )
        items.push_back( block );
    std::sort( items.begin(), items.end() );
    return items;
  };
  UnitTest( [&]() { return items_only( actual ) == items_only( expected ); }, true,
            "a split storage file holds exactly the items an unsplit one does" );

  // Every area an unsplit file names is named in the split one too, and no others.
  auto area_names = []( const std::vector<std::string>& all )
  {
    std::set<std::string> names;
    for ( const auto& block : all )
      if ( is_area( block ) )
        names.insert( value_of( block, "Name" ) );
    return names;
  };
  UnitTest( [&]() { return area_names( actual ) == area_names( expected ); }, true,
            "and names the same storage areas" );

  // The invariant the loader depends on: an item is filed under the last area element before it.
  // The roots are named after the area they belong in, so a piece that failed to open its own
  // area would show up here as an item in the wrong one.
  UnitTest(
      [&]()
      {
        std::string open_area;
        for ( const auto& block : actual )
        {
          if ( is_area( block ) )
          {
            open_area = value_of( block, "Name" );
            continue;
          }
          if ( open_area.empty() )
            return false;  // an item with no area to put it in
          const std::string name = value_of( block, "Name" );
          // Only the roots carry the area in their name; the items inside them do not.
          if ( name.compare( 0, 4, "box_" ) == 0 &&
               name.find( open_area ) == std::string::npos )
            return false;
        }
        return true;
      },
      true, "and every item follows the storage area it belongs to" );
}

/// write_parallel cuts several files into pieces and hands them to arbitrary threads. Losing or
/// duplicating a piece would corrupt every save file at once; the two-file case -- pcs.txt with
/// pcequip.txt, npcs.txt with npcequip.txt -- has to keep each piece's halves in the file they
/// belong to; and a part marked `ordered` has to come out in exactly the order one thread would
/// have written it, which is what items.txt depends on. Build parts whose output says which piece
/// it came from, and check them against the same parts run straight through.
void save_parallel_test()
{
  // Well past the point where write_parallel stops formatting on one thread.
  constexpr size_t MANY = 4000;
  constexpr size_t FEW = 37;

  // A piece's text is long enough that the pieces overflow their buffers, which is where losing
  // part of one would show up.
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
  constexpr size_t FILES = 6;
  constexpr size_t ORDERED_FILE = 5;
  auto run = [&]( bool parallel, std::vector<std::vector<std::string>>& text )
  {
    std::vector<std::unique_ptr<Clib::StreamWriter>> writers;
    for ( size_t f = 0; f < FILES; ++f )
      writers.push_back( std::make_unique<Clib::StreamWriter>() );

    std::vector<Core::SavePart> parts = {
        { "big", MANY, { writers[0].get() }, one_file( "Big", MANY ) },
        { "paired", MANY, { writers[1].get(), writers[2].get() }, two_files( "Paired" ) },
        { "small", FEW, { writers[3].get() }, one_file( "Small", FEW ) },
        // A file with nothing to write still has to come out empty rather than upset the rest.
        { "empty", 0, { writers[4].get() }, one_file( "Empty", 0 ) },
        // The items.txt case: split over every thread, assembled in piece order afterwards.
        { "ordered", MANY, { writers[ORDERED_FILE].get() }, one_file( "Ordered", MANY ), true } };

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
      text.push_back( blocks( *writer ) );
  };

  std::vector<std::vector<std::string>> expected;
  std::vector<std::vector<std::string>> actual;
  run( false, expected );
  run( true, actual );

  UnitTest( [&]() { return expected[0].size() == MANY; }, true,
            "the test parts are big enough to be split" );

  auto sorted = []( std::vector<std::string> v )
  {
    std::sort( v.begin(), v.end() );
    return v;
  };
  for ( size_t f = 0; f < FILES; ++f )
  {
    if ( f == ORDERED_FILE )
      continue;
    UnitTest( [&]() { return actual[f].size(); }, expected[f].size(),
              fmt::format( "split file {} holds as many blocks as an unsplit one", f ) );
    UnitTest( [&]() { return sorted( actual[f] ) == sorted( expected[f] ); }, true,
              fmt::format( "and file {} holds exactly the same blocks", f ) );
  }

  // The ordered part is the strict case: same blocks, same sequence, no sorting allowed.
  UnitTest( [&]() { return actual[ORDERED_FILE] == expected[ORDERED_FILE]; }, true,
            "an ordered part comes out in the order one thread would have written it" );
}
}  // namespace Pol::Testing
