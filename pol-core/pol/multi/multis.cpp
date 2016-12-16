/** @file
 *
 * @par History
 * - 2005/11/26 Shinigami: changed "strcmp" into "stricmp" to suppress Script Errors
 * - 2009/09/03 MuadDib:   Relocation of multi related cpp/h
 */


#include "multi.h"

#include "../../clib/passert.h"
#include "../../clib/strutil.h"
#include "../../clib/logfacility.h"

#include "../../bscript/bstruct.h"
#include "../../bscript/objmembers.h"

#include "../item/itemdesc.h"
#include "../objtype.h"
#include "../globals/state.h"

#include "../uobject.h"
#include "../ustruct.h"
#include "../polcfg.h"

#include <stdexcept>

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
  return NULL;
}
UHouse* UMulti::as_house()
{
  return NULL;
}

void UMulti::register_object( UObject* /*obj*/ )
{
}

void UMulti::unregister_object( UObject* /*obj*/ )
{
}

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
  ret->addMember( "xmin", new Bscript::BLong( x + md.minrx ) );
  ret->addMember( "xmax", new Bscript::BLong( x + md.maxrx ) );
  ret->addMember( "ymin", new Bscript::BLong( y + md.minry ) );
  ret->addMember( "ymax", new Bscript::BLong( y + md.maxry ) );
  return ret.release();
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
    return NULL;
  }
}

Bscript::BObjectImp* UMulti::get_script_member( const char* membername ) const
{
  Bscript::ObjMember* objmember = Bscript::getKnownObjMember( membername );
  if ( objmember != NULL )
    return this->get_script_member_id( objmember->id );
  else
    return NULL;
}

size_t UMulti::estimatedSize() const
{
  return base::estimatedSize() + sizeof( u16 ) /*multiid*/
      ;
}
}
}
