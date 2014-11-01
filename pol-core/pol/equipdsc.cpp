/*
History
=======


Notes
=======

*/
#include "equipdsc.h"

#include "../clib/cfgelem.h"
#include "../bscript/bstruct.h"


namespace Pol {
  namespace Core {
	EquipDesc::EquipDesc( u32 objtype, Clib::ConfigElem& elem, Type type, const Plib::Package* pkg ) :
	  ItemDesc( objtype, elem, type, pkg )
	{}

	void EquipDesc::PopulateStruct( Bscript::BStruct* descriptor ) const
	{
	  base::PopulateStruct( descriptor );
	}
    size_t EquipDesc::estimatedSize() const
    {
      return base::estimatedSize();
    }
	/*
	EquipDesc::EquipDesc()
	{
	}
	*/
  }
}
