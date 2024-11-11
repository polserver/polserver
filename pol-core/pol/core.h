/** @file
 *
 * @par History
 * - 2006/09/15 austin:    added ENUMERATE_ROOT_ONLY flag
 * - 2005/04/04 Shinigami: added constant for use with can_delete_character( chr, delete_by )
 * - 2005/04/28 Shinigami: added constant for use with EnumerateItemsInContainer( container, flags
 * := 0 )
 * - 2005/04/31 Shinigami: added constant for use with LogToFile( filename, line, flags := 0 )
 * - 2005/06/06 Shinigami: added constant for use with ListStatics*
 * - 2005/07/04 Shinigami: added constant for use with List[Statics/Items]*, List* constants renamed
 * - 2005/09/03 Shinigami: added constant for use with FindPath
 * - 2006/05/07 Shinigami: added constant for use with Send*Window
 * - 2008/10/29 Luth:      added constant for use with PrintTextAbove
 */


#ifndef CORE_H
#define CORE_H

#include <string>

#include "base/position.h"

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
namespace Realms
{
class Realm;
}
namespace Core
{
void apply_polcfg( bool initial );

class ItemsVector;
enum Priority
{
  ToolTipPriorityNone = 0,
  ToolTipPrioritySystem = 1,
  ToolTipPriorityScript = 2,
  ToolTipPriorityShutdown = 3
};
void CoreSetSysTrayToolTip( const std::string& text, Priority pri );

bool move_character_to( Mobile::Character* chr, Pos4d newpos, int flags );

const int MOVEITEM_IGNOREMOVABLE = 0x20000000L;
const int MOVEITEM_FORCELOCATION = 0x40000000L;
const int RESURRECT_FORCELOCATION = 0x01;

const int INSERT_ADD_ITEM = 0x01;
const int INSERT_INCREASE_STACK = 0x02;

const int DELETE_BY_PLAYER = 0x00;
const int DELETE_BY_SCRIPT = 0x01;

const int ENUMERATE_IGNORE_LOCKED = 0x01;
const int ENUMERATE_ROOT_ONLY = 0x02;

const int ITEMS_IGNORE_STATICS = 0x01;
const int ITEMS_IGNORE_MULTIS = 0x02;

const int LIST_IGNORE_Z = 0x40000000L;

const int FP_IGNORE_MOBILES = 0x01;
const int FP_IGNORE_DOORS = 0x02;

const int VENDOR_SEND_AOS_TOOLTIP = 0x01;
const int VENDOR_BUYABLE_CONTAINER_FILTER = 0x02;

const int SENDDIALOGMENU_FORCE_OLD = 0x01;

const int LOG_DATETIME = 0x01;

const int FINDOBJTYPE_IGNORE_LOCKED = 0x01;
const int FINDOBJTYPE_ROOT_ONLY = 0x02;

const int FINDSUBSTANCE_IGNORE_LOCKED = 0x01;
const int FINDSUBSTANCE_ROOT_ONLY = 0x02;
const int FINDSUBSTANCE_FIND_ALL = 0x04;

const int JOURNAL_PRINT_NAME = 0x00;
const int JOURNAL_PRINT_YOU_SEE = 0x01;

Items::Item* find_walkon_item( const ItemsVector& ivec, short z );
void restart_all_clients();
}  // namespace Core
}  // namespace Pol
#endif
