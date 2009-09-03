/*
History
=======
2006/09/15 austin:    added ENUMERATE_ROOT_ONLY flag
2005/04/04 Shinigami: added constant for use with can_delete_character( chr, delete_by )
2005/04/28 Shinigami: added constant for use with EnumerateItemsInContainer( container, flags := 0 )
2005/04/31 Shinigami: added constant for use with LogToFile( filename, line, flags := 0 )
2005/06/06 Shinigami: added constant for use with ListStatics*
2005/07/04 Shinigami: added constant for use with List[Statics/Items]*, List* constants renamed
2005/09/03 Shinigami: added constant for use with FindPath
2006/05/07 Shinigami: added constant for use with Send*Window
2008/10/29 Luth:      added constant for use with PrintTextAbove

Notes
=======

*/


#ifndef CORE_H
#define CORE_H

class Character;
class Item;
class Items;
class Realm;

enum Priority
{
    ToolTipPriorityNone=0,
    ToolTipPrioritySystem=1,
    ToolTipPriorityScript=2,
    ToolTipPriorityShutdown=3
};
void CoreSetSysTrayToolTip( const string& text, Priority pri );

bool move_character_to( Character* chr, 
                       unsigned short x,
                        unsigned short y,
                        int z,
                        long flags,
						Realm* oldrealm );

const long MOVEITEM_IGNOREMOVABLE	= 0x20000000L;
const long MOVEITEM_FORCELOCATION	= 0x40000000L;
const long RESURRECT_FORCELOCATION	= 0x01;

const long INSERT_ADD_ITEM          = 0x01;
const long INSERT_INCREASE_STACK    = 0x02;

const long DELETE_BY_PLAYER         = 0x00;
const long DELETE_BY_SCRIPT         = 0x01;

const long ENUMERATE_IGNORE_LOCKED	= 0x01;
const long ENUMERATE_ROOT_ONLY		= 0x02;

const long ITEMS_IGNORE_STATICS     = 0x01;
const long ITEMS_IGNORE_MULTIS      = 0x02;

const long LIST_IGNORE_Z            = 0x40000000L;

const long FP_IGNORE_MOBILES        = 0x01;
const long FP_IGNORE_DOORS          = 0x02;

const long VENDOR_SEND_AOS_TOOLTIP  = 0x01;

const long SAVE_FULL                = 0x00;
const long SAVE_INCREMENTAL         = 0x01;

const long LOG_DATETIME             = 0x01;

const long FINDSUBSTANCE_IGNORE_LOCKED = 0x01;
const long FINDSUBSTANCE_ROOT_ONLY     = 0x02;
const long FINDSUBSTANCE_FIND_ALL      = 0x04;

const long JOURNAL_PRINT_NAME			= 0x00;
const long JOURNAL_PRINT_YOU_SEE		= 0x01;

Item* find_walkon_item( Items& ivec, int z );
void atomic_cout( const string& msg );
void restart_all_clients();

#endif
