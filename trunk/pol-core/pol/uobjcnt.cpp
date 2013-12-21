/*
History
=======


Notes
=======

*/

#include "../clib/stl_inc.h"
#ifdef _MSC_VER
#pragma warning( disable: 4786 )
#endif

#include "uobjcnt.h"
namespace Pol {
  namespace Core {
	int unreaped_orphans;
	int uobject_count;
	int uitem_count;
	int umulti_count;
	int ucharacter_count;
	int npc_count;
	int uobj_count_echrref;

	ItemSet existing_items;
  }
}