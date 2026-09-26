#include <stdexcept>
#include <string>

#include "clib/rawtypes.h"
#include "plib/systemstate.h"
#include "pol/containr.h"
#include "pol/globals/object_storage.h"
#include "pol/globals/uvars.h"
#include "pol/item/item.h"
#include "pol/item/location.h"
#include "pol/loaddata.h"
#include "pol/realms/realm.h"
#include "pol/reftypes.h"
#include "pol/testing/testenv.h"

namespace Pol::Testing
{
namespace
{
constexpr u32 CONTAINER_OBJTYPE = 0xe75;  // a backpack, per the test shard's itemdesc.cfg
constexpr u32 ITEM_OBJTYPE = 0x0eed;      // gold
// serials nothing in the test world has: the first in the item range, the second a character's
constexpr u32 MISSING_CONTAINER = 0x7FFFFFF0;
constexpr u32 MISSING_CHARACTER = 0x3FFFFFF0;

Items::Item* item_in_world( u32 objtype, const Core::Pos4d& p )
{
  auto* item = Items::Item::create( objtype );
  item->setposition( p );
  (void)Items::relocate( *item, Items::InWorld{} );
  return item;
}

// Runs the deferred inserts and returns what they threw, or "" when they did not.
std::string insert_deferred()
{
  try
  {
    Core::insert_deferred_items();
  }
  catch ( const std::exception& ex )
  {
    Core::objStorageManager.deferred_insertions.clear();
    return ex.what();
  }
  return "";
}
}  // namespace

// The world loads an item before the container or character that holds it when the save lists
// them in that order, and inserts it once everything is loaded. A holder that never turns up is a
// broken save: IgnoreLoadErrors decides whether the item is dropped or the load stops.
void deferred_insertion_test()
{
  auto* realm = Core::gamestate.Realms[0];
  const Core::Pos4d spot( realm->area().nw() + Core::Vec2d( 44, 44 ), 0, realm );
  auto& config = Plib::systemstate.config;
  const bool previous_ignore = config.ignore_load_errors;

  {
    auto* cont = static_cast<Core::UContainer*>( item_in_world( CONTAINER_OBJTYPE, spot ) );
    Core::ItemRef item( Items::Item::create( ITEM_OBJTYPE ) );
    Core::defer_item_insertion( item.get(), cont->serial, 0, 0 );
    const std::string thrown = insert_deferred();
    UnitTest( [&] { return thrown.empty() && item->container() == cont; }, true,
              "a deferred item goes into its container once the load is done" );
    UnitTest( [] { return Core::objStorageManager.deferred_insertions.empty(); }, true,
              "and nothing is left deferred" );
    item->destroy();
    cont->destroy();
  }

  config.ignore_load_errors = true;
  {
    Core::ItemRef item( Items::Item::create( ITEM_OBJTYPE ) );
    Core::defer_item_insertion( item.get(), MISSING_CONTAINER, 0, 0 );
    const std::string thrown = insert_deferred();
    UnitTest( [&] { return thrown.empty() && item->orphan(); }, true,
              "with IgnoreLoadErrors an item whose container is missing is destroyed" );
  }
  {
    Core::ItemRef item( Items::Item::create( ITEM_OBJTYPE ) );
    Core::defer_item_insertion( item.get(), MISSING_CHARACTER, 0, 0 );
    const std::string thrown = insert_deferred();
    UnitTest( [&] { return thrown.empty() && item->orphan(); }, true,
              "with IgnoreLoadErrors an item whose character is missing is destroyed" );
  }

  config.ignore_load_errors = false;
  {
    Core::ItemRef item( Items::Item::create( ITEM_OBJTYPE ) );
    Core::defer_item_insertion( item.get(), MISSING_CONTAINER, 0, 0 );
    const std::string thrown = insert_deferred();
    UnitTest( [&] { return thrown; }, std::string( "Data file integrity error" ),
              "without IgnoreLoadErrors a missing container stops the load" );
    item->destroy();
  }
  {
    Core::ItemRef item( Items::Item::create( ITEM_OBJTYPE ) );
    Core::defer_item_insertion( item.get(), MISSING_CHARACTER, 0, 0 );
    const std::string thrown = insert_deferred();
    UnitTest( [&] { return thrown; }, std::string( "Data file integrity error" ),
              "without IgnoreLoadErrors a missing character stops the load" );
    item->destroy();
  }

  config.ignore_load_errors = previous_ignore;
}
}  // namespace Pol::Testing
