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


EquipDesc::EquipDesc( u16 objtype, ConfigElem& elem, Type type, const Package* pkg ) :
    ItemDesc( objtype, elem, type, pkg ),
    maxhp( elem.remove_ushort( "MAXHP" ))
{
}

void EquipDesc::PopulateStruct( BStruct* descriptor ) const
{
    base::PopulateStruct( descriptor );
	descriptor->addMember( "MaxHP", new BLong(maxhp) );
}
/*
EquipDesc::EquipDesc()
{
}
*/
