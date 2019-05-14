/** @file
 *
 * @par History
 * - 2009/08/25 Shinigami: STLport-5.2.1 fix: init order changed of realm and saveonexit_
 * - 2009/09/14 MuadDib:   UObject::setgraphic added error printing.
 * - 2009/12/02 Turley:    added config.max_tile_id - Tomi
 */


#include "uobject.h"

#include <atomic>
#include <iosfwd>
#include <stddef.h>
#include <string>

#include "../clib/cfgelem.h"
#include "../clib/logfacility.h"
#include "../clib/passert.h"
#include "../clib/rawtypes.h"
#include "../clib/refptr.h"
#include "../clib/streamsaver.h"
#include "../plib/clidata.h"
#include "../plib/systemstate.h"
#include "../plib/uconst.h"
#include "baseobject.h"
#include "dynproperties.h"
#include "globals/state.h"
#include "globals/uvars.h"
#include "item/itemdesc.h"
#include "objtype.h"
#include "polcfg.h"
#include "proplist.h"
#include "realms.h"
#include "realms/realm.h"
#include "syshookscript.h"
#include "tooltips.h"
#include "uobjcnt.h"

namespace Pol
{
namespace Core
{
std::set<UObject*> unreaped_orphan_instances;
std::ofstream orphans_txt( "orphans.txt", std::ios::out | std::ios::trunc );

int display_orphan( UObject* o )
{
  Clib::FMTStreamWriter sw;
  Clib::OFStreamWriter sw_orphan( &orphans_txt );
  sw() << o->name() << ", " << o->ref_counted_count() << '\n';
  o->printOn( sw );
  o->printOnDebug( sw_orphan );
  INFO_PRINT << sw().str();
  // ref_ptr<UObject>::display_referers( o->as_ref_counted() );

  return 0;
}
void display_unreaped_orphan_instances()
{
  //    orphans_txt.open( "orphans.txt", ios::out|ios::trunc );

  for ( auto& obj : unreaped_orphan_instances )
  {
    display_orphan( obj );
  }
  // for( std::set<UObject*>::iterator itr = unreaped_orphan_instances.begin();
}


std::atomic<unsigned int> UObject::dirty_writes;
std::atomic<unsigned int> UObject::clean_writes;

UObject::UObject( u32 objtype, UOBJ_CLASS i_uobj_class )
    : ref_counted(),
      ULWObject( i_uobj_class ),
      DynamicPropsHolder(),
      serial_ext( 0 ),
      objtype_( objtype ),
      color( 0 ),
      facing( Plib::FACING_N ),
      _rev( 0 ),
      name_( "" ),
      flags_(),
      proplist_( CPropProfiler::class_to_type( i_uobj_class ) )
{
  graphic = Items::getgraphic( objtype );
  flags_.set( OBJ_FLAGS::DIRTY );
  flags_.set( OBJ_FLAGS::SAVE_ON_EXIT );
  height = Plib::tileheight( graphic );
  ++stateManager.uobjcount.uobject_count;
}

UObject::~UObject()
{
  if ( ref_counted::count() != 0 )
  {
    POLLOG_INFO << "Ouch! UObject::~UObject() with count()==" << ref_counted::count() << "\n";
  }
  passert( ref_counted::count() == 0 );
  if ( serial == 0 )
  {
    --stateManager.uobjcount.unreaped_orphans;
  }
  --stateManager.uobjcount.uobject_count;
}

size_t UObject::estimatedSize() const
{
  size_t size = sizeof( UObject ) + proplist_.estimatedSize();
  size += estimateSizeDynProps();
  return size;
}

//
//    Every UObject is registered with the objecthash after creation.
//    (This can't happen in the ctor since the object isn't fully created yet)
//
//    Scripts may still reference any object, so rather than outright delete,
//    we set its serial to 0 (marking it "orphan", though "zombie" would probably be a better term).
//    Later, when all _other_ references to the object have been deleted,
//    objecthash.Reap() will remove its reference to this object, deleting it.
//
void UObject::destroy()
{
  if ( serial != 0 )
  {
    if ( ref_counted::count() < 1 )
    {
      POLLOG_INFO << "Ouch! UObject::destroy() with count()==" << ref_counted::count() << "\n";
    }

    set_dirty();  // we will have to write a 'object deleted' directive once

    serial =
        0;  // used to set serial_ext to 0.  This way, if debugging, one can find out the old serial
    passert( ref_counted::count() >= 1 );

    ++stateManager.uobjcount.unreaped_orphans;
  }
}

bool UObject::dirty() const
{
  return flags_.get( OBJ_FLAGS::DIRTY );
}

void UObject::clear_dirty() const
{
  if ( dirty() )
    ++dirty_writes;
  else
    ++clean_writes;
  flags_.remove( OBJ_FLAGS::DIRTY );
}

bool UObject::getprop( const std::string& propname, std::string& propval ) const
{
  return proplist_.getprop( propname, propval );
}

void UObject::setprop( const std::string& propname, const std::string& propvalue )
{
  if ( propname[0] != '#' )
    set_dirty();
  proplist_.setprop( propname, propvalue );  // VOID_RETURN
}

void UObject::eraseprop( const std::string& propname )
{
  if ( propname[0] != '#' )
    set_dirty();
  proplist_.eraseprop( propname );  // VOID_RETURN
}

void UObject::copyprops( const UObject& from )
{
  set_dirty();
  proplist_.copyprops( from.proplist_ );
}

void UObject::copyprops( const PropertyList& proplist )
{
  set_dirty();
  proplist_.copyprops( proplist );
}

void UObject::getpropnames( std::vector<std::string>& propnames ) const
{
  proplist_.getpropnames( propnames );
}

const PropertyList& UObject::getprops() const
{
  return proplist_;
}

std::string UObject::name() const
{
  return name_;
}

std::string UObject::description() const
{
  return name_;
}

void UObject::setname( const std::string& newname )
{
  set_dirty();
  increv();
  send_object_cache_to_inrange( this );
  name_ = newname;
}

UObject* UObject::owner()
{
  return nullptr;
}

const UObject* UObject::owner() const
{
  return nullptr;
}

UObject* UObject::self_as_owner()
{
  return this;
}

const UObject* UObject::self_as_owner() const
{
  return this;
}

UObject* UObject::toplevel_owner()
{
  return this;
}

const UObject* UObject::toplevel_owner() const
{
  return this;
}


void UObject::printProperties( Clib::StreamWriter& sw ) const
{
  using namespace fmt;

  if ( !name_.get().empty() )
    sw() << "\tName\t" << name_.get() << pf_endl;

  sw() << "\tSerial\t0x" << hex( serial ) << pf_endl;
  sw() << "\tObjType\t0x" << hex( objtype_ ) << pf_endl;
  sw() << "\tGraphic\t0x" << hex( graphic ) << pf_endl;

  if ( color != 0 )
    sw() << "\tColor\t0x" << hex( color ) << pf_endl;

  sw() << "\tX\t" << x << pf_endl;
  sw() << "\tY\t" << y << pf_endl;
  sw() << "\tZ\t" << (int)z << pf_endl;

  if ( facing )
    sw() << "\tFacing\t" << static_cast<int>( facing ) << pf_endl;

  sw() << "\tRevision\t" << rev() << pf_endl;
  if ( realm == nullptr )
    sw() << "\tRealm\tbritannia" << pf_endl;
  else
    sw() << "\tRealm\t" << realm->name() << pf_endl;

  s16 value = fire_resist().mod;
  if ( value != 0 )
    sw() << "\tFireResistMod\t" << static_cast<int>( value ) << pf_endl;
  value = cold_resist().mod;
  if ( value != 0 )
    sw() << "\tColdResistMod\t" << static_cast<int>( value ) << pf_endl;
  value = energy_resist().mod;
  if ( value != 0 )
    sw() << "\tEnergyResistMod\t" << static_cast<int>( value ) << pf_endl;
  value = poison_resist().mod;
  if ( value != 0 )
    sw() << "\tPoisonResistMod\t" << static_cast<int>( value ) << pf_endl;
  value = physical_resist().mod;
  if ( value != 0 )
    sw() << "\tPhysicalResistMod\t" << static_cast<int>( value ) << pf_endl;

  value = fire_damage().mod;
  if ( value != 0 )
    sw() << "\tFireDamageMod\t" << static_cast<int>( value ) << pf_endl;
  value = cold_damage().mod;
  if ( value != 0 )
    sw() << "\tColdDamageMod\t" << static_cast<int>( value ) << pf_endl;
  value = energy_damage().mod;
  if ( value != 0 )
    sw() << "\tEnergyDamageMod\t" << static_cast<int>( value ) << pf_endl;
  value = poison_damage().mod;
  if ( value != 0 )
    sw() << "\tPoisonDamageMod\t" << static_cast<int>( value ) << pf_endl;
  value = physical_damage().mod;
  if ( value != 0 )
    sw() << "\tPhysicalDamageMod\t" << static_cast<int>( value ) << pf_endl;
  // new mod stuff
  value = lower_reagent_cost().mod;
  if ( value )
    sw() << "\tLowerReagentCostMod\t" << static_cast<int>( value ) << pf_endl;
  value = defence_increase().mod;
  if ( value )
    sw() << "\tDefenceIncreaseMod\t" << static_cast<int>( value ) << pf_endl;
  value = defence_increase_cap().mod;
  if ( value )
    sw() << "\tDefenceIncreaseCapMod\t" << static_cast<int>( value ) << pf_endl;
  value = lower_mana_cost().mod;
  if ( value )
    sw() << "\tLowerManaCostMod\t" << static_cast<int>( value ) << pf_endl;
  value = hit_chance().mod;
  if ( value )
    sw() << "\tHitChanceMod\t" << static_cast<int>( value ) << pf_endl;
  value = fire_resist_cap().mod;
  if ( value )
    sw() << "\tFireResistCapMod\t" << static_cast<int>( value ) << pf_endl;
  value = cold_resist_cap().mod;
  if ( value )
    sw() << "\tColdResistCapMod\t" << static_cast<int>( value ) << pf_endl;
  value = energy_resist_cap().mod;
  if ( value )
    sw() << "\tEnergyResistCapMod\t" << static_cast<int>( value ) << pf_endl;
  value = physical_resist_cap().mod;
  if ( value )
    sw() << "\tPhysicalResistCapMod\t" << static_cast<int>( value ) << pf_endl;
  value = poison_resist_cap().mod;
  if ( value )
    sw() << "\tPoisonResistCapMod\t" << static_cast<int>( value ) << pf_endl;
  value = spell_damage_increase().mod;
  if ( value )
    sw() << "\tSpellDamageIncreaseMod\t" << static_cast<int>( value ) << pf_endl;
  value = faster_casting().mod;
  if ( value )
    sw() << "\tFasterCastingMod\t" << static_cast<int>( value ) << pf_endl;
  value = faster_cast_recovery().mod;
  if ( value )
    sw() << "\tFasterCastRecoveryMod\t" << static_cast<int>( value ) << pf_endl;
  value = luck().mod;
  if ( value )
    sw() << "\tLuckMod\t" << static_cast<int>( value ) << pf_endl;
  // end new mod stuff


  proplist_.printProperties( sw );
}

void UObject::printDebugProperties( Clib::StreamWriter& sw ) const
{
  sw() << "# uobj_class: " << (int)uobj_class_ << pf_endl;
}

/// Fixes invalid graphic, moving here to allow it to be overridden in subclass (see Multi)
void UObject::fixInvalidGraphic()
{
  if ( graphic > ( Plib::systemstate.config.max_tile_id ) )
    graphic = GRAPHIC_NODRAW;
}

void UObject::readProperties( Clib::ConfigElem& elem )
{
  name_ = elem.remove_string( "NAME", "" );

  // serial, objtype extracted by caller
  graphic = elem.remove_ushort( "GRAPHIC", static_cast<u16>( objtype_ ) );
  fixInvalidGraphic();

  height = Plib::tileheight( graphic );

  color = elem.remove_ushort( "COLOR", 0 );


  std::string realmstr = elem.remove_string( "Realm", "britannia" );
  realm = find_realm( realmstr );
  if ( !realm )
  {
    ERROR_PRINT.Format( "{} '{}' (0x{:X}): has an invalid realm property '{}'.\n" )
        << classname() << name() << serial << realmstr;
    throw std::runtime_error( "Data integrity error" );
  }
  x = elem.remove_ushort( "X" );
  y = elem.remove_ushort( "Y" );
  z = static_cast<s8>( elem.remove_int( "Z" ) );
  if ( !realm->valid( x, y, z ) )
  {
    x = static_cast<u16>( realm->width() ) - 1;
    y = static_cast<u16>( realm->height() ) - 1;
    z = 0;
  }

  unsigned short tmp = elem.remove_ushort( "FACING", 0 );
  setfacing( static_cast<unsigned char>( tmp ) );

  _rev = elem.remove_ulong( "Revision", 0 );


  proplist_.readProperties( elem );
}

void UObject::printSelfOn( Clib::StreamWriter& sw ) const
{
  printOn( sw );
}

void UObject::printOn( Clib::StreamWriter& sw ) const
{
  sw() << classname() << pf_endl;
  sw() << "{" << pf_endl;
  printProperties( sw );
  sw() << "}" << pf_endl;
  sw() << pf_endl;
  // sw.flush();
}

void UObject::printOnDebug( Clib::StreamWriter& sw ) const
{
  sw() << classname() << pf_endl;
  sw() << "{" << pf_endl;
  printProperties( sw );
  printDebugProperties( sw );
  sw() << "}" << pf_endl;
  sw() << pf_endl;
  // sw.flush();
}

Clib::StreamWriter& operator<<( Clib::StreamWriter& writer, const UObject& obj )
{
  obj.printOn( writer );
  return writer;
}

bool UObject::setgraphic( u16 /*newgraphic*/ )
{
  ERROR_PRINT.Format(
      "UOBject::SetGraphic used, object class does not have a graphic member! Object Serial: "
      "0x{:X}\n" )
      << serial;
  return false;
}

bool UObject::setcolor( u16 newcolor )
{
  set_dirty();

  if ( color != newcolor )
  {
    color = newcolor;
    on_color_changed();
  }

  return true;
}

void UObject::on_color_changed() {}

void UObject::on_facing_changed() {}

bool UObject::saveonexit() const
{
  return flags_.get( OBJ_FLAGS::SAVE_ON_EXIT );
}

void UObject::saveonexit( bool newvalue )
{
  flags_.change( OBJ_FLAGS::SAVE_ON_EXIT, newvalue );
}

const char* UObject::target_tag() const
{
  return "object";
}

bool UObject::get_method_hook( const char* methodname, Bscript::Executor* ex, ExportScript** hook,
                               unsigned int* PC ) const
{
  return gamestate.system_hooks.get_method_hook( gamestate.system_hooks.uobject_method_script.get(),
                                                 methodname, ex, hook, PC );
}
}  // namespace Core
}  // namespace Pol
