/** @file
 *
 * @par History
 * - 2009/09/03 MuadDib:     Changes for account related source file relocation
 *                           Changes for multi related source file relocation
 */

#include <stddef.h>

#include "bscript/berror.h"
#include "clib/clib_endian.h"
#include "clib/logfacility.h"
#include "clib/rawtypes.h"
#include "pol/globals/object_storage.h"
#include "pol/item/itemdesc.h"
#include "pol/ufunc.h"
#include "pol/multi/boat.h"
#include "pol/multi/house.h"
#include "pol/multi/multi.h"


namespace Pol::Multi
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
    ERROR_PRINTLN( "Tried to create multi {:#x} but no definition exists in itemdesc.cfg",
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


Bscript::BObjectImp* UMulti::scripted_create( const Items::ItemDesc& descriptor,
                                              const Core::Pos4d& pos, int flags )
{
  if ( descriptor.type == Items::ItemDesc::BOATDESC )
    return UBoat::scripted_create( descriptor, pos, flags );
  if ( descriptor.type == Items::ItemDesc::HOUSEDESC )
    return UHouse::scripted_create( descriptor, pos, flags );
  return new Bscript::BError( "Don't know what kind of multi to make" );
}
}  // namespace Pol::Multi
