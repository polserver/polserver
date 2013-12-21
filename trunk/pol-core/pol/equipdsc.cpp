/*
History
=======


Notes
=======

*/

#include "../clib/stl_inc.h"
#include "../clib/cfgelem.h"

#include "../bscript/bstruct.h"

#include "equipdsc.h"

namespace Pol {
  namespace Core {
	EquipDesc::EquipDesc( u32 objtype, Clib::ConfigElem& elem, Type type, const Plib::Package* pkg ) :
	  ItemDesc( objtype, elem, type, pkg )
	{}

	void EquipDesc::PopulateStruct( Bscript::BStruct* descriptor ) const
	{
	  base::PopulateStruct( descriptor );
	}
	/*
	EquipDesc::EquipDesc()
	{
	}
	*/
  }
}
