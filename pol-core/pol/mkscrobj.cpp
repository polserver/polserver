/** @file
 *
 * @par History
 */


#include "mkscrobj.h"
#include "uoscrobj.h"


namespace Pol::Core
{
Bscript::BObjectImp* make_mobileref( Mobile::Character* chr )
{
  return new Module::ECharacterRefObjImp( chr );
}

Bscript::BObjectImp* make_itemref( Items::Item* item )
{
  return new Module::EItemRefObjImp( item );
}

Bscript::BObjectImp* make_boatref( Multi::UBoat* boat )
{
  return new Module::EUBoatRefObjImp( boat );
}
}  // namespace Pol::Core
