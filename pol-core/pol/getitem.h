/** @file
 *
 * @par History
 */


#ifndef __GETITEM_H
#define __GETITEM_H
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
void undo_get_item( Mobile::Character* chr, Items::Item* item );
}
}
#endif
