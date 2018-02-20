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
#include "../multi/boat.h"
#include "../multi/multi.h"
#include "../realms/realm.h"
#include "../uconst.h"
#include "../uoexhelp.h"

namespace Pol
{
namespace Bscript
{
using namespace Module;
template <>
TmplExecutorModule<Module::UBoatExecutorModule>::FunctionTable
    TmplExecutorModule<Module::UBoatExecutorModule>::function_table = {
        {"MoveBoat", &UBoatExecutorModule::mf_MoveBoat},
        {"MoveBoatRelative", &UBoatExecutorModule::mf_MoveBoatRelative},
        {"MoveBoatXY", &UBoatExecutorModule::mf_MoveBoatXY},
        {"TurnBoat", &UBoatExecutorModule::mf_TurnBoat},
        {"RegisterItemWithBoat", &UBoatExecutorModule::mf_RegisterItemWithBoat},
        {"BoatFromItem", &UBoatExecutorModule::mf_BoatFromItem},
        {"SystemFindBoatBySerial", &UBoatExecutorModule::mf_SystemFindBoatBySerial}};
}
namespace Module
{
UBoatExecutorModule::UBoatExecutorModule( Bscript::Executor& exec )
    : Bscript::TmplExecutorModule<UBoatExecutorModule>( "boat", exec )
{
}

Bscript::BObjectImp* UBoatExecutorModule::mf_MoveBoat()
{
  Multi::UBoat* boat = NULL;
  int direction, speed;
  if ( getUBoatParam( exec, 0, boat ) && getParam( 1, direction, 0, 7 ) &&
       getParam( 2, speed, 1, 4 ) )
  {
    Core::UFACING move_dir = static_cast<Core::UFACING>( direction & 7 );
    boat->move( move_dir, static_cast<u8>( speed ), false );
    return new Bscript::BLong( 1 );
  }
  return NULL;
}

Bscript::BObjectImp* UBoatExecutorModule::mf_MoveBoatXY()
{
  Multi::UBoat* boat = NULL;
  unsigned short x, y;
  if ( getUBoatParam( exec, 0, boat ) &&
       getParam( 1, x, 0, static_cast<u16>( boat->realm->width() ) ) &&
       getParam( 2, y, 0, static_cast<u16>( boat->realm->height() ) ) )
  {
    return new Bscript::BLong( boat->move_xy( x, y, 0, boat->realm ) );
  }
  else
  {
    return new Bscript::BError( "Invalid Parameter type" );
  }
}

Bscript::BObjectImp* UBoatExecutorModule::mf_TurnBoat()
{
  Multi::UBoat* boat = NULL;
  int relative_dir;
  if ( getUBoatParam( exec, 0, boat ) && getParam( 1, relative_dir ) )
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
  Multi::UBoat* boat = NULL;
  int direction, speed;
  if ( getUBoatParam( exec, 0, boat ) && getParam( 1, direction, 0, 7 ) &&
       getParam( 2, speed, 1, 4 ) )
  {
    Core::UFACING move_dir = static_cast<Core::UFACING>( direction & 7 );
    boat->move( move_dir, static_cast<u8>( speed ), true );
    return new Bscript::BLong( 1 );
  }
  return NULL;
}

Bscript::BObjectImp* UBoatExecutorModule::mf_RegisterItemWithBoat()
{
  Multi::UBoat* boat = NULL;
  Core::UObject* obj = NULL;
  if ( getUBoatParam( exec, 0, boat ) && getUObjectParam( exec, 1, obj ) )
  {
    boat->register_object( obj );
    return new Bscript::BLong( 1 );
  }
  return NULL;
}

Bscript::BObjectImp* UBoatExecutorModule::mf_SystemFindBoatBySerial()
{
  Multi::UBoat* boat = NULL;
  if ( getUBoatParam( exec, 0, boat ) )
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
  Items::Item* item = NULL;
  if ( getItemParam( exec, 0, item ) )
  {
    if ( item->ismulti() )
    {
      Multi::UMulti* multi = static_cast<Multi::UMulti*>( item );
      Multi::UBoat* boat = multi->as_boat();
      if ( boat != NULL )
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
}
}
