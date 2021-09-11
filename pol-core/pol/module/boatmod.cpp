/** @file
 *
 * @par History
 * - 2006/09/26 Shinigami: GCC 3.4.x fix - added "template<>" to TmplExecutorModule
 * - 2009/09/03 MuadDib:   Changes for account related source file relocation
 *                         Changes for multi related source file relocation
 */


#include "boatmod.h"
#include <stddef.h>

#include "../../bscript/berror.h"
#include "../../clib/rawtypes.h"
#include "../../plib/uconst.h"
#include "../multi/boat.h"
#include "../multi/multi.h"
#include "../realms/realm.h"

#include <module_defs/boat.h>

namespace Pol
{
namespace Module
{
UBoatExecutorModule::UBoatExecutorModule( Bscript::Executor& exec )
    : Bscript::TmplExecutorModule<UBoatExecutorModule, Core::PolModule>( exec )
{
}

Bscript::BObjectImp* UBoatExecutorModule::mf_MoveBoat()
{
  Multi::UBoat* boat = nullptr;
  int direction, speed;
  if ( getUBoatParam( 0, boat ) && getParam( 1, direction, 0, 7 ) && getParam( 2, speed, 1, 4 ) )
  {
    Core::UFACING move_dir = static_cast<Core::UFACING>( direction & 7 );
    boat->move( move_dir, static_cast<u8>( speed ), false );
    return new Bscript::BLong( 1 );
  }
  return nullptr;
}

Bscript::BObjectImp* UBoatExecutorModule::mf_MoveBoatXY()
{
  Multi::UBoat* boat = nullptr;
  unsigned short x, y;
  if ( getUBoatParam( 0, boat ) &&
       getParam( 1, x, 0, static_cast<u16>( boat->realm()->width() ) ) &&
       getParam( 2, y, 0, static_cast<u16>( boat->realm()->height() ) ) )
  {
    return new Bscript::BLong( boat->move_xy( x, y, 0, boat->realm() ) );
  }
  else
  {
    return new Bscript::BError( "Invalid Parameter type" );
  }
}

Bscript::BObjectImp* UBoatExecutorModule::mf_TurnBoat()
{
  Multi::UBoat* boat = nullptr;
  int relative_dir;
  if ( getUBoatParam( 0, boat ) && getParam( 1, relative_dir ) )
  {
    relative_dir &= 3;
    return new Bscript::BLong(
        boat->turn( static_cast<Multi::UBoat::RELATIVE_DIR>( relative_dir ) ) );
  }
  else
  {
    return new Bscript::BError( "Invalid Parameter type" );
  }
}

Bscript::BObjectImp* UBoatExecutorModule::mf_MoveBoatRelative()
{
  Multi::UBoat* boat = nullptr;
  int direction, speed;
  if ( getUBoatParam( 0, boat ) && getParam( 1, direction, 0, 7 ) && getParam( 2, speed, 1, 4 ) )
  {
    Core::UFACING move_dir = static_cast<Core::UFACING>( direction & 7 );
    boat->move( move_dir, static_cast<u8>( speed ), true );
    return new Bscript::BLong( 1 );
  }
  return nullptr;
}

Bscript::BObjectImp* UBoatExecutorModule::mf_RegisterItemWithBoat()
{
  Multi::UBoat* boat = nullptr;
  Core::UObject* obj = nullptr;
  if ( getUBoatParam( 0, boat ) && getUObjectParam( 1, obj ) )
  {
    boat->register_object( obj );
    return new Bscript::BLong( 1 );
  }
  return nullptr;
}

Bscript::BObjectImp* UBoatExecutorModule::mf_SystemFindBoatBySerial()
{
  Multi::UBoat* boat = nullptr;
  if ( getUBoatParam( 0, boat ) )
  {
    return boat->make_ref();
  }
  else
  {
    return new Bscript::BError( "Boat not found." );
  }
}

Bscript::BObjectImp* UBoatExecutorModule::mf_BoatFromItem()
{
  Items::Item* item = nullptr;
  if ( getItemParam( 0, item ) )
  {
    if ( item->ismulti() )
    {
      Multi::UMulti* multi = static_cast<Multi::UMulti*>( item );
      Multi::UBoat* boat = multi->as_boat();
      if ( boat != nullptr )
        return boat->make_ref();
      else
        return new Bscript::BError( "Multi wasn't a boat" );
    }
    else
    {
      return new Bscript::BError( "Item wasn't a multi" );
    }
  }
  else
  {
    return new Bscript::BError( "Invalid parameter type." );
  }
}
}  // namespace Module
}  // namespace Pol
