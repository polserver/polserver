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


EquipDesc::EquipDesc( u32 objtype, ConfigElem& elem, Type type, const Package* pkg ) :
    ItemDesc( objtype, elem, type, pkg )   
{
}

void EquipDesc::PopulateStruct( BStruct* descriptor ) const
{
    base::PopulateStruct( descriptor );
}
/*
EquipDesc::EquipDesc()
{
}
*/
