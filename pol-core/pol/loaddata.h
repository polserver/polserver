/** @file
 *
 * @par History
 */


#ifndef LOADDATA_H
#define LOADDATA_H

#include <vector>

#include "clib/rawtypes.h"
#include "plib/poltype.h"

namespace Pol
{
namespace Mobile
{
class Character;
}
namespace Items
{
class Item;
}
namespace Core
{
class UContainer;

/**
 * The containers the loader is currently inside, innermost last.
 *
 * A save writes a container and then its contents, depth first, so the container a line names is
 * almost always the one we are standing in; asking here first spares it an object hash lookup.
 *
 * A miss is never wrong, only unhelpful: it falls through to system_find_item(), and every loaded
 * item is in the object hash from the moment Item::create() makes it.
 */
class LoadedContainerStack
{
public:
  /// The container with this serial, if it is one of the ones we are inside; nullptr otherwise.
  /// Leaves the stack standing at that container, having dropped whatever we had descended into.
  Items::Item* find( pol_serial_t serial );
  /// Descend into an item, if it is a container and still alive.
  void push( Items::Item* item );
  void clear();

private:
  std::vector<UContainer*> conts_;
};
extern LoadedContainerStack loaded_container_stack;

void slurp( const char* filename, const char* tags, int sysfind_flags = 0 );

void defer_item_insertion( Items::Item* item, pol_serial_t container_serial, u8 saved_layer,
                           u8 saved_slot );
void insert_deferred_items();
void equip_loaded_item( Mobile::Character* chr, Items::Item* item );
void add_loaded_item( Items::Item* cont_item, Items::Item* item, u8 saved_layer, u8 saved_slot );
}  // namespace Core
}  // namespace Pol
#endif
