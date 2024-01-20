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
#include <utility>

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
#include "realms/realm.h"
#include "realms/realms.h"
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
  Clib::StreamWriter sw( nullptr );
  Clib::StreamWriter sw_orphan( &orphans_txt );
  sw.write( "{}, {}\n", o->name(), o->ref_counted_count() );
  o->printOn( sw );
  o->printOnDebug( sw_orphan );
  INFO_PRINT( sw.buffer() );
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
      facing( Core::FACING_N ),
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
    POLLOG_INFOLN( "Ouch! UObject::~UObject() with count()=={}", ref_counted::count() );
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
      POLLOG_INFOLN( "Ouch! UObject::destroy() with count()=={}", ref_counted::count() );
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

void UObject::setposition( Pos4d newpos )
{
  set_dirty();
  pos( std::move( newpos ) );
}

UFACING UObject::direction_toward( UObject* other ) const
{
  return pos2d().direction_toward( other->toplevel_owner()->pos2d() );
}
UFACING UObject::direction_toward( const Pos2d& other ) const
{
  return pos2d().direction_toward( other );
}
UFACING UObject::direction_away( UObject* other ) const
{
  return pos2d().direction_away( other->toplevel_owner()->pos2d() );
}
UFACING UObject::direction_away( const Pos2d& other ) const
{
  return pos2d().direction_away( other );
}

void UObject::printProperties( Clib::StreamWriter& sw ) const
{
  using namespace fmt;

  if ( !name_.get().empty() )
    sw.add( "Name", name_.get() );

  sw.add( "Serial", Clib::hexint( serial ) );
  sw.add( "ObjType", Clib::hexint( objtype_ ) );
  sw.add( "Graphic", Clib::hexint( graphic ) );

  if ( color != 0 )
    sw.add( "Color", Clib::hexint( color ) );

  sw.add( "X", x() );
  sw.add( "Y", y() );
  sw.add( "Z", (int)z() );

  if ( facing )
    sw.add( "Facing", static_cast<int>( facing ) );

  sw.add( "Revision", rev() );
  if ( realm() == nullptr )
    sw.add( "Realm", "britannia" );
  else
    sw.add( "Realm", realm()->name() );

  s16 value = fire_resist().mod;
  if ( value != 0 )
    sw.add( "FireResistMod", static_cast<int>( value ) );
  value = cold_resist().mod;
  if ( value != 0 )
    sw.add( "ColdResistMod", static_cast<int>( value ) );
  value = energy_resist().mod;
  if ( value != 0 )
    sw.add( "EnergyResistMod", static_cast<int>( value ) );
  value = poison_resist().mod;
  if ( value != 0 )
    sw.add( "PoisonResistMod", static_cast<int>( value ) );
  value = physical_resist().mod;
  if ( value != 0 )
    sw.add( "PhysicalResistMod", static_cast<int>( value ) );

  value = fire_damage().mod;
  if ( value != 0 )
    sw.add( "FireDamageMod", static_cast<int>( value ) );
  value = cold_damage().mod;
  if ( value != 0 )
    sw.add( "ColdDamageMod", static_cast<int>( value ) );
  value = energy_damage().mod;
  if ( value != 0 )
    sw.add( "EnergyDamageMod", static_cast<int>( value ) );
  value = poison_damage().mod;
  if ( value != 0 )
    sw.add( "PoisonDamageMod", static_cast<int>( value ) );
  value = physical_damage().mod;
  if ( value != 0 )
    sw.add( "PhysicalDamageMod", static_cast<int>( value ) );
  // new mod stuff
  value = lower_reagent_cost().mod;
  if ( value )
    sw.add( "LowerReagentCostMod", static_cast<int>( value ) );
  value = defence_increase().mod;
  if ( value )
    sw.add( "DefenceIncreaseMod", static_cast<int>( value ) );
  value = defence_increase_cap().mod;
  if ( value )
    sw.add( "DefenceIncreaseCapMod", static_cast<int>( value ) );
  value = lower_mana_cost().mod;
  if ( value )
    sw.add( "LowerManaCostMod", static_cast<int>( value ) );
  value = hit_chance().mod;
  if ( value )
    sw.add( "HitChanceMod", static_cast<int>( value ) );
  value = fire_resist_cap().mod;
  if ( value )
    sw.add( "FireResistCapMod", static_cast<int>( value ) );
  value = cold_resist_cap().mod;
  if ( value )
    sw.add( "ColdResistCapMod", static_cast<int>( value ) );
  value = energy_resist_cap().mod;
  if ( value )
    sw.add( "EnergyResistCapMod", static_cast<int>( value ) );
  value = physical_resist_cap().mod;
  if ( value )
    sw.add( "PhysicalResistCapMod", static_cast<int>( value ) );
  value = poison_resist_cap().mod;
  if ( value )
    sw.add( "PoisonResistCapMod", static_cast<int>( value ) );
  value = spell_damage_increase().mod;
  if ( value )
    sw.add( "SpellDamageIncreaseMod", static_cast<int>( value ) );
  value = faster_casting().mod;
  if ( value )
    sw.add( "FasterCastingMod", static_cast<int>( value ) );
  value = faster_cast_recovery().mod;
  if ( value )
    sw.add( "FasterCastRecoveryMod", static_cast<int>( value ) );
  value = luck().mod;
  if ( value )
    sw.add( "LuckMod", static_cast<int>( value ) );
  value = swing_speed_increase().mod;
  if ( value )
    sw.add( "SwingSpeedIncreaseMod", static_cast<int>( value ) );
  // end new mod stuff


  proplist_.printProperties( sw );
}

void UObject::printDebugProperties( Clib::StreamWriter& sw ) const
{
  sw.write( "# uobj_class: {}\n", (int)uobj_class_ );
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
  auto* realm_tmp = find_realm( realmstr );
  if ( !realm_tmp )
  {
    ERROR_PRINTLN( "{} '{}' ({:#X}): has an invalid realm property '{}'.", classname(), name(),
                   serial, realmstr );
    throw std::runtime_error( "Data integrity error" );
  }
  setposition( Pos4d( elem.remove_ushort( "X" ), elem.remove_ushort( "Y" ),
                      static_cast<s8>( elem.remove_int( "Z" ) ), realm_tmp ) );

  unsigned short tmp = elem.remove_ushort( "FACING", 0 );
  setfacing( static_cast<unsigned char>( tmp ) );

  _rev = elem.remove_ulong( "Revision", 0 );

  s16 mod_value = static_cast<s16>( elem.remove_int( "FIRERESISTMOD", 0 ) );
  if ( mod_value != 0 )
    fire_resist( fire_resist().setAsMod( mod_value ) );
  mod_value = static_cast<s16>( elem.remove_int( "COLDRESISTMOD", 0 ) );
  if ( mod_value != 0 )
    cold_resist( cold_resist().setAsMod( mod_value ) );
  mod_value = static_cast<s16>( elem.remove_int( "ENERGYRESISTMOD", 0 ) );
  if ( mod_value != 0 )
    energy_resist( energy_resist().setAsMod( mod_value ) );
  mod_value = static_cast<s16>( elem.remove_int( "POISONRESISTMOD", 0 ) );
  if ( mod_value != 0 )
    poison_resist( poison_resist().setAsMod( mod_value ) );
  mod_value = static_cast<s16>( elem.remove_int( "PHYSICALRESISTMOD", 0 ) );
  if ( mod_value != 0 )
    physical_resist( physical_resist().setAsMod( mod_value ) );

  mod_value = static_cast<s16>( elem.remove_int( "FIREDAMAGEMOD", 0 ) );
  if ( mod_value != 0 )
    fire_damage( fire_damage().setAsMod( mod_value ) );
  mod_value = static_cast<s16>( elem.remove_int( "COLDDAMAGEMOD", 0 ) );
  if ( mod_value != 0 )
    cold_damage( cold_damage().setAsMod( mod_value ) );
  mod_value = static_cast<s16>( elem.remove_int( "ENERGYDAMAGEMOD", 0 ) );
  if ( mod_value != 0 )
    energy_damage( energy_damage().setAsMod( mod_value ) );
  mod_value = static_cast<s16>( elem.remove_int( "POISONDAMAGEMOD", 0 ) );
  if ( mod_value != 0 )
    poison_damage( poison_damage().setAsMod( mod_value ) );
  mod_value = static_cast<s16>( elem.remove_int( "PHYSICALDAMAGEMOD", 0 ) );
  if ( mod_value != 0 )
    physical_damage( physical_damage().setAsMod( mod_value ) );
  mod_value = static_cast<s16>( elem.remove_int( "DEFENCEINCREASEMOD", 0 ) );
  if ( mod_value != 0 )
    defence_increase( defence_increase().setAsMod( mod_value ) );
  mod_value = static_cast<s16>( elem.remove_int( "DEFENCEINCREASECAPMOD", 0 ) );
  if ( mod_value != 0 )
    defence_increase_cap( defence_increase_cap().setAsMod( mod_value ) );
  mod_value = static_cast<s16>( elem.remove_int( "LOWERMANACOSTMOD", 0 ) );
  if ( mod_value != 0 )
    lower_mana_cost( lower_mana_cost().setAsMod( mod_value ) );
  mod_value = static_cast<s16>( elem.remove_int( "HITCHANCEMOD", 0 ) );
  if ( mod_value != 0 )
    hit_chance( hit_chance().setAsMod( mod_value ) );
  mod_value = static_cast<s16>( elem.remove_int( "FIRERESISTCAPMOD", 0 ) );
  if ( mod_value != 0 )
    fire_resist_cap( fire_resist_cap().setAsMod( mod_value ) );
  mod_value = static_cast<s16>( elem.remove_int( "COLDRESISTCAPMOD", 0 ) );
  if ( mod_value != 0 )
    cold_resist_cap( cold_resist_cap().setAsMod( mod_value ) );
  mod_value = static_cast<s16>( elem.remove_int( "ENERGYRESISTCAPMOD", 0 ) );
  if ( mod_value != 0 )
    energy_resist_cap( energy_resist_cap().setAsMod( mod_value ) );
  mod_value = static_cast<s16>( elem.remove_int( "POISONRESISTCAPMOD", 0 ) );
  if ( mod_value != 0 )
    poison_resist_cap( poison_resist_cap().setAsMod( mod_value ) );
  mod_value = static_cast<s16>( elem.remove_int( "PHYSICALRESISTCAPMOD", 0 ) );
  if ( mod_value != 0 )
    physical_resist_cap( physical_resist_cap().setAsMod( mod_value ) );
  mod_value = static_cast<s16>( elem.remove_int( "LOWERREAGENTCOSTMOD", 0 ) );
  if ( mod_value != 0 )
    lower_reagent_cost( lower_reagent_cost().setAsMod( mod_value ) );
  mod_value = static_cast<s16>( elem.remove_int( "SPELLDAMAGEINCREASEMOD", 0 ) );
  if ( mod_value != 0 )
    spell_damage_increase( spell_damage_increase().setAsMod( mod_value ) );
  mod_value = static_cast<s16>( elem.remove_int( "FASTERCASTINGMOD", 0 ) );
  if ( mod_value != 0 )
    faster_casting( faster_casting().setAsMod( mod_value ) );
  mod_value = static_cast<s16>( elem.remove_int( "FASTERCASTRECOVERYMOD", 0 ) );
  if ( mod_value != 0 )
    faster_cast_recovery( faster_cast_recovery().setAsMod( mod_value ) );
  mod_value = static_cast<s16>( elem.remove_int( "LUCKMOD", 0 ) );
  if ( mod_value != 0 )
    luck( luck().setAsMod( mod_value ) );
  mod_value = static_cast<s16>( elem.remove_int( "SWINGSPEEDINCREASEMOD", 0 ) );
  if ( mod_value != 0 )
    swing_speed_increase( swing_speed_increase().setAsMod( mod_value ) );


  proplist_.readProperties( elem );
}

void UObject::printSelfOn( Clib::StreamWriter& sw ) const
{
  printOn( sw );
}

void UObject::printOn( Clib::StreamWriter& sw ) const
{
  sw.write( "{}\n{{\n", classname() );
  printProperties( sw );
  sw.write( "}\n\n" );
}

void UObject::printOnDebug( Clib::StreamWriter& sw ) const
{
  sw.write( "{}\n{{\n", classname() );
  printProperties( sw );
  printDebugProperties( sw );
  sw.write( "}\n\n" );
}

bool UObject::setgraphic( u16 /*newgraphic*/ )
{
  ERROR_PRINTLN(
      "UOBject::SetGraphic used, object class does not have a graphic member! Object Serial: {:#X}",
      serial );
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
