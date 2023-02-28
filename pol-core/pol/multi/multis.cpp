/** @file
 *
 * @par History
 * - 2005/11/26 Shinigami: changed "strcmp" into "stricmp" to suppress Script Errors
 * - 2009/09/03 MuadDib:   Relocation of multi related cpp/h
 */


#include <stddef.h>

#include "../../bscript/bobject.h"
#include "../../bscript/bstruct.h"
#include "../../bscript/executor.h"
#include "../../bscript/objmembers.h"
#include "../../clib/logfacility.h"
#include "../../clib/passert.h"
#include "../baseobject.h"
#include "../globals/state.h"
#include "../globals/uvars.h"
#include "../item/item.h"
#include "../item/itemdesc.h"
#include "../syshookscript.h"
#include "../uobjcnt.h"
#include "multi.h"
#include "multidef.h"

namespace Pol
{
namespace Multi
{
UMulti::UMulti( const Items::ItemDesc& itemdesc ) : Item( itemdesc, Core::UOBJ_CLASS::CLASS_MULTI )
{
  multiid = itemdesc.multiid;

  if ( !MultiDefByMultiIDExists( itemdesc.multiid ) )
  {
    ERROR_PRINT.Format( "Tried to create a Multi type 0x{:X}\n" ) << itemdesc.objtype;
    throw std::runtime_error( "Unvalid Multi type" );
  }
  ++Core::stateManager.uobjcount.umulti_count;
}

UMulti::~UMulti()
{
  --Core::stateManager.uobjcount.umulti_count;
}

void UMulti::double_click( Network::Client* /*client*/ )
{
  ERROR_PRINT << "Ack! You can't double-click a multi!\n";
  throw std::runtime_error( "double_click() on a multi should not be possible." );
}

UBoat* UMulti::as_boat()
{
  return nullptr;
}
UHouse* UMulti::as_house()
{
  return nullptr;
}

void UMulti::register_object( UObject* /*obj*/ ) {}

void UMulti::unregister_object( UObject* /*obj*/ ) {}

const char* UMulti::classname() const
{
  return "MULTI";
}

const MultiDef& UMulti::multidef() const
{
  passert( MultiDefByMultiIDExists( multiid ) );

  return *MultiDefByMultiID( multiid );
}

Bscript::BStruct* UMulti::footprint() const
{
  const MultiDef& md = multidef();
  std::unique_ptr<Bscript::BStruct> ret( new Bscript::BStruct );
  ret->addMember( "xmin", new Bscript::BLong( x() + md.minrxyz.x() ) );
  ret->addMember( "xmax", new Bscript::BLong( x() + md.maxrxyz.x() ) );
  ret->addMember( "ymin", new Bscript::BLong( y() + md.minrxyz.y() ) );
  ret->addMember( "ymax", new Bscript::BLong( y() + md.maxrxyz.y() ) );
  return ret.release();
}
Core::Range3d UMulti::current_box() const
{
  const MultiDef& md = multidef();
  return Core::Range3d( pos() + md.minrxyz, pos() + md.maxrxyz );
}

Core::Range3d UMulti::search_box() const
{
  return current_box();
}

Bscript::BObjectImp* UMulti::get_script_member_id( const int id ) const  /// id test
{
  Bscript::BObjectImp* imp = base::get_script_member_id( id );
  if ( imp )
    return imp;

  switch ( id )
  {
  case Bscript::MBR_FOOTPRINT:
    return footprint();
    break;
  default:
    return nullptr;
  }
}

Bscript::BObjectImp* UMulti::get_script_member( const char* membername ) const
{
  Bscript::ObjMember* objmember = Bscript::getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->get_script_member_id( objmember->id );
  else
    return nullptr;
}

bool UMulti::get_method_hook( const char* methodname, Bscript::Executor* ex,
                              Core::ExportScript** hook, unsigned int* PC ) const
{
  if ( Core::gamestate.system_hooks.get_method_hook(
           Core::gamestate.system_hooks.multi_method_script.get(), methodname, ex, hook, PC ) )
    return true;
  return base::get_method_hook( methodname, ex, hook, PC );
}

size_t UMulti::estimatedSize() const
{
  return base::estimatedSize() + sizeof( u16 ) /*multiid*/
      ;
}
}  // namespace Multi
}  // namespace Pol
