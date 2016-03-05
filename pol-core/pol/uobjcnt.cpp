/** @file
 *
 * @par History
 */



#include "uobjcnt.h"

namespace Pol {
  namespace Core {
	UObjCount::UObjCount() :
	 unreaped_orphans(0),
	 uobject_count(0),
	 uitem_count(0),
	 ucharacter_count(0),
	 npc_count(0),
	 umulti_count(0),
	 uobj_count_echrref(0)
	 {}
  }
}