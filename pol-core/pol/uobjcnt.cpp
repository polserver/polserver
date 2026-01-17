/** @file
 *
 * @par History
 */


#include "uobjcnt.h"


namespace Pol::Core
{
// This comment is needed for correct indentation to be detected
UObjCount::UObjCount()
    : unreaped_orphans( 0 ),
      uobject_count( 0 ),
      uitem_count( 0 ),
      ucharacter_count( 0 ),
      npc_count( 0 ),
      umulti_count( 0 ),
      uobj_count_echrref( 0 )
{
}
}  // namespace Pol::Core
