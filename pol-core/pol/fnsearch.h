/** @file
 *
 * @par History
 */


#ifndef FNSEARCH_H
#define FNSEARCH_H

#include "../clib/rawtypes.h"
#include "utype.h"

namespace Pol
{
namespace Items
{
class Item;
}
namespace Mobile
{
class Character;
}
namespace Multi
{
class UMulti;
}
namespace Core
{
class UObject;

const unsigned SYSFIND_SEARCH_OFFLINE_MOBILES = 1;
/*const unsigned SYSFIND_SEARCH_STORAGE_AREAS = 2;*/  // 02-07-2009 Turley obsolete
const unsigned SYSFIND_SKIP_WORLD = 4;

Multi::UMulti* system_find_multi( USERIAL serial );
UObject* system_find_object( u32 serial );
// 02-07-2009 Turley removed flag param till someone finds something usefull
Items::Item* system_find_item( USERIAL serial /*, int sysfind_flags = 0 */ );
Mobile::Character* system_find_mobile( USERIAL serial /*, int sysfind_flags = 0 */ );

Mobile::Character* find_character( USERIAL serial );
UObject* find_toplevel_object( USERIAL serial );
Items::Item* find_toplevel_item( USERIAL serial );
}
}

#endif
