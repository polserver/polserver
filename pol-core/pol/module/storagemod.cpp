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
        {"StorageAreas", &StorageExecutorModule::mf_StorageAreas, UINT_MAX},
        {"FindStorageArea", &StorageExecutorModule::mf_FindStorageArea, UINT_MAX},
        {"CreateStorageArea", &StorageExecutorModule::mf_CreateStorageArea, UINT_MAX},
        {"FindRootItemInStorageArea", &StorageExecutorModule::mf_FindRootItemInStorageArea, UINT_MAX},
        {"CreateRootItemInStorageArea", &StorageExecutorModule::mf_CreateRootItemInStorageArea, UINT_MAX},
        {"DestroyRootItemInStorageArea", &StorageExecutorModule::mf_DestroyRootItemInStorageArea, UINT_MAX}};
}  // namespace Bscript
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
  if ( str != nullptr )
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

  if ( item != nullptr )
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

  if ( area == nullptr || !getStringParam( 1, name ) ||
       !Core::getObjtypeParam( exec, 2, descriptor ) )
    return new BError( "Invalid parameter type" );

  Items::Item* item = Items::Item::create( *descriptor );
  if ( item == nullptr )
    return new BError( "Unable to create item" );

  item->setname( name->value() );

  if ( item->realm == nullptr )
    item->realm = Core::find_realm( std::string( "britannia" ) );

  area->insert_root_item( item );

  return new EItemRefObjImp( item );
}
}  // namespace Module
}  // namespace Pol
