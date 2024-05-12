/** @file
 *
 * @par History
 * - 2005/02/14 Shinigami: find_container_desc - fixed passert condition
 * - 2005/12/07 MuadDib:   Changed "has already been specified for objtype" to
 *                         "is the same as objtype" for easier non-english understanding.
 * - 2006/05/19 MuadDib:   Removed read_itemdesc_file( "config/wepndesc.cfg" ); line, due to
 *                         no longer being used by ANYTHING in the core. So no longer will
 *                         require this file to run POL. Same with config/armrdesc.cfg
 * - 2009/08/25 Shinigami: STLport-5.2.1 fix: init order changed of max_slots
 * - 2009/09/03 MuadDib:   Relocation of multi related cpp/h
 * - 2009/12/02 Turley:    added config.max_tile_id - Tomi
 */


#include "itemdesc.h"

#include <ctype.h>
#include <iosfwd>
#include <iterator>
#include <stdlib.h>
#include <string.h>

#include "../../bscript/bobject.h"
#include "../../bscript/bstruct.h"
#include "../../bscript/dict.h"
#include "../../bscript/impstr.h"
#include "../../clib/cfgelem.h"
#include "../../clib/cfgfile.h"
#include "../../clib/esignal.h"
#include "../../clib/fileutil.h"
#include "../../clib/logfacility.h"
#include "../../clib/passert.h"
#include "../../clib/stlutil.h"
#include "../../clib/strutil.h"
#include "../../plib/clidata.h"
#include "../../plib/mapcell.h"
#include "../../plib/pkg.h"
#include "../../plib/systemstate.h"
#include "../../plib/uconst.h"
#include "../dice.h"
#include "../extobj.h"
#include "../globals/settings.h"
#include "../globals/uvars.h"
#include "../multi/multidef.h"
#include "../network/pktdef.h"
#include "../proplist.h"
#include "../syshookscript.h"
#include "../uobject.h"
#include "armrtmpl.h"
#include "regions/resource.h"
#include "wepntmpl.h"


namespace Pol
{
namespace Items
{
unsigned int get_objtype_byname( const char* str )
{
  auto itr = Core::gamestate.objtype_byname.find( str );
  if ( itr == Core::gamestate.objtype_byname.end() )
    return 0;
  else
    return ( *itr ).second;
}

unsigned int get_objtype_from_string( const std::string& str )
{
  unsigned int objtype;
  const char* ot_str = str.c_str();
  if ( isdigit( *ot_str ) )
  {
    objtype = static_cast<u32>( strtoul( ot_str, nullptr, 0 ) );
  }
  else
  {
    objtype = get_objtype_byname( ot_str );
    if ( !objtype )
    {
      throw std::runtime_error( "Can't convert " + str + " to an objtype" );
    }
  }
  return objtype;
}

ResourceComponent::ResourceComponent( const std::string& rname, unsigned amount )
    : rd( Core::find_resource_def( rname ) ), amount( amount )
{
  if ( rd == nullptr )
  {
    ERROR_PRINTLN( "itemdesc.cfg: Resource '{}' not found", rname );
    throw std::runtime_error( "Configuration error" );
  }
}


ItemDesc* ItemDesc::create( Clib::ConfigElem& elem, const Plib::Package* pkg )
{
  u32 objtype = static_cast<u32>( strtoul( elem.rest(), nullptr, 0 ) );
  if ( !objtype )
  {
    elem.throw_error( "Element must have objtype specified" );
  }

  if ( Core::gamestate.old_objtype_conversions.count( objtype ) )
  {
    elem.throw_error(
        "Objtype is defined as an OldObjtype of " +
        find_itemdesc( Core::gamestate.old_objtype_conversions[objtype] ).objtype_description() );
  }

  ItemDesc* descriptor = nullptr;

  if ( elem.type_is( "Container" ) )
  {
    descriptor = new ContainerDesc( objtype, elem, pkg );
  }
  else if ( elem.type_is( "Item" ) )
  {
    descriptor = new ItemDesc( objtype, elem, ItemDesc::ITEMDESC, pkg );
  }
  else if ( elem.type_is( "Door" ) )
  {
    descriptor = new DoorDesc( objtype, elem, pkg );
  }
  else if ( elem.type_is( "Weapon" ) )
  {
    // This one is making leaks when the descriptor is not given to a UWeapon object...
    // I really can't figure a good way to avoid this leak everytime
    descriptor = new WeaponDesc( objtype, elem, pkg );
  }
  else if ( elem.type_is( "Armor" ) )
  {
    // Ugly but effective workaround, needed to avoid coverage error
    bool forceShield = ( objtype == Core::settingsManager.extobj.shield );
    descriptor = new ArmorDesc( objtype, elem, pkg, forceShield );
  }
  else if ( elem.type_is( "Boat" ) )
  {
    descriptor = new BoatDesc( objtype, elem, pkg );
  }
  else if ( elem.type_is( "House" ) )
  {
    descriptor = new HouseDesc( objtype, elem, pkg );
  }
  else if ( elem.type_is( "Spellbook" ) )
  {
    descriptor = new SpellbookDesc( objtype, elem, pkg );
  }
  else if ( elem.type_is( "SpellScroll" ) )
  {
    descriptor = new SpellScrollDesc( objtype, elem, pkg );
  }
  else if ( elem.type_is( "Map" ) )
  {
    descriptor = new MapDesc( objtype, elem, pkg );
  }
  else
  {
    elem.throw_error( std::string( "Unexpected element type: " ) + elem.type() );
  }
  return descriptor;
}

ItemDesc::ItemDesc( u32 objtype, Clib::ConfigElem& elem, Type type, const Plib::Package* pkg )
    : type( type ),
      pkg( pkg ),
      objtype( objtype ),
      graphic( elem.remove_ushort( "GRAPHIC", 0 ) ),
      // Changed from Valid Color Mask to cfg mask in ssopt.
      color( elem.remove_ushort( "COLOR", 0 ) & Core::settingsManager.ssopt.item_color_mask ),
      facing( static_cast<unsigned char>( elem.remove_ushort( "FACING", 127 ) ) ),
      desc( elem.remove_string( "DESC", "" ) ),
      tooltip( elem.remove_string( "TOOLTIP", "" ) ),
      walk_on_script( elem.remove_string( "WALKONSCRIPT", "" ), pkg, "scripts/items/" ),
      on_use_script( elem.remove_string( "SCRIPT", "" ), pkg, "scripts/items/" ),
      equip_script( elem.remove_string( "EQUIPSCRIPT", "" ) ),
      unequip_script( elem.remove_string( "UNEQUIPSCRIPT", "" ) ),
      snoop_script( elem.remove_string( "SNOOPSCRIPT", "" ), pkg, "scripts/items/" ),
      control_script( elem.remove_string( "CONTROLSCRIPT", "" ), pkg, "scripts/control/" ),
      create_script( elem.remove_string( "CREATESCRIPT", "" ), pkg, "scripts/control/" ),
      destroy_script( elem.remove_string( "DESTROYSCRIPT", "" ), pkg, "scripts/control/" ),
      requires_attention( elem.remove_bool( "REQUIRESATTENTION", true ) ),
      lockable( elem.remove_bool( "LOCKABLE", false ) ),
      vendor_sells_for( elem.remove_ulong( "VENDORSELLSFOR", 0 ) ),
      vendor_buys_for( elem.remove_ulong( "VENDORBUYSFOR", 0 ) ),
      decay_time(
          elem.remove_ulong( "DECAYTIME", Core::settingsManager.ssopt.default_decay_time ) ),
      movable( DEFAULT ),
      doubleclick_range( elem.remove_ushort(
          "DoubleclickRange", Core::settingsManager.ssopt.default_doubleclick_range ) ),
      use_requires_los( elem.remove_bool( "UseRequiresLOS", true ) ),  // Dave 11/24
      ghosts_can_use( elem.remove_bool( "GhostsCanUse", false ) ),     // Dave 11/24
      can_use_while_paralyzed( elem.remove_bool( "CanUseWhileParalyzed", false ) ),
      can_use_while_frozen( elem.remove_bool( "CanUseWhileFrozen", false ) ),
      newbie( elem.remove_bool( "NEWBIE", false ) ),
      insured( elem.remove_bool( "INSURED", false ) ),
      invisible( elem.remove_bool( "INVISIBLE", false ) ),
      cursed( elem.remove_bool( "CURSED", false ) ),
      decays_on_multis( elem.remove_bool( "DecaysOnMultis", false ) ),
      blocks_casting_if_in_hand( elem.remove_bool( "BlocksCastingIfInHand", true ) ),
      no_drop( elem.remove_bool( "NoDrop", false ) ),
      base_str_req( elem.remove_ushort( "StrRequired", 0 ) * 10 ),
      quality( elem.remove_double( "QUALITY", 1.0 ) ),
      lower_reag_cost( 0 ),
      spell_damage_increase( 0 ),
      faster_casting( 0 ),
      faster_cast_recovery( 0 ),
      defence_increase( 0 ),
      defence_increase_cap( 0 ),
      lower_mana_cost( 0 ),
      hit_chance( 0 ),
      resist_fire_cap( 0 ),
      resist_cold_cap( 0 ),
      resist_energy_cap( 0 ),
      resist_physical_cap( 0 ),
      resist_poison_cap( 0 ),
      defence_increase_mod( 0 ),
      defence_increase_cap_mod( 0 ),
      lower_mana_cost_mod( 0 ),
      hit_chance_mod( 0 ),
      resist_fire_cap_mod( 0 ),
      resist_cold_cap_mod( 0 ),
      resist_energy_cap_mod( 0 ),
      resist_physical_cap_mod( 0 ),
      resist_poison_cap_mod( 0 ),
      lower_reagent_cost_mod( 0 ),
      spell_damage_increase_mod( 0 ),
      faster_casting_mod( 0 ),
      faster_cast_recovery_mod( 0 ),
      luck( 0 ),
      luck_mod( 0 ),
      swing_speed_increase( 0 ),
      swing_speed_increase_mod( 0 ),
      props( Core::CPropProfiler::Type::ITEM ),
      method_script( nullptr ),
      save_on_exit( elem.remove_bool( "SaveOnExit", true ) )

{
  if ( type == BOATDESC || type == HOUSEDESC )
  {
    multiid = elem.remove_ushort( "MultiID", 0xFFFF );

    if ( multiid == 0xFFFF )
    {
      ERROR_PRINTLN(
          "Itemdesc has no 'multiid' specified for a multi.\n"
          "      Note: read corechanges.txt for the new multi format" );
      elem.throw_error( "Configuration error" );
    }
  }
  else
  {
    if ( graphic == 0 )
    {
      if ( objtype <= Plib::systemstate.config.max_tile_id )
      {
        graphic = static_cast<u16>( objtype );
      }
      else
      {
        ERROR_PRINTLN( "Itemdesc has no 'graphic' specified, and no 'objtype' is valid either." );
        elem.throw_error( "Configuration error" );
      }
    }
  }

  maxhp = elem.remove_ushort( "MAXHP", 0 );


  // Make sure Weapons and Armors ALL have this value defined to not break the core combat system
  if ( maxhp == 0 && ( type == WEAPONDESC || type == ARMORDESC ) )
  {
    ERROR_PRINTLN( "itemdesc.cfg, objtype {:#x}  has no MaxHP specified.", objtype );
    elem.throw_error( "Configuration error" );
  }


  unsigned short stacklimit = elem.remove_ushort( "StackLimit", MAX_STACK_ITEMS );

  if ( stacklimit > MAX_STACK_ITEMS )
    stack_limit = MAX_STACK_ITEMS;
  else
    stack_limit = stacklimit;

  if ( tooltip.length() > PKTOUT_B7_MAX_CHARACTERS )
  {
    tooltip.erase( PKTOUT_B7_MAX_CHARACTERS, std::string::npos );
  }

  unsigned short in_movable;
  if ( elem.remove_prop( "MOVABLE", &in_movable ) )
  {
    movable = in_movable ? MOVABLE : UNMOVABLE;
  }

  std::string weight_str;
  if ( elem.remove_prop( "WEIGHT", &weight_str ) )
  {
    const char* s = weight_str.c_str();
    char* endptr;
    weightmult = strtoul( s, &endptr, 0 );
    s = static_cast<const char*>( endptr );
    while ( *s && isspace( *s ) )
      ++s;
    if ( *s == '/' )
    {
      ++s;
      weightdiv = strtoul( s, &endptr, 0 );
      if ( !weightdiv )
      {
        elem.throw_error( "Poorly formed weight directive: " + weight_str );
      }
    }
    else if ( *s == '\0' )
    {
      weightdiv = 1;
    }
    else
    {
      elem.throw_error( "Poorly formed weight directive: " + weight_str );
    }
  }
  else
  {
    weightmult = Plib::tileweight( graphic );
    weightdiv = 1;
  }

  std::string temp;
  while ( elem.remove_prop( "Resource", &temp ) )
  {
    ISTRINGSTREAM is( temp );
    std::string rname;
    unsigned amount;
    if ( is >> rname >> amount )
    {
      resources.push_back( ResourceComponent( rname, amount ) );
    }
    else
    {
      ERROR_PRINTLN( "itemdesc.cfg, objtype {:#x} : Resource '{}' is malformed.", objtype, temp );
      throw std::runtime_error( "Configuration file error" );
    }
  }

  while ( elem.remove_prop( "Name", &temp ) || elem.remove_prop( "ObjtypeName", &temp ) )
  {
    if ( Core::gamestate.objtype_byname.count( temp.c_str() ) )
    {
      ERROR_PRINTLN( "Warning! objtype {:#x} : ObjtypeName '{}' is the same as objtype {:#x}",
                     objtype, temp, Core::gamestate.objtype_byname[temp.c_str()] );
      // throw runtime_error( "Configuration file error" );
    }
    else
    {
      Core::gamestate.objtype_byname[temp.c_str()] = objtype;
    }

    if ( objtypename.empty() )
      objtypename = temp;

    /*
            if (objtype_byname.count( temp.c_str() ))
            {
            cerr << "itemdesc.cfg, objtype 0x" << hex << objtype << dec
            << ": Name '" << temp << "' has already been specified for objtype 0x"
            << hex << objtype_byname[ temp.c_str() ] << dec << endl;
            throw runtime_error( "Configuration file error" );
            }
            */
    // if (!objtype_byname.count( temp.c_str() ))
    //  objtype_byname[ temp.c_str() ] = objtype;
  }

  props.readProperties( elem );

  if ( elem.remove_prop( "MethodScript", &temp ) )
  {
    if ( pkg == nullptr )
      throw std::runtime_error( "MethodScripts can only be specified in a package" );
    if ( !temp.empty() )
    {
      Core::ExportScript* shs = new Core::ExportScript( pkg, temp );
      if ( shs->Initialize() )
        method_script = shs;
      else
        delete shs;
    }
  }

  unsigned int old_objtype;
  while ( elem.remove_prop( "OldObjtype", &old_objtype ) )
  {
    if ( Core::gamestate.old_objtype_conversions.count( old_objtype ) )
    {
      elem.throw_error( objtype_description() + " specifies OldObjtype " +
                        Clib::hexint( old_objtype ) + " which is already mapped to " +
                        find_itemdesc( Core::gamestate.old_objtype_conversions[old_objtype] )
                            .objtype_description() );
    }
    if ( has_itemdesc( old_objtype ) )
    {
      elem.throw_error(
          objtype_description() + " specifies OldObjtype " + Clib::hexint( old_objtype ) +
          " which is already defined as " +
          find_itemdesc( Core::gamestate.old_objtype_conversions[objtype] ).objtype_description() );
    }
    Core::gamestate.old_objtype_conversions[old_objtype] = objtype;
  }

  if ( elem.remove_prop( "StackingIgnoresCProps", &temp ) )
  {
    ISTRINGSTREAM is( temp );
    std::string cprop_name;
    while ( is >> cprop_name )
    {
      ignore_cprops.insert( cprop_name );
    }
  }
  auto diceValue = [this]( const std::string& value,
                           const std::string& error_msg ) -> unsigned short
  {
    Core::Dice dice;
    std::string errmsg;
    if ( !dice.load( value.c_str(), &errmsg ) )
    {
      ERROR_PRINTLN( "Error loading itemdesc.cfg {} for {} : {}", error_msg, objtype_description(),
                     errmsg );
      throw std::runtime_error( "Error loading Item Mods" );
    }
    return dice.roll();
  };

  if ( elem.remove_prop( "SpellDamageIncrease", &temp ) )
    spell_damage_increase = diceValue( temp, "Spell Damage Increase" );

  if ( elem.remove_prop( "LowerReagentCost", &temp ) )
    lower_reag_cost = diceValue( temp, "Lower Reagent Cost" );


  if ( elem.remove_prop( "FasterCasting", &temp ) )
    faster_casting = diceValue( temp, "Faster Casting" );

  if ( elem.remove_prop( "FasterCastRecovery", &temp ) )
    faster_cast_recovery = diceValue( temp, "Faster Cast Recovery" );

  if ( elem.remove_prop( "DefenceIncrease", &temp ) )
    defence_increase = diceValue( temp, "Defence Increase" );

  if ( elem.remove_prop( "DefenceIncreaseCap", &temp ) )
    defence_increase_cap = diceValue( temp, "Defence Increase Cap" );

  if ( elem.remove_prop( "LowerManaCost", &temp ) )
    lower_mana_cost = diceValue( temp, "Lower man Cost" );

  if ( elem.remove_prop( "HitChance", &temp ) )
    hit_chance = diceValue( temp, "Hit Chance" );

  if ( elem.remove_prop( "FireResistCap", &temp ) )
    resist_fire_cap = diceValue( temp, "Fire Resist Cap" );

  if ( elem.remove_prop( "ColdResistCap", &temp ) )
    resist_cold_cap = diceValue( temp, "Cold Resist Cap" );

  if ( elem.remove_prop( "EnergyResistCap", &temp ) )
    resist_energy_cap = diceValue( temp, "Energy Resist Cap" );

  if ( elem.remove_prop( "PhysicalResistCap", &temp ) )
    resist_physical_cap = diceValue( temp, "Physical Resist Cap" );

  if ( elem.remove_prop( "PoisonResistCap", &temp ) )
    resist_poison_cap = diceValue( temp, "Poison Resist Cap" );

  if ( elem.remove_prop( "LUCK", &temp ) )
    luck = diceValue( temp, "Luck" );

  if ( elem.remove_prop( "SWINGSPEEDINCREASE", &temp ) )
    swing_speed_increase = diceValue( temp, "Swing Speed Increase" );

  // change mods to dice rolls if needed
  if ( elem.remove_prop( "DefenceIncreaseMod", &temp ) )
    defence_increase_mod = diceValue( temp, "Defence Increase Mod" );

  if ( elem.remove_prop( "DefenceIncreaseCapMod", &temp ) )
    defence_increase_cap_mod = diceValue( temp, "Defence Increase Cap Mod" );

  if ( elem.remove_prop( "LowerManaCostMod", &temp ) )
    lower_mana_cost_mod = diceValue( temp, "Lower Mana Cost Mod" );

  if ( elem.remove_prop( "hitchance_mod", &temp ) )  // to be made redundant in the future
    hit_chance_mod = diceValue( temp, "Hit Chance Mod" );

  if ( elem.remove_prop( "HitChanceMod", &temp ) )
    hit_chance_mod = diceValue( temp, "Hit Chance Mod" );

  if ( elem.remove_prop( "FireResistCapMod", &temp ) )
    resist_fire_cap_mod = diceValue( temp, "Fire Resist Cap Mod" );

  if ( elem.remove_prop( "ColdResistCapMod", &temp ) )
    resist_cold_cap_mod = diceValue( temp, "Cold Resist Cap" );

  if ( elem.remove_prop( "EnergyResistCapMod", &temp ) )
    resist_energy_cap_mod = diceValue( temp, "Energy Resist Cap" );

  if ( elem.remove_prop( "PhysicalResistCapMod", &temp ) )
    resist_physical_cap_mod = diceValue( temp, "Physical Resist Cap" );

  if ( elem.remove_prop( "PoisonResistCapMod", &temp ) )
    resist_poison_cap_mod = diceValue( temp, "Poison Resist Cap" );

  if ( elem.remove_prop( "LowerReagentCostMod", &temp ) )
    lower_reagent_cost_mod = diceValue( temp, "Lower Reagent Cost Mod" );

  if ( elem.remove_prop( "SpellDamageIncreaseMod", &temp ) )
    spell_damage_increase_mod = diceValue( temp, "Spell Damage Increase Mod" );

  if ( elem.remove_prop( "FasterCastingMod", &temp ) )
    faster_casting_mod = diceValue( temp, "Faster Casting Mod" );

  if ( elem.remove_prop( "FasterCastRecoveryMod", &temp ) )
    faster_cast_recovery_mod = diceValue( temp, "Faster Cast Recovery Mod" );

  if ( elem.remove_prop( "LUCKMOD", &temp ) )
    luck_mod = diceValue( temp, "Luck Mod" );

  if ( elem.remove_prop( "SWINGSPEEDINCREASEMOD", &temp ) )
    swing_speed_increase_mod = diceValue( temp, "Swing Speed Increase Mod" );

  memset( &element_resist, 0, sizeof( element_resist ) );
  memset( &element_damage, 0, sizeof( element_damage ) );
  for ( unsigned resist = 0; resist <= Core::ELEMENTAL_TYPE_MAX; ++resist )
  {
    std::string tmp;
    bool passed = false;

    switch ( resist )
    {
    case Core::ELEMENTAL_FIRE:
      passed = elem.remove_prop( "FIRERESIST", &tmp );
      break;
    case Core::ELEMENTAL_COLD:
      passed = elem.remove_prop( "COLDRESIST", &tmp );
      break;
    case Core::ELEMENTAL_ENERGY:
      passed = elem.remove_prop( "ENERGYRESIST", &tmp );
      break;
    case Core::ELEMENTAL_POISON:
      passed = elem.remove_prop( "POISONRESIST", &tmp );
      break;
    case Core::ELEMENTAL_PHYSICAL:
      passed = elem.remove_prop( "PHYSICALRESIST", &tmp );
      break;
    }

    if ( passed )
    {
      Core::Dice dice;
      std::string errmsg;
      if ( !dice.load( tmp.c_str(), &errmsg ) )
      {
        ERROR_PRINTLN( "Error loading itemdesc.cfg Elemental Resistances for {} : {}",
                       objtype_description(), errmsg );
        throw std::runtime_error( "Error loading Item Elemental Resistances" );
      }
      switch ( resist )
      {
      case Core::ELEMENTAL_FIRE:
        element_resist.fire = dice.roll();
        break;
      case Core::ELEMENTAL_COLD:
        element_resist.cold = dice.roll();
        break;
      case Core::ELEMENTAL_ENERGY:
        element_resist.energy = dice.roll();
        break;
      case Core::ELEMENTAL_POISON:
        element_resist.poison = dice.roll();
        break;
      case Core::ELEMENTAL_PHYSICAL:
        element_resist.physical = dice.roll();
        break;
      }
    }
  }

  for ( unsigned edamage = 0; edamage <= Core::ELEMENTAL_TYPE_MAX; ++edamage )
  {
    std::string tmp;
    bool passed = false;

    switch ( edamage )
    {
    case Core::ELEMENTAL_FIRE:
      passed = elem.remove_prop( "FIREDAMAGE", &tmp );
      break;
    case Core::ELEMENTAL_COLD:
      passed = elem.remove_prop( "COLDDAMAGE", &tmp );
      break;
    case Core::ELEMENTAL_ENERGY:
      passed = elem.remove_prop( "ENERGYDAMAGE", &tmp );
      break;
    case Core::ELEMENTAL_POISON:
      passed = elem.remove_prop( "POISONDAMAGE", &tmp );
      break;
    case Core::ELEMENTAL_PHYSICAL:
      passed = elem.remove_prop( "PHYSICALDAMAGE", &tmp );
      break;
    }

    if ( passed )
    {
      Core::Dice dice;
      std::string errmsg;
      if ( !dice.load( tmp.c_str(), &errmsg ) )
      {
        ERROR_PRINTLN( "Error loading itemdesc.cfg elemental damages for {} : {}",
                       objtype_description(), errmsg );
        throw std::runtime_error( "Error loading Item Elemental Damages" );
      }
      switch ( edamage )
      {
      case Core::ELEMENTAL_FIRE:
        element_damage.fire = dice.roll();
        break;
      case Core::ELEMENTAL_COLD:
        element_damage.cold = dice.roll();
        break;
      case Core::ELEMENTAL_ENERGY:
        element_damage.energy = dice.roll();
        break;
      case Core::ELEMENTAL_POISON:
        element_damage.poison = dice.roll();
        break;
      case Core::ELEMENTAL_PHYSICAL:
        element_damage.physical = dice.roll();
        break;
      }
    }
  }
}

ItemDesc::ItemDesc( Type type )
    : type( type ),
      pkg( nullptr ),
      objtype( 0 ),
      graphic( 0 ),
      color( 0 ),
      facing( 127 ),
      weightmult( 1 ),
      weightdiv( 1 ),
      desc( "" ),
      tooltip( "" ),
      // walk_on_script2(""),
      // on_use_script2(""),
      // control_script2(""),
      // create_script2(""),
      // destroy_script2(""),
      requires_attention( false ),
      lockable( false ),
      vendor_sells_for( 0 ),
      vendor_buys_for( 0 ),
      decay_time( Core::settingsManager.ssopt.default_decay_time ),
      movable( DEFAULT ),
      doubleclick_range( Core::settingsManager.ssopt.default_doubleclick_range ),
      use_requires_los( true ),
      ghosts_can_use( false ),
      can_use_while_paralyzed( false ),
      can_use_while_frozen( false ),
      newbie( false ),
      insured( false ),
      invisible( false ),
      cursed( false ),
      decays_on_multis( false ),
      blocks_casting_if_in_hand( true ),
      no_drop( false ),
      base_str_req( 0 ),
      stack_limit( MAX_STACK_ITEMS ),
      quality( 1.0 ),
      multiid( 0xFFFF ),
      maxhp( 0 ),
      lower_reag_cost( 0 ),
      spell_damage_increase( 0 ),
      faster_casting( 0 ),
      faster_cast_recovery( 0 ),
      defence_increase( 0 ),
      defence_increase_cap( 0 ),
      lower_mana_cost( 0 ),
      hit_chance( 0 ),
      resist_fire_cap( 0 ),
      resist_cold_cap( 0 ),
      resist_energy_cap( 0 ),
      resist_physical_cap( 0 ),
      resist_poison_cap( 0 ),
      defence_increase_mod( 0 ),
      defence_increase_cap_mod( 0 ),
      lower_mana_cost_mod( 0 ),
      hit_chance_mod( 0 ),
      resist_fire_cap_mod( 0 ),
      resist_cold_cap_mod( 0 ),
      resist_energy_cap_mod( 0 ),
      resist_physical_cap_mod( 0 ),
      resist_poison_cap_mod( 0 ),
      lower_reagent_cost_mod( 0 ),
      spell_damage_increase_mod( 0 ),
      faster_casting_mod( 0 ),
      faster_cast_recovery_mod( 0 ),
      luck( 0 ),
      luck_mod( 0 ),
      swing_speed_increase( 0 ),
      swing_speed_increase_mod( 0 ),
      props( Core::CPropProfiler::Type::ITEM ),
      method_script( nullptr ),
      save_on_exit( true )
{
  memset( &element_resist, 0, sizeof( element_resist ) );
  memset( &element_damage, 0, sizeof( element_damage ) );
}

ItemDesc::~ItemDesc()
{
  unload_scripts();
}

void ItemDesc::PopulateStruct( Bscript::BStruct* descriptor ) const
{
  using namespace Bscript;
  std::string typestr;
  switch ( type )
  {
  case ITEMDESC:
    typestr = "Item";
    break;
  case CONTAINERDESC:
    typestr = "Container";
    break;
  case DOORDESC:
    typestr = "Door";
    break;
  case WEAPONDESC:
    typestr = "Weapon";
    break;
  case ARMORDESC:
    typestr = "Armor";
    break;
  case BOATDESC:
    typestr = "Boat";
    break;
  case HOUSEDESC:
    typestr = "House";
    break;
  case SPELLBOOKDESC:
    typestr = "Spellbook";
    break;
  case SPELLSCROLLDESC:
    typestr = "Spellscroll";
    break;
  case MAPDESC:
    typestr = "Map";
    break;
  default:
    typestr = "Unknown";
    break;
  }
  descriptor->addMember( "ObjClass", new String( typestr ) );
  descriptor->addMember( "ObjType", new BLong( objtype ) );
  descriptor->addMember( "Graphic", new BLong( graphic ) );
  descriptor->addMember( "Name", new String( objtypename ) );
  descriptor->addMember( "Color", new BLong( color ) );
  descriptor->addMember( "Facing", new BLong( facing ) );
  descriptor->addMember( "Desc", new String( desc ) );
  descriptor->addMember( "Tooltip", new String( tooltip ) );
  descriptor->addMember( "WalkOnScript", new String( walk_on_script.relativename( pkg ) ) );
  descriptor->addMember( "Script", new String( on_use_script.relativename( pkg ) ) );
  descriptor->addMember( "EquipScript", new String( equip_script ) );
  descriptor->addMember( "UnequipScript", new String( unequip_script ) );
  descriptor->addMember( "SnoopScript", new String( snoop_script.relativename( pkg ) ) );
  descriptor->addMember( "ControlScript", new String( control_script.relativename( pkg ) ) );
  descriptor->addMember( "CreateScript", new String( create_script.relativename( pkg ) ) );
  descriptor->addMember( "DestroyScript", new String( destroy_script.relativename( pkg ) ) );
  descriptor->addMember( "MethodScript",
                         new String( method_script ? method_script->scriptname() : "" ) );
  descriptor->addMember( "RequiresAttention", new BLong( requires_attention ) );
  descriptor->addMember( "Lockable", new BLong( lockable ) );
  descriptor->addMember( "VendorSellsFor", new BLong( vendor_sells_for ) );
  descriptor->addMember( "VendorBuysFor", new BLong( vendor_buys_for ) );
  descriptor->addMember( "DecayTime", new BLong( decay_time ) );
  descriptor->addMember( "Movable", new BLong( default_movable() ) );
  descriptor->addMember( "DoubleClickRange", new BLong( doubleclick_range ) );
  descriptor->addMember( "UseRequiresLOS", new BLong( use_requires_los ) );
  descriptor->addMember( "GhostsCanUse", new BLong( ghosts_can_use ) );
  descriptor->addMember( "CanUseWhileFrozen", new BLong( can_use_while_frozen ) );
  descriptor->addMember( "CanUseWhileParalyzed", new BLong( can_use_while_paralyzed ) );
  descriptor->addMember( "Newbie", new BLong( newbie ) );
  descriptor->addMember( "Insured", new BLong( insured ) );
  descriptor->addMember( "Cursed", new BLong( cursed ) );
  descriptor->addMember( "Invisible", new BLong( invisible ) );
  descriptor->addMember( "DecaysOnMultis", new BLong( decays_on_multis ) );
  descriptor->addMember( "BlocksCastingIfInHand", new BLong( blocks_casting_if_in_hand ) );
  descriptor->addMember( "NoDrop", new BLong( no_drop ) );
  descriptor->addMember( "StrRequired", new BLong( base_str_req ) );
  descriptor->addMember( "StackLimit", new BLong( stack_limit ) );
  descriptor->addMember( "Weight", new Double( static_cast<double>( weightmult ) / weightdiv ) );
  descriptor->addMember( "FireResist", new BLong( element_resist.fire ) );
  descriptor->addMember( "ColdResist", new BLong( element_resist.cold ) );
  descriptor->addMember( "EnergyResist", new BLong( element_resist.energy ) );
  descriptor->addMember( "PoisonResist", new BLong( element_resist.poison ) );
  descriptor->addMember( "PhysicalResist", new BLong( element_resist.physical ) );
  descriptor->addMember( "FireDamage", new BLong( element_damage.fire ) );
  descriptor->addMember( "ColdDamage", new BLong( element_damage.cold ) );
  descriptor->addMember( "EnergyDamage", new BLong( element_damage.energy ) );
  descriptor->addMember( "PoisonDamage", new BLong( element_damage.poison ) );
  descriptor->addMember( "PhysicalDamage", new BLong( element_damage.physical ) );
  descriptor->addMember( "Quality", new Double( quality ) );
  descriptor->addMember( "MultiID", new BLong( multiid ) );
  descriptor->addMember( "MaxHp", new BLong( maxhp ) );

  // new props
  descriptor->addMember( "LowerReagentCost", new BLong( lower_reag_cost ) );
  descriptor->addMember( "SpellDamageIncrease", new BLong( spell_damage_increase ) );
  descriptor->addMember( "FasterCasting", new BLong( faster_casting ) );
  descriptor->addMember( "FasterCastRecovery", new BLong( faster_cast_recovery ) );
  descriptor->addMember( "DefenceIncrease", new BLong( defence_increase ) );
  descriptor->addMember( "DefenceIncreaseCap", new BLong( defence_increase_cap ) );
  descriptor->addMember( "LowerManaCost", new BLong( lower_mana_cost ) );
  descriptor->addMember( "HitChance", new BLong( hit_chance ) );
  descriptor->addMember( "FireResistCap", new BLong( resist_fire_cap ) );
  descriptor->addMember( "ColdResistCap", new BLong( resist_cold_cap ) );
  descriptor->addMember( "EnergyResistCap", new BLong( resist_energy_cap ) );
  descriptor->addMember( "PhysicalResistCap", new BLong( resist_physical_cap ) );
  descriptor->addMember( "PoisonResistCap", new BLong( resist_poison_cap ) );
  descriptor->addMember( "Luck", new BLong( luck ) );
  descriptor->addMember( "SwingSpeedIncrease", new BLong( swing_speed_increase ) );

  // new mods
  descriptor->addMember( "DefenceIncreaseMod", new BLong( defence_increase_mod ) );
  descriptor->addMember( "DefenceIncreaseCapMod", new BLong( defence_increase_cap_mod ) );
  descriptor->addMember( "LowerManaCostMod", new BLong( lower_mana_cost_mod ) );
  descriptor->addMember( "HitChanceMod", new BLong( hit_chance_mod ) );
  descriptor->addMember( "FireResistCapMod", new BLong( resist_fire_cap_mod ) );
  descriptor->addMember( "ColdResistCapMod", new BLong( resist_cold_cap_mod ) );
  descriptor->addMember( "EnergyResistCapMod", new BLong( resist_energy_cap_mod ) );
  descriptor->addMember( "PhysicalResistCapMod", new BLong( resist_physical_cap_mod ) );
  descriptor->addMember( "PoisonResistCapMod", new BLong( resist_poison_cap_mod ) );
  descriptor->addMember( "LowerReagentCostMod", new BLong( lower_reagent_cost_mod ) );
  descriptor->addMember( "SpellDamageIncreaseMod", new BLong( spell_damage_increase_mod ) );
  descriptor->addMember( "FasterCastingMod", new BLong( faster_casting_mod ) );
  descriptor->addMember( "FasterCastRecoveryMod", new BLong( faster_cast_recovery_mod ) );
  descriptor->addMember( "LuckMod", new BLong( luck_mod ) );
  descriptor->addMember( "SwingSpeedIncreaseMod", new BLong( swing_speed_increase_mod ) );


  std::set<std::string>::const_iterator set_itr;
  std::unique_ptr<ObjArray> ignorecp( new ObjArray );
  for ( set_itr = ignore_cprops.begin(); set_itr != ignore_cprops.end(); ++set_itr )
    ignorecp->addElement( new String( *set_itr ) );

  descriptor->addMember( "StackingIgnoresCProps", ignorecp.release() );

  auto cpropdict = new BDictionary();
  std::vector<std::string> propnames;
  std::vector<std::string>::const_iterator vec_itr;
  std::string tempval;
  props.getpropnames( propnames );
  for ( vec_itr = propnames.begin(); vec_itr != propnames.end(); ++vec_itr )
  {
    props.getprop( *vec_itr, tempval );
    cpropdict->addMember( vec_itr->c_str(), BObjectImp::unpack( tempval.c_str() ) );
  }

  descriptor->addMember( "CProps", cpropdict );

  // FIXME: this should be a single member similar to CProps, called Resources
  std::vector<ResourceComponent>::const_iterator resource_itr;
  for ( resource_itr = resources.begin(); resource_itr != resources.end(); ++resource_itr )
  {
    descriptor->addMember( "Resource", new String( resource_itr->rd->name() + " " +
                                                   Clib::tostring( resource_itr->amount ) ) );
  }
}

void ItemDesc::unload_scripts()
{
  if ( method_script != nullptr )
  {
    delete method_script;
    method_script = nullptr;
  }
}

std::string ItemDesc::objtype_description() const
{
  std::string tmp;
  if ( pkg )
    fmt::format_to( std::back_inserter( tmp ), ":{}:", pkg->name() );
  fmt::format_to( std::back_inserter( tmp ), "{} ({:#x})", objtypename, objtype );
  return tmp;
}

bool ItemDesc::default_movable() const
{
  if ( movable == DEFAULT )
    return ( ( Plib::tile_flags( graphic ) & Plib::FLAG::MOVABLE ) != 0 );
  else
    return movable ? true : false;
}

size_t ItemDesc::estimatedSize() const
{
  size_t size = sizeof( ItemDesc ) + objtypename.capacity() + tooltip.capacity() +
                walk_on_script.estimatedSize() + on_use_script.estimatedSize() +
                control_script.estimatedSize() + create_script.estimatedSize() +
                destroy_script.estimatedSize() + 3 * sizeof( ResourceComponent* ) +
                resources.capacity() * sizeof( ResourceComponent ) + props.estimatedSize();
  size += 3 * sizeof( void* );
  for ( const auto& ignore : ignore_cprops )
  {
    size += ignore.capacity() + 3 * sizeof( void* );
  }
  return size;
}

ContainerDesc::ContainerDesc( u32 objtype, Clib::ConfigElem& elem, const Plib::Package* pkg )
    : ItemDesc( objtype, elem, CONTAINERDESC, pkg ),
      gump( elem.remove_ushort( "GUMP" ) ),
      bounds( Core::Pos2d( elem.remove_ushort( "MINX" ), elem.remove_ushort( "MINY" ) ),
              Core::Pos2d( elem.remove_ushort( "MAXX" ), elem.remove_ushort( "MAXY" ) ), nullptr ),
      max_weight( elem.remove_ushort( "MAXWEIGHT",
                                      Core::settingsManager.ssopt.default_container_max_weight ) ),
      max_items( elem.remove_ushort( "MAXITEMS",
                                     Core::settingsManager.ssopt.default_container_max_items ) ),
      max_slots( static_cast<u8>(
          elem.remove_ushort( "MAXSLOTS", Core::settingsManager.ssopt.default_max_slots ) ) ),
      held_weight_multiplier_mod( elem.remove_double( "HeldWeightMultiplierMod", 1.0 ) ),
      no_drop_exception( elem.remove_bool( "NoDropException", false ) ),
      can_insert_script( elem.remove_string( "CANINSERTSCRIPT", "" ), pkg, "scripts/control/" ),
      on_insert_script( elem.remove_string( "ONINSERTSCRIPT", "" ), pkg, "scripts/control/" ),
      can_remove_script( elem.remove_string( "CANREMOVESCRIPT", "" ), pkg, "scripts/control/" ),
      on_remove_script( elem.remove_string( "ONREMOVESCRIPT", "" ), pkg, "scripts/control/" )
{
  // FIXME: in theory, should never happen due to conversion to u8. Maybe here as note during
  // rewrite. Add a remove_uchar/remove_char for allowing
  // use of max 0-255 integers control due to packet limits, in configuration files. Yay.
  //  if ( max_slots > 255 )
  //  {
  //      cerr << "Warning! Container " << hexint( objtype ) << ": Invalid MaxSlots defined.
  // MaxSlots max value is 255. Setting to 255." << endl;
  //      max_slots = 255;
  //  }
}

void ContainerDesc::PopulateStruct( Bscript::BStruct* descriptor ) const
{
  using namespace Bscript;
  base::PopulateStruct( descriptor );
  descriptor->addMember( "Gump", new BLong( gump ) );
  descriptor->addMember( "MinX", new BLong( bounds.nw().x() ) );
  descriptor->addMember( "MinY", new BLong( bounds.nw().y() ) );
  descriptor->addMember( "MaxX", new BLong( bounds.se().x() ) );
  descriptor->addMember( "MaxY", new BLong( bounds.se().y() ) );
  descriptor->addMember( "MaxWeight", new BLong( max_weight ) );
  descriptor->addMember( "MaxItems", new BLong( max_items ) );
  descriptor->addMember( "MaxSlots", new BLong( max_slots ) );
  descriptor->addMember( "NoDropException", new BLong( no_drop_exception ) );
  descriptor->addMember( "CanInsertScript", new String( can_insert_script.relativename( pkg ) ) );
  descriptor->addMember( "CanRemoveScript", new String( can_remove_script.relativename( pkg ) ) );
  descriptor->addMember( "OnInsertScript", new String( on_insert_script.relativename( pkg ) ) );
  descriptor->addMember( "OnRemoveScript", new String( on_remove_script.relativename( pkg ) ) );
}

size_t ContainerDesc::estimatedSize() const
{
  return base::estimatedSize() + sizeof( u16 ) /*gump*/
         + sizeof( Core::Range2d )             /*bounds*/
         + sizeof( u16 )                       /*max_weight*/
         + sizeof( u16 )                       /*max_items*/
         + sizeof( u8 )                        /*max_slots*/
         + sizeof( bool )                      /*no_drop_exception*/
         + sizeof( double )                    /*held_weight_multiplier_mod*/
         + can_insert_script.estimatedSize() + on_insert_script.estimatedSize() +
         can_remove_script.estimatedSize() + on_remove_script.estimatedSize();
}

DoorDesc::DoorDesc( u32 objtype, Clib::ConfigElem& elem, const Plib::Package* pkg )
    : ItemDesc( objtype, elem, DOORDESC, pkg ),
      mod( static_cast<s16>( elem.remove_int( "XMOD" ) ),
           static_cast<s16>( elem.remove_int( "YMOD" ) ) ),
      open_graphic( elem.remove_ushort( "OPENGRAPHIC", 0 ) )
{
}

void DoorDesc::PopulateStruct( Bscript::BStruct* descriptor ) const
{
  base::PopulateStruct( descriptor );
  descriptor->addMember( "XMod", new Bscript::BLong( mod.x() ) );
  descriptor->addMember( "YMod", new Bscript::BLong( mod.y() ) );
  descriptor->addMember( "OpenGraphic", new Bscript::BLong( open_graphic ) );
}
size_t DoorDesc::estimatedSize() const
{
  return base::estimatedSize() + sizeof( Core::Vec2d ) /*mod*/
         + sizeof( u16 )                               /*open_graphic*/
      ;
}

SpellbookDesc::SpellbookDesc( u32 objtype, Clib::ConfigElem& elem, const Plib::Package* pkg )
    : ContainerDesc( objtype, elem, pkg ), spelltype( elem.remove_string( "SPELLTYPE" ) )
{
  type = SPELLBOOKDESC;
}

void SpellbookDesc::PopulateStruct( Bscript::BStruct* descriptor ) const
{
  base::PopulateStruct( descriptor );
  descriptor->addMember( "Spelltype", new Bscript::String( spelltype ) );
}

size_t SpellbookDesc::estimatedSize() const
{
  return base::estimatedSize() + spelltype.capacity();
}

SpellScrollDesc::SpellScrollDesc( u32 objtype, Clib::ConfigElem& elem, const Plib::Package* pkg )
    : ItemDesc( objtype, elem, SPELLSCROLLDESC, pkg ),
      spelltype( elem.remove_string( "SPELLTYPE" ) )
{
}

void SpellScrollDesc::PopulateStruct( Bscript::BStruct* descriptor ) const
{
  base::PopulateStruct( descriptor );
  descriptor->addMember( "Spelltype", new Bscript::String( spelltype ) );
}

size_t SpellScrollDesc::estimatedSize() const
{
  return base::estimatedSize() + spelltype.capacity();
}

MultiDesc::MultiDesc( u32 objtype, Clib::ConfigElem& elem, Type type, const Plib::Package* pkg )
    : ItemDesc( objtype, elem, type, pkg )
{
  if ( !Multi::MultiDefByMultiIDExists( multiid ) )
  {
    elem.throw_error( "Multi definition not found.  Objtype=" + Clib::hexint( objtype ) +
                      ", multiid=" + Clib::hexint( multiid ) );
  }
}
void MultiDesc::PopulateStruct( Bscript::BStruct* descriptor ) const
{
  base::PopulateStruct( descriptor );
}
size_t MultiDesc::estimatedSize() const
{
  return base::estimatedSize();
}

BoatDesc::BoatDesc( u32 objtype, Clib::ConfigElem& elem, const Plib::Package* pkg )
    : MultiDesc( objtype, elem, BOATDESC, pkg ), alternates()
{
  u16 alternate;
  alternates.push_back( multiid );
  while ( elem.remove_prop( "ALTERNATEMULTIID", &alternate ) )
    alternates.push_back( alternate );
}

void BoatDesc::PopulateStruct( Bscript::BStruct* descriptor ) const
{
  base::PopulateStruct( descriptor );
  std::unique_ptr<Bscript::ObjArray> a( new Bscript::ObjArray );

  // `alternates` contains the base multi id at index 0, so start at 1.
  for ( size_t i = 1; i < alternates.size(); ++i )
    a->addElement( new Bscript::BLong( alternates[i] ) );

  descriptor->addMember( "AlternateMultiID", a.release() );
}
size_t BoatDesc::estimatedSize() const
{
  return base::estimatedSize() + alternates.capacity();
}

HouseDesc::HouseDesc( u32 objtype, Clib::ConfigElem& elem, const Plib::Package* pkg )
    : MultiDesc( objtype, elem, HOUSEDESC, pkg )
{
}

void HouseDesc::PopulateStruct( Bscript::BStruct* descriptor ) const
{
  base::PopulateStruct( descriptor );
}
size_t HouseDesc::estimatedSize() const
{
  return base::estimatedSize();
}

MapDesc::MapDesc( u32 objtype, Clib::ConfigElem& elem, const Plib::Package* pkg )
    : ItemDesc( objtype, elem, MAPDESC, pkg ), editable( elem.remove_bool( "EDITABLE", true ) )
{
}

void MapDesc::PopulateStruct( Bscript::BStruct* descriptor ) const
{
  base::PopulateStruct( descriptor );
  descriptor->addMember( "Editable", new Bscript::BLong( editable ) );
}
size_t MapDesc::estimatedSize() const
{
  return sizeof( bool ) /*editable*/ + base::estimatedSize();
}

bool has_itemdesc( u32 objtype )
{
  return Core::gamestate.desctable.count( objtype ) > 0;
}

bool objtype_is_lockable( u32 objtype )
{
  const ItemDesc& id = find_itemdesc( objtype );
  return id.lockable;
}

unsigned short getgraphic( u32 objtype )
{
  const ItemDesc& id = find_itemdesc( objtype );

  // Check here to make sure multis are created without a graphic entry in itemdesc.
  if ( id.type == id.BOATDESC || id.type == id.HOUSEDESC )
    return 0;

  if ( id.graphic )
  {
    return id.graphic;
  }
  else if ( objtype <= Plib::systemstate.config.max_tile_id )
  {
    return static_cast<u16>( objtype );
  }
  else
  {
    throw std::runtime_error( "Objtype " + Clib::hexint( objtype ) + " must define a graphic" );
  }
}
unsigned short getcolor( unsigned int objtype )
{
  const ItemDesc& id = find_itemdesc( objtype );
  return id.color;
}

const ItemDesc& find_itemdesc( unsigned int objtype )
{
  const auto& obj = Core::gamestate.desctable.find( objtype );
  if ( obj != Core::gamestate.desctable.end() )
    return *( obj->second );
  else
    return *( Core::gamestate.empty_itemdesc.get() );
}

const ContainerDesc& find_container_desc( u32 objtype )
{
  const ContainerDesc* cd = static_cast<const ContainerDesc*>( &find_itemdesc( objtype ) );
  passert_r( ( cd->type == ItemDesc::CONTAINERDESC ) || ( cd->type == ItemDesc::SPELLBOOKDESC ),
             "ObjType " + Clib::hexint( objtype ) +
                 " should be defined as container or spellbook, but is not" );
  return *cd;
}

const DoorDesc& fast_find_doordesc( u32 objtype )
{
  const DoorDesc* dd = static_cast<const DoorDesc*>( &find_itemdesc( objtype ) );
  passert( dd->type == ItemDesc::DOORDESC );
  return *dd;
}

const MultiDesc& find_multidesc( u32 objtype )
{
  const MultiDesc* md = static_cast<const MultiDesc*>( &find_itemdesc( objtype ) );
  passert( md->type == ItemDesc::BOATDESC || md->type == ItemDesc::HOUSEDESC );
  return *md;
}

const ItemDesc* CreateItemDescriptor( Bscript::BStruct* itemdesc_struct )
{
  Clib::ConfigElem elem;
  Clib::StubConfigSource stub_source;
  elem.set_source( &stub_source );

  const Bscript::BStruct::Contents& struct_cont = itemdesc_struct->contents();
  Bscript::BStruct::Contents::const_iterator itr;
  for ( itr = struct_cont.begin(); itr != struct_cont.end(); ++itr )
  {
    const std::string& key = ( *itr ).first;
    Bscript::BObjectImp* val_imp = ( *itr ).second->impptr();

    if ( key == "CProps" )
    {
      if ( val_imp->isa( Bscript::BObjectImp::OTDictionary ) )
      {
        Bscript::BDictionary* cpropdict = static_cast<Bscript::BDictionary*>( val_imp );
        const Bscript::BDictionary::Contents& cprop_cont = cpropdict->contents();
        Bscript::BDictionary::Contents::const_iterator ditr;
        for ( ditr = cprop_cont.begin(); ditr != cprop_cont.end(); ++ditr )
        {
          elem.add_prop( "cprop", ( ( *ditr ).first->getStringRep() + "\t" +
                                    ( *ditr ).second->impptr()->pack() ) );
        }
      }
      else
      {
        throw std::runtime_error( "CreateItemDescriptor: CProps must be a dictionary, but is: " +
                                  std::string( val_imp->typeOf() ) );
      }
    }
    else if ( key == "StackingIgnoresCProps" )
    {
      if ( val_imp->isa( Bscript::BObjectImp::OTArray ) )
      {
        OSTRINGSTREAM os;
        // FIXME verify that it's an ObjArray...
        Bscript::ObjArray* ignorecp = static_cast<Bscript::ObjArray*>( itr->second->impptr() );
        const Bscript::ObjArray::Cont& conts = ignorecp->ref_arr;
        Bscript::ObjArray::Cont::const_iterator aitr;
        for ( aitr = conts.begin(); aitr != conts.end(); ++aitr )
        {
          os << ( *aitr ).get()->impptr()->getStringRep() << " ";
        }
        elem.add_prop( key, OSTRINGSTREAM_STR( os ) );
      }
      else
      {
        throw std::runtime_error(
            "CreateItemDescriptor: StackingIgnoresCProps must be an array, but is: " +
            std::string( val_imp->typeOf() ) );
      }
    }
    else if ( key == "Coverage" )  // Dave 7/13 needs to be parsed out into individual lines
    {
      if ( val_imp->isa( Bscript::BObjectImp::OTArray ) )
      {
        // FIXME verify that it's an ObjArray...
        Bscript::ObjArray* coverage = static_cast<Bscript::ObjArray*>( itr->second->impptr() );
        const Bscript::ObjArray::Cont& conts = coverage->ref_arr;
        Bscript::ObjArray::Cont::const_iterator aitr;
        for ( aitr = conts.begin(); aitr != conts.end(); ++aitr )
        {
          OSTRINGSTREAM os;
          os << ( *aitr ).get()->impptr()->getStringRep();
          elem.add_prop( key.c_str(), OSTRINGSTREAM_STR( os ) );
        }
      }
      else
      {
        throw std::runtime_error( "CreateItemDescriptor: Coverage must be an array, but is: " +
                                  std::string( val_imp->typeOf() ) );
      }
    }
    else if ( key == "ObjClass" )
    {
      std::string value = val_imp->getStringRep();
      elem.set_type( value.c_str() );
    }
    else if ( key == "ObjType" )
    {
      std::string value = val_imp->getStringRep();
      elem.set_rest( value.c_str() );
    }
    else if ( Clib::strlowerASCII( key ) == "name" || Clib::strlowerASCII( key ) == "objtypename" ||
              Clib::strlowerASCII( key ) == "oldobjtype" ||
              Clib::strlowerASCII( key ) == "methodscript" ||
              Clib::strlowerASCII( key ) == "weight" )
    {
      // all of these only affect the main descriptor, so they're left out.
      //   name, objtypename, and oldobjtype would try to insert aliases
      //   methodscript would create scripts and such.
      //   weight, for values like 0.1 (came in as 1/10), won't parse correctly.
      continue;
    }
    else
    {
      std::string value = val_imp->getStringRep();
      elem.add_prop( key, std::move( value ) );
    }
  }

  unsigned int objtype = static_cast<unsigned int>( strtoul( elem.rest(), nullptr, 0 ) );
  ItemDesc* id = ItemDesc::create( elem, find_itemdesc( objtype ).pkg );

  Core::gamestate.dynamic_item_descriptors.push_back( id );

  return id;
}


void read_itemdesc_file( const char* filename, Plib::Package* pkg = nullptr )
{
  /*
      if (1)
      {
      ref_ptr<StoredConfigFile> scfg = FindConfigFile( "config/itemdesc.cfg" );
      ConfigFile cf( filename );
      scfg->load( cf );
      }
      */
  Clib::ConfigFile cf( filename,
                       "CONTAINER ITEM DOOR WEAPON ARMOR BOAT HOUSE SPELLBOOK SPELLSCROLL MAP" );

  Clib::ConfigElem elem;
  while ( cf.read( elem ) )
  {
    ItemDesc* descriptor = ItemDesc::create( elem, pkg );


    // string unused_name, unused_value;
    // while (elem.remove_first_prop( &unused_name, &unused_value ))
    //{
    //  elem.warn_with_line( "Property '" + unused_name + "' (value '" + unused_value + "') is
    // unused." );
    //}

    if ( has_itemdesc( descriptor->objtype ) )
    {
      auto objpkg = find_itemdesc( descriptor->objtype ).pkg;
      std::string tmp =
          fmt::format( "Error: Objtype {:#x} is already defined in {}itemdesc.cfg",
                       descriptor->objtype, objpkg == nullptr ? "config/" : objpkg->dir() );
      ERROR_PRINTLN( tmp );

      elem.throw_error(
          fmt::format( "ObjType {:#x} defined more than once.", descriptor->objtype ) );
    }
    Core::gamestate.desctable[descriptor->objtype] = descriptor;

    // just make sure this will work later.
    getgraphic( descriptor->objtype );
  }
}

void load_package_itemdesc( Plib::Package* pkg )
{
  // string filename = pkg->dir() + "itemdesc.cfg";
  std::string filename = GetPackageCfgPath( pkg, "itemdesc.cfg" );
  if ( Clib::FileExists( filename.c_str() ) )
  {
    read_itemdesc_file( filename.c_str(), pkg );
  }
}

void write_objtypes_txt()
{
  std::ofstream ofs( "objtypes.txt" );
  unsigned int last_objtype = 0;
  for ( const auto& elem : Core::gamestate.desctable )
  {
    const ItemDesc* itemdesc = elem.second;
    unsigned int i = elem.first;

    if ( i != last_objtype + 1 )
    {
      unsigned int first = last_objtype + 1;
      unsigned int last = i - 1;
      if ( first == last )
      {
        ofs << "# " << Clib::hexint( first ) << " unused\n";
      }
      else
      {
        ofs << "# " << Clib::hexint( first ) << " - " << Clib::hexint( last ) << " unused\n";
      }
    }

    if ( !Core::gamestate.old_objtype_conversions.count( i ) )
    {
      ofs << Clib::hexint( i ) << " ";
      if ( itemdesc->objtypename.empty() == false )
        ofs << itemdesc->objtypename;
      else
        ofs << "[n/a]";
      if ( itemdesc->pkg )
        ofs << " " << itemdesc->pkg->name();
      ofs << '\n';
    }
    else  // it's converted
    {
      ofs << "# " << Clib::hexint( i ) << " converts to "
          << Clib::hexint( (int)Core::gamestate.old_objtype_conversions[i] ) << '\n';
    }

    last_objtype = i;
  }

  if ( last_objtype != Plib::systemstate.config.max_objtype )
  {
    unsigned int first = last_objtype + 1;
    unsigned int last = Plib::systemstate.config.max_objtype;
    if ( first == last )
    {
      ofs << "# " << Clib::hexint( first ) << " unused\n";
    }
    else
    {
      ofs << "# " << Clib::hexint( first ) << " - " << Clib::hexint( last ) << " unused\n";
    }
  }
}

void load_itemdesc()
{
  //  CreateEmptyStoredConfigFile( "config/itemdesc.cfg" );
  if ( Clib::FileExists( "config/itemdesc.cfg" ) )
    read_itemdesc_file( "config/itemdesc.cfg" );
  //  read_itemdesc_file( "config/wepndesc.cfg" );
  //  read_itemdesc_file( "config/armrdesc.cfg" );
  for ( auto& pkg : Plib::systemstate.packages )
    load_package_itemdesc( pkg );

  write_objtypes_txt();
}

void unload_itemdesc()
{
  for ( auto& elem : Core::gamestate.desctable )
  {
    if ( elem.second != Core::gamestate.empty_itemdesc.get() )
    {
      delete elem.second;
      elem.second = Core::gamestate.empty_itemdesc.get();
    }
  }

  Core::gamestate.objtype_byname.clear();
  Core::gamestate.old_objtype_conversions.clear();
  for ( auto& item : Core::gamestate.dynamic_item_descriptors )
  {
    delete item;
  }
  Core::gamestate.dynamic_item_descriptors.clear();
}

void remove_resources( u32 objtype, u16 /*amount*/ )
{
  const ItemDesc& id = find_itemdesc( objtype );
  for ( const auto& rc : id.resources )
  {
    rc.rd->consume( rc.amount );
  }
}

void return_resources( u32 objtype, u16 /*amount*/ )
{
  // MuadDib Added 03/22/09. This can cause a crash in shutdown with orphaned/leaked items
  // after saving of data files, and clearing all objects. At this stage, there is no need
  // to free and replenish ore/wood/etc resources in the world. o_O
  if ( !Clib::exit_signalled )
  {
    const ItemDesc& id = find_itemdesc( objtype );
    for ( const auto& rc : id.resources )
    {
      rc.rd->produce( rc.amount );
    }
  }
}
}  // namespace Items
}  // namespace Pol
