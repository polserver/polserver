/** @file
 *
 * @par History
 * - 2006/10/07 Shinigami: GCC 3.4.x fix - added "template<>" to TmplExecutorModule
 */


#include "storagemod.h"
#include <stddef.h>

#include "../../bscript/berror.h"
#include "../../bscript/impstr.h"
#include "../../clib/clib.h"
#include "../globals/uvars.h"
#include "../item/item.h"
#include "../storage.h"
#include "../uoscrobj.h"
#include "realms/realms.h"

#include <module_defs/storage.h>

namespace Pol
{
namespace Core
{
Bscript::BObjectImp* CreateStorageAreasImp();
}
namespace Module
{
using namespace Bscript;

StorageExecutorModule::StorageExecutorModule( Bscript::Executor& exec )
    : Bscript::TmplExecutorModule<StorageExecutorModule, Core::PolModule>( exec )
{
}

BObjectImp* StorageExecutorModule::mf_StorageAreas()
{
  return Core::CreateStorageAreasImp();
}


BObjectImp* StorageExecutorModule::mf_FindStorageArea()
{
  const String* str = getStringParam( 0 );
  if ( str != nullptr )
  {
    Core::StorageArea* area = Core::gamestate.storage.find_area( str->value() );

    if ( area )
      return new Core::StorageAreaImp( area );
  }
  return new BLong( 0 );  // non-string passed, or not found.
}

BObjectImp* StorageExecutorModule::mf_CreateStorageArea()
{
  auto name = static_cast<String*>( getParamImp( 0, BObjectImp::OTString ) );
  if ( name )
  {
    Core::StorageArea* area = Core::gamestate.storage.create_area( name->value() );
    if ( area )
      return new Core::StorageAreaImp( area );
  }
  return new BLong( 0 );  // non-string passed, or not found.
}

BObjectImp* StorageExecutorModule::mf_FindRootItemInStorageArea()
{
  auto area_imp =
      static_cast<Core::StorageAreaImp*>( getParamImp( 0, Bscript::BObjectImp::OTStorageArea ) );
  const String* name = getStringParam( 1 );

  if ( !area_imp || !name )
    return new BError( "Invalid parameter type" );

  auto area = area_imp->area();
  Items::Item* item = area->find_root_item( name->value() );

  if ( item != nullptr )
    return new EItemRefObjImp( item );
  return new BError( "Root item not found." );
}

BObjectImp* StorageExecutorModule::mf_DestroyRootItemInStorageArea()
{
  auto area_imp =
      static_cast<Core::StorageAreaImp*>( getParamImp( 0, Bscript::BObjectImp::OTStorageArea ) );
  const String* name = getStringParam( 1 );

  if ( !area_imp || !name )
    return new BError( "Invalid parameter type" );

  auto area = area_imp->area();

  bool result = area->delete_root_item( name->value() );
  return new BLong( result ? 1 : 0 );
}

BObjectImp* StorageExecutorModule::mf_CreateRootItemInStorageArea()
{
  auto area_imp =
      static_cast<Core::StorageAreaImp*>( getParamImp( 0, Bscript::BObjectImp::OTStorageArea ) );
  const String* name;
  const Items::ItemDesc* descriptor;

  if ( area_imp == nullptr || !getStringParam( 1, name ) || !getObjtypeParam( 2, descriptor ) )
    return new BError( "Invalid parameter type" );

  auto area = area_imp->area();
  Items::Item* item = Items::Item::create( *descriptor );
  if ( item == nullptr )
    return new BError( "Unable to create item" );

  item->setname( name->value() );

  if ( item->realm() == nullptr )  // TODO POS no realm needed
    item->setposition( Core::Pos4d( 0, 0, 0, Core::find_realm( std::string( "britannia" ) ) ) );

  area->insert_root_item( item );

  return new EItemRefObjImp( item );
}
}  // namespace Module
}  // namespace Pol
