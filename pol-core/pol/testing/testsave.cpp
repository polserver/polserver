/** @file
 *
 * @par History
 */


#include <pol_global_config.h>

#ifdef ENABLE_BENCHMARK
#include <benchmark/benchmark.h>
#include <cstdlib>
#include <string>

#include "clib/streamsaver.h"
#include "clib/strutil.h"
#include "pol/base/position.h"
#include "pol/containr.h"
#include "pol/globals/uvars.h"
#include "pol/item/item.h"
#include "pol/item/location.h"
#include "pol/proplist.h"
#include "pol/realms/realm.h"
#include "pol/savedata.h"
#include "pol/storage.h"

namespace Pol::Testing
{
namespace
{
/// The platform bit bucket. Formatting to it measures the cost of building the save text without
/// the filesystem in the way; the same benchmark pointed at a real file measures the i/o on top.
/// Comparing the two across platforms is what says whether a save is cpu- or disk-bound.
constexpr const char* null_device()
{
#ifdef _WIN32
  return "NUL";
#else
  return "/dev/null";
#endif
}

// The shape of a real shard's storage, measured on a 656 MB storage.txt: nearly all of it is one
// area of player bank boxes -- a few thousand root containers of a few hundred items each, and
// under three cprops per item. Scaled down by POL_BENCH_STORAGE_DIV (default 20) so a run is
// quick; set it to 1 to format the full ~650 MB.
constexpr unsigned ROOT_CONTAINERS = 5630;
constexpr unsigned ITEMS_PER_ROOT = 428;
constexpr unsigned CPROPS_PER_ITEM = 3;
/// Top-level items in the world, in the same proportion to storage as the shard that was
/// measured: 88.8MB of items.txt against 656.7MB of storage.txt.
constexpr unsigned WORLD_ITEMS = 325000;
constexpr u32 CONTAINER_OBJTYPE = 0xe75;
constexpr u32 ITEM_OBJTYPE = 0x3e3f;

unsigned scale_divisor()
{
  const char* env = std::getenv( "POL_BENCH_STORAGE_DIV" );
  const unsigned div = env ? static_cast<unsigned>( std::atoi( env ) ) : 20;
  return div ? div : 1;
}

void add_cprops( Core::UObject* obj, unsigned count, unsigned seed )
{
  for ( unsigned i = 0; i < count; ++i )
    obj->setprop( fmt::format( "scriptprop_number_{}", i ),
                  fmt::format( "i{}", 1000000 + ( seed + i ) * 7919 ) );
}

/// Fill the live storage once, and report how many items went in. Left in place afterwards: a
/// benchmark run exits before the world would be saved.
size_t populate_storage()
{
  static size_t items = 0;
  if ( items )
    return items;

  const unsigned div = scale_divisor();
  const unsigned roots = std::max( 1u, ROOT_CONTAINERS / div );
  const unsigned per_root = std::max( 1u, ITEMS_PER_ROOT / div );

  auto* area = Core::gamestate.storage.create_area( "benchmark_bank" );
  for ( unsigned r = 0; r < roots; ++r )
  {
    auto* root = Items::Item::create( CONTAINER_OBJTYPE );
    root->setname( fmt::format( "benchmark_box_{}", r ) );
    add_cprops( root, CPROPS_PER_ITEM, r );
    ++items;

    auto* container = static_cast<Core::UContainer*>( root );
    for ( unsigned i = 0; i < per_root; ++i )
    {
      auto* item = Items::Item::create( ITEM_OBJTYPE );
      item->setname( fmt::format( "benchmark_item_{}_{}", r, i ) );
      add_cprops( item, CPROPS_PER_ITEM, r + i );
      container->add( item );
      ++items;
    }
    area->insert_root_item( root );
  }
  return items;
}

/// Top-level items scattered over the first realm, which is what items.txt is made of. Placed in
/// a block near the origin: the zone walk visits every zone of the realm regardless, so where
/// they sit changes nothing about the work.
size_t populate_world_items()
{
  static size_t items = 0;
  if ( items )
    return items;
  if ( Core::gamestate.Realms.empty() )
    return 0;

  auto* realm = Core::gamestate.Realms.front();
  const unsigned wanted = std::max( 1u, WORLD_ITEMS / scale_divisor() );
  const u16 side = static_cast<u16>( std::sqrt( static_cast<double>( wanted ) ) ) + 1;
  for ( unsigned i = 0; i < wanted; ++i )
  {
    const Core::Pos4d p( Core::Pos3d( static_cast<u16>( i % side ), static_cast<u16>( i / side ),
                                      static_cast<s8>( 0 ) ),
                         realm );
    auto* item = Items::Item::create( ITEM_OBJTYPE );
    item->setposition( p );
    (void)Items::relocate( *item, Items::InWorld{} );
    add_cprops( item, CPROPS_PER_ITEM, i );
    ++items;
  }
  return items;
}
}  // namespace

/// One item's property block, the innermost loop of every save file.
static void BM_item_print( benchmark::State& state )
{
  auto* item = Items::Item::create( ITEM_OBJTYPE );
  item->setname( "benchmark_item" );
  add_cprops( item, CPROPS_PER_ITEM, 0 );
  Clib::StreamWriter sw( null_device() );
  while ( state.KeepRunning() )
    item->printOn( sw );
}
BENCHMARK( BM_item_print );

/// Just the cprops of one item -- the per-property allocation cost on its own.
static void BM_cprops_print( benchmark::State& state )
{
  Core::PropertyList props( Core::CPropProfiler::Type::ITEM );
  for ( unsigned i = 0; i < CPROPS_PER_ITEM; ++i )
    props.setprop( fmt::format( "scriptprop_number_{}", i ), fmt::format( "i{}", 1000000 + i ) );
  Clib::StreamWriter sw( null_device() );
  while ( state.KeepRunning() )
    props.printProperties( sw );
}
BENCHMARK( BM_cprops_print );

/// The whole storage file, formatted but not written: the save's long pole, cpu only.
static void BM_storage_print_null( benchmark::State& state )
{
  const size_t items = populate_storage();
  size_t bytes = 0;
  while ( state.KeepRunning() )
  {
    Clib::StreamWriter sw( null_device() );
    Core::gamestate.storage.print( sw );
    bytes = sw.bytes_written();
    sw.flush_close();
  }
  state.SetItemsProcessed( state.iterations() * items );
  state.SetBytesProcessed( state.iterations() * bytes );
}
BENCHMARK( BM_storage_print_null )->Unit( benchmark::kMillisecond );

/// The same storage file written for real, so the delta against the null-device run is the i/o.
static void BM_storage_print_file( benchmark::State& state )
{
  const size_t items = populate_storage();
  size_t bytes = 0;
  while ( state.KeepRunning() )
  {
    Clib::StreamWriter sw( "benchmark_storage.ndt" );
    Core::gamestate.storage.print( sw );
    bytes = sw.bytes_written();
    sw.flush_close();
  }
  state.SetItemsProcessed( state.iterations() * items );
  state.SetBytesProcessed( state.iterations() * bytes );
}
BENCHMARK( BM_storage_print_file )->Unit( benchmark::kMillisecond );

/// The whole thing: what a shard is actually frozen for. Reports the critical part, which is the
/// stretch the world is stopped, rather than the wall time of the call.
static void BM_world_save( benchmark::State& state )
{
  const size_t stored = populate_storage();
  const size_t in_world = populate_world_items();
  s64 critical = 0;
  size_t bytes = 0;
  while ( state.KeepRunning() )
  {
    Core::SaveResult result;
    Core::write_data( {}, &result );
    Core::SaveContext::ready();  // let the commit finish before timing the next one
    critical = result.critical_ms;
    bytes = 0;
    for ( const auto& file : result.files )
      bytes += file.bytes;
  }
  state.counters["critical_ms"] = static_cast<double>( critical );
  state.counters["MB"] = static_cast<double>( bytes ) / ( 1024 * 1024 );
  state.SetItemsProcessed( state.iterations() * ( stored + in_world ) );
}
BENCHMARK( BM_world_save )->Unit( benchmark::kMillisecond )->Iterations( 5 );
}  // namespace Pol::Testing
#endif
