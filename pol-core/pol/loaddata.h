/** @file
 *
 * @par History
 */


#ifndef LOADDATA_H
#define LOADDATA_H

#include "../plib/poltype.h"

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
void slurp( const char* filename, const char* tags, int sysfind_flags = 0 );

void defer_item_insertion( Items::Item* item, pol_serial_t container_serial );
void insert_deferred_items();
void equip_loaded_item( Mobile::Character* chr, Items::Item* item );
void add_loaded_item( Items::Item* cont_item, Items::Item* item );
}  // namespace Core
}  // namespace Pol
#endif
