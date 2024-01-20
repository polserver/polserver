/** @file
 *
 * @par History
 * - 2009/09/03 MuadDib:     Changes for account related source file relocation
 *                           Changes for multi related source file relocation
 */

#include <stddef.h>

#include "../../bscript/berror.h"
#include "../../clib/clib_endian.h"
#include "../../clib/logfacility.h"
#include "../../clib/rawtypes.h"
#include "../globals/object_storage.h"
#include "../item/itemdesc.h"
#include "../ufunc.h"
#include "boat.h"
#include "house.h"
#include "multi.h"


namespace Pol
{
namespace Multi
{
bool isboat( u32 objtype )
{
  return Items::find_itemdesc( objtype ).type == Items::ItemDesc::BOATDESC;
}

bool ishouse( u32 objtype )
{
  return Items::find_itemdesc( objtype ).type == Items::ItemDesc::HOUSEDESC;
}

// scripted_create duplicates some of this work
// Dave changed 3/8/3 to use objecthash
UMulti* UMulti::create( const Items::ItemDesc& descriptor, u32 serial )
{
  UMulti* multi = nullptr;

  if ( descriptor.type == Items::ItemDesc::BOATDESC )
  {
    multi = new UBoat( descriptor );
  }
  else if ( descriptor.type == Items::ItemDesc::HOUSEDESC )
  {
    multi = new UHouse( descriptor );
  }
  else
  {
    ERROR_PRINTLN( "Tried to create multi {:#X} but no definition exists in itemdesc.cfg",
                   descriptor.objtype );
    return nullptr;
  }

  if ( serial )
    multi->serial = Core::UseItemSerialNumber( serial );
  else
    multi->serial = Core::GetNewItemSerialNumber();

  multi->serial_ext = ctBEu32( multi->serial );

  ////HASH
  Core::objStorageManager.objecthash.Insert( multi );
  ////

  return multi;
}


Bscript::BObjectImp* UMulti::scripted_create( const Items::ItemDesc& descriptor, u16 x, u16 y, s8 z,
                                              Realms::Realm* realm, int flags )
{
  if ( descriptor.type == Items::ItemDesc::BOATDESC )
    return UBoat::scripted_create( descriptor, x, y, z, realm, flags );
  else if ( descriptor.type == Items::ItemDesc::HOUSEDESC )
    return UHouse::scripted_create( descriptor, x, y, z, realm, flags );
  else
    return new Bscript::BError( "Don't know what kind of multi to make" );
}
}  // namespace Multi
}  // namespace Pol
