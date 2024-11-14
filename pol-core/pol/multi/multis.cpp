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
#include "../../clib/cfgelem.h"
#include "../../clib/logfacility.h"
#include "../../clib/passert.h"
#include "../../clib/streamsaver.h"
#include "../../plib/uconst.h"
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
  const auto& desc = static_cast<const Items::MultiDesc&>( itemdesc );
  multiid_ = desc.multiid;
  items_decay_ = desc.items_decay;

  if ( !MultiDefByMultiIDExists( desc.multiid ) )
  {
    ERROR_PRINTLN( "Tried to create a Multi type {:#x}", desc.objtype );
    throw std::runtime_error( "Invalid Multi type" );
  }
  ++Core::stateManager.uobjcount.umulti_count;
}

UMulti::~UMulti()
{
  --Core::stateManager.uobjcount.umulti_count;
}

void UMulti::double_click( Network::Client* /*client*/ )
{
  ERROR_PRINTLN( "Ack! You can't double-click a multi!" );
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
  passert( MultiDefByMultiIDExists( multiid_ ) );

  return *MultiDefByMultiID( multiid_ );
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

u8 UMulti::visible_size() const
{
  const MultiDef& md = multidef();
  return md.max_radius;
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
  case Bscript::MBR_ITEMS_DECAY:
    return new Bscript::BLong( items_decay() );
  default:
    return nullptr;
  }
}

Bscript::BObjectImp* UMulti::get_script_member( const char* membername ) const
{
  Bscript::ObjMember* objmember = Bscript::getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->get_script_member_id( objmember->id );
  return nullptr;
}

Bscript::BObjectImp* UMulti::set_script_member_id( const int id, int value )
{
  auto* imp = base::set_script_member_id( id, value );
  if ( imp != nullptr )
    return imp;

  switch ( id )
  {
  case Bscript::MBR_ITEMS_DECAY:
    items_decay_ = value != 0;
    return new Bscript::BLong( items_decay_ );
  default:
    break;
  }
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

void UMulti::readProperties( Clib::ConfigElem& elem )
{
  base::readProperties( elem );
  // POL098 and earlier was not saving a MultiID in its data files,
  // but it was using 0x4000 + id as graphic instead. Not respecting
  // this would rotate most of the boats during POL098 -> POL99 migration
  if ( as_boat() )
  {
    if ( Core::settingsManager.polvar.DataWrittenBy99OrLater )
      multiid_ = elem.remove_ushort( "MultiID", this->multidef().multiid );
  }
  else
    multiid_ = elem.remove_ushort( "MultiID", multidef().multiid );
  const auto& desc = static_cast<const Items::MultiDesc&>( itemdesc() );
  items_decay_ = elem.remove_ushort( "ItemsDecay", desc.items_decay );
}

void UMulti::printProperties( Clib::StreamWriter& sw ) const
{
  base::printProperties( sw );

  sw.add( "MultiID", multiid_ );
  if ( static_cast<const Items::MultiDesc&>( itemdesc() ).items_decay != items_decay_ )
    sw.add( "ItemsDecay", items_decay_ );
}

void UMulti::items_decay( bool decay )
{
  items_decay_ = decay;
}

size_t UMulti::estimatedSize() const
{
  return base::estimatedSize() + sizeof( u16 ) /*multiid*/
         + sizeof( bool ) /*items_decay*/;
}
}  // namespace Multi
}  // namespace Pol
