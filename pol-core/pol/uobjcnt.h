/*
History
=======


Notes
=======

*/
#ifndef __UOBJCNT_H
#define __UOBJCNT_H
namespace Pol {
  namespace Items {
	class Item;
  }
  namespace Core {
	extern int unreaped_orphans;
	extern int uobject_count;
	extern int uitem_count;
	extern int ucharacter_count;
	extern int npc_count;
	extern int umulti_count;
	extern int uobj_count_echrref;

	typedef std::set<Items::Item*> ItemSet;
	extern ItemSet existing_items;
  }
}
#endif
