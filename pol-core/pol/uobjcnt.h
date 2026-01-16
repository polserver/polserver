/** @file
 *
 * @par History
 */

#ifndef __UOBJCNT_H
#define __UOBJCNT_H


namespace Pol::Core
{
struct UObjCount
{
  UObjCount();
  int unreaped_orphans;
  int uobject_count;
  int uitem_count;
  int ucharacter_count;
  int npc_count;
  int umulti_count;
  int uobj_count_echrref;
};
}  // namespace Pol::Core

#endif
