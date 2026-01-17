/** @file
 *
 * @par History
 */

#include "equipdsc.h"


namespace Pol::Core
{
EquipDesc::EquipDesc( u32 objtype, Clib::ConfigElem& elem, Type type, const Plib::Package* pkg )
    : ItemDesc( objtype, elem, type, pkg ), is_intrinsic( false ), is_pc_intrinsic( false )
{
}

void EquipDesc::PopulateStruct( Bscript::BStruct* descriptor ) const
{
  base::PopulateStruct( descriptor );
}
size_t EquipDesc::estimatedSize() const
{
  return base::estimatedSize() + sizeof( bool ) /*is_intrinsic*/
         + sizeof( bool );                      /*is_pc_intrinsic*/
}
/*
EquipDesc::EquipDesc()
{
}
*/
}  // namespace Pol::Core
