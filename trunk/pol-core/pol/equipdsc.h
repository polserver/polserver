/*
History
=======


Notes
=======

*/

#ifndef EQUIPDSC_H
#define EQUIPDSC_H

#include "item/itemdesc.h"
namespace Pol {
  namespace Clib {
	class ConfigElem;
  }
  namespace Plib {
	class Package;
  }
  namespace Core {

	class EquipDesc : public Items::ItemDesc
	{
	public:
	  typedef Items::ItemDesc base;
	  EquipDesc( u32 objtype, Clib::ConfigElem& elem, Type type, const Plib::Package* pkg );
	  EquipDesc(); // for dummy template
	  virtual void PopulateStruct( Bscript::BStruct* descriptor ) const;
	};
  }
}
#endif
