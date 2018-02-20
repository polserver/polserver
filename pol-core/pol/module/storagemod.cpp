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
#include "../realms.h"
#include "../storage.h"
#include "../uoexhelp.h"
#include "../uoscrobj.h"


namespace Pol
{
namespace Bscript
{
using namespace Module;
template <>
TmplExecutorModule<StorageExecutorModule>::FunctionTable
    TmplExecutorModule<StorageExecutorModule>::function_table = {
        {"StorageAreas", &StorageExecutorModule::mf_StorageAreas},
        {"FindStorageArea", &StorageExecutorModule::mf_FindStorageArea},
        {"CreateStorageArea", &StorageExecutorModule::mf_CreateStorageArea},
        {"FindRootItemInStorageArea", &StorageExecutorModule::mf_FindRootItemInStorageArea},
        {"CreateRootItemInStorageArea", &StorageExecutorModule::mf_CreateRootItemInStorageArea},
        {"DestroyRootItemInStorageArea", &StorageExecutorModule::mf_DestroyRootItemInStorageArea}};
}
namespace Core
{
Bscript::BObjectImp* CreateStorageAreasImp();
}
namespace Module
{
using namespace Bscript;

StorageExecutorModule::StorageExecutorModule( Bscript::Executor& exec )
    : Bscript::TmplExecutorModule<StorageExecutorModule>( "Storage", exec )
{
}

BObjectImp* StorageExecutorModule::mf_StorageAreas()
{
  return Core::CreateStorageAreasImp();
}


BObjectImp* StorageExecutorModule::mf_FindStorageArea()
{
  const String* str = getStringParam( 0 );
  if ( str != NULL )
  {
    Core::StorageArea* area = Core::gamestate.storage.find_area( str->value() );

    if ( area )
      return new BApplicPtr( &storage_area_type, area );
  }
  return new BLong( 0 );  // non-string passed, or not found.
}

BObjectImp* StorageExecutorModule::mf_CreateStorageArea()
{
  String* name = EXPLICIT_CAST( String*, BObjectImp* )( getParamImp( 0, BObjectImp::OTString ) );
  if ( name )
  {
    Core::StorageArea* area = Core::gamestate.storage.create_area( name->value() );
    if ( area )
      return new BApplicPtr( &storage_area_type, area );
  }
  return new BLong( 0 );  // non-string passed, or not found.
}

BObjectImp* StorageExecutorModule::mf_FindRootItemInStorageArea()
{
  Core::StorageArea* area =
      EXPLICIT_CAST( Core::StorageArea*, void* )( exec.getApplicPtrParam( 0, &storage_area_type ) );
  const String* name = getStringParam( 1 );

  if ( !area || !name )
    return new BError( "Invalid parameter type" );

  Items::Item* item = area->find_root_item( name->value() );

  if ( item != NULL )
    return new EItemRefObjImp( item );
  else
    return new BError( "Root item not found." );
}

BObjectImp* StorageExecutorModule::mf_DestroyRootItemInStorageArea()
{
  Core::StorageArea* area =
      EXPLICIT_CAST( Core::StorageArea*, void* )( getApplicPtrParam( 0, &storage_area_type ) );
  const String* name = getStringParam( 1 );

  if ( !area || !name )
    return new BError( "Invalid parameter type" );

  bool result = area->delete_root_item( name->value() );
  return new BLong( result ? 1 : 0 );
}

BObjectImp* StorageExecutorModule::mf_CreateRootItemInStorageArea()
{
  Core::StorageArea* area =
      EXPLICIT_CAST( Core::StorageArea*, void* )( getApplicPtrParam( 0, &storage_area_type ) );
  const String* name;
  const Items::ItemDesc* descriptor;

  if ( area == NULL || !getStringParam( 1, name ) || !Core::getObjtypeParam( exec, 2, descriptor ) )
    return new BError( "Invalid parameter type" );

  Items::Item* item = Items::Item::create( *descriptor );
  if ( item == NULL )
    return new BError( "Unable to create item" );

  item->setname( name->value() );

  if ( item->realm == NULL )
    item->realm = Core::find_realm( std::string( "britannia" ) );

  area->insert_root_item( item );

  return new EItemRefObjImp( item );
}
}
}
