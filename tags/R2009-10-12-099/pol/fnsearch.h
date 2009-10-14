/*
History
=======


Notes
=======

*/

#ifndef FNSEARCH_H
#define FNSEARCH_H

#include "utype.h"

class Item;
class Character;
class UObject;
class UMulti;


const unsigned SYSFIND_SEARCH_OFFLINE_MOBILES = 1;
/*const unsigned SYSFIND_SEARCH_STORAGE_AREAS = 2;*/ //02-07-2009 Turley obsolete
const unsigned SYSFIND_SKIP_WORLD = 4;

UMulti* system_find_multi( USERIAL serial );
UObject* system_find_object( u32 serial );
//02-07-2009 Turley removed flag param till someone finds something usefull
Item* system_find_item( USERIAL serial/*, int sysfind_flags = 0 */);
Character* system_find_mobile( USERIAL serial/*, int sysfind_flags = 0 */);

Character* find_character( USERIAL serial );
UObject* find_toplevel_object( USERIAL serial );
Item* find_toplevel_item( USERIAL serial );
Character *find_character( USERIAL serial );


#endif
