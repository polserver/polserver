/** @file
 *
 * @par History
 * - 2005/06/15 Shinigami: added CanMove - checks if an NPC can move in given direction
 *                         (IsLegalMove works in a different way and is used for bounding boxes
 * only)
 * - 2006/01/18 Shinigami: set Master first and then start Script in NPC::readNpcProperties
 * - 2006/01/18 Shinigami: added attached_npc_ - to get attached NPC from AI-Script-Process Obj
 * - 2006/09/17 Shinigami: send_event() will return error "Event queue is full, discarding event"
 * - 2009/03/27 MuadDib:   NPC::inform_moved() && NPC::inform_imoved()
 *                         split the left/entered area to fix bug where one would trigger when not
 * enabled.
 * - 2009/07/23 MuadDib:   updates for new Enum::Packet Out ID
 * - 2009/08/25 Shinigami: STLport-5.2.1 fix: params not used
 *                         STLport-5.2.1 fix: init order changed of damaged_sound
 * - 2009/09/15 MuadDib:   Cleanup from registered houses on destroy
 * - 2009/09/18 MuadDib:   Adding save/load of registered house serial
 * - 2009/09/22 MuadDib:   Rewrite for Character/NPC to use ar(), ar_mod(), ar_mod(newvalue)
 * virtuals.
 * - 2009/10/14 Turley:    Added char.deaf() methods & char.deafened member
 * - 2009/10/23 Turley:    fixed OPPONENT_MOVED,LEFTAREA,ENTEREDAREA
 * - 2009/11/16 Turley:    added NpcPropagateEnteredArea()/inform_enteredarea() for event on
 * resurrection
 * - 2010/01/15 Turley:    (Tomi) SaveOnExit as npcdesc entry
 */

#include "npc.h"

#include <stdlib.h>

#include "../../bscript/berror.h"
#include "../../bscript/executor.h"
#include "../../clib/cfgelem.h"
#include "../../clib/fileutil.h"
#include "../../clib/logfacility.h"
#include "../../clib/passert.h"
#include "../../clib/random.h"
#include "../../clib/refptr.h"
#include "../../clib/streamsaver.h"
#include "../baseobject.h"
#include "../dice.h"
#include "../fnsearch.h"
#include "../globals/state.h"
#include "../globals/uvars.h"
#include "../item/armor.h"
#include "../item/weapon.h"
#include "../listenpt.h"
#include "../module/npcmod.h"
#include "../module/uomod.h"
#include "../multi/multi.h"
#include "../npctmpl.h"
#include "../scrdef.h"
#include "../scrsched.h"
#include "../scrstore.h"
#include "../syshookscript.h"
#include "../ufunc.h"
#include "../uobjcnt.h"
#include "../uobject.h"
#include "../uoexec.h"
#include "../uoscrobj.h"
#include "../uworld.h"
#include "attribute.h"
#include "charactr.h"
#include "wornitems.h"


/* An area definition is as follows:
   pt: (x,y)
   rect: [pt-pt]
   area: rect,rect,...
   So, format is: [(x1,y1)-(x2,y2)],[],[],...
   Well, right now, the format is x1 y1 x2 y2 ... (ick)
*/
namespace Pol
{
namespace Mobile
{
unsigned short calc_thru_damage( double damage, unsigned short ar );

NPC::NPC( u32 objtype, const Clib::ConfigElem& elem )
    : Character( objtype, Core::UOBJ_CLASS::CLASS_NPC ),
      // UOBJECT INTERFACE
      // NPC INTERFACE
      npc_ar_( 0 ),
      // MOVEMENT
      run_speed( dexterity() ),
      anchor(),
      // EVENTS
      // SCRIPT
      script( "" ),
      ex( nullptr ),
      // MISC
      damaged_sound( 0 ),
      template_name(),
      master_( nullptr ),
      template_( Core::find_npc_template( elem ) )
{
  connected( true );
  logged_in( true );
  use_adjustments( true );
  ++Core::stateManager.uobjcount.npc_count;
}

NPC::~NPC()
{
  stop_scripts();
  --Core::stateManager.uobjcount.npc_count;
}

void NPC::stop_scripts()
{
  if ( ex != nullptr )
  {
    // this will force the execution engine to stop running this script immediately
    // dont delete the executor here, since it could currently run
    ex->seterror( true );
    ex->revive();
    if ( ex->in_debugger_holdlist() )
      ex->revive_debugged();
  }
}

void NPC::destroy()
{
  // stop_scripts();
  wornitems->destroy_contents();
  if ( registered_multi > 0 )
  {
    Multi::UMulti* multi = Core::system_find_multi( registered_multi );
    if ( multi != nullptr )
    {
      multi->unregister_object( (UObject*)this );
    }
    registered_multi = 0;
  }
  base::destroy();
}

const char* NPC::classname() const
{
  return "NPC";
}

bool NPC::anchor_allows_move( Core::UFACING fdir ) const
{
  auto newpos = pos().move( fdir );

  if ( anchor.enabled && !warmode() )
  {
    unsigned short curdist = distance_to( anchor.pos );
    unsigned short newdist = newpos.xy().pol_distance( anchor.pos );
    if ( newdist > curdist )  // if we're moving further away, see if we can
    {
      if ( newdist > anchor.dstart )
      {
        int perc = 100 - ( newdist - anchor.dstart ) * anchor.psub;
        if ( perc < 5 )
          perc = 5;
        if ( Clib::random_int( 99 ) > perc )
          return false;
      }
    }
  }
  return true;
}

bool NPC::could_move( Core::UFACING fdir ) const
{
  short newz;
  Multi::UMulti* supporting_multi;
  Items::Item* walkon_item;
  // Check for diagonal move - use Nandos change from charactr.cpp -- OWHorus (2011-04-26)
  if ( fdir & 1 )  // check if diagonal movement is allowed -- Nando (2009-02-26)
  {
    u8 tmp_facing = ( fdir + 1 ) & 0x7;
    auto tmp_pos = pos().move( static_cast<Core::UFACING>( tmp_facing ) );

    // needs to save because if only one direction is blocked, it shouldn't block ;)
    short current_boost = gradual_boost;
    bool walk1 = realm()->walkheight( this, tmp_pos.xy(), tmp_pos.z(), &newz, &supporting_multi,
                                      &walkon_item, &current_boost );

    tmp_facing = ( fdir - 1 ) & 0x7;
    tmp_pos = pos().move( static_cast<Core::UFACING>( tmp_facing ) );
    current_boost = gradual_boost;
    if ( !walk1 && !realm()->walkheight( this, tmp_pos.xy(), tmp_pos.z(), &newz, &supporting_multi,
                                         &walkon_item, &current_boost ) )
      return false;
  }
  auto new_pos = pos().move( fdir );
  short current_boost = gradual_boost;
  return realm()->walkheight( this, new_pos.xy(), new_pos.z(), &newz, &supporting_multi,
                              &walkon_item, &current_boost ) &&
         !npc_path_blocked( fdir ) && anchor_allows_move( fdir );
}

bool NPC::npc_path_blocked( Core::UFACING fdir ) const
{
  if ( can_freemove() )
    return false;

  Core::MOVEBLOCKMODE moveBlockMode = Core::settingsManager.ssopt.mobiles_block_npc_movement;

  if ( moveBlockMode == Core::MOVEBLOCKMODE_NONE )
    return false;
  if ( !this->master() && moveBlockMode == Core::MOVEBLOCKMODE_SAME_MASTER )
    return false;

  auto new_pos = pos().move( fdir );

  Core::Pos2d gridp = Core::zone_convert( new_pos );

  if ( moveBlockMode == Core::MOVEBLOCKMODE_ALL )
  {
    for ( const auto& chr : realm()->getzone_grid( gridp ).characters )
    {
      // First check if there really is a character blocking
      if ( chr->pos2d() == new_pos.xy() && chr->z() >= z() - 10 && chr->z() <= z() + 10 )
      {
        if ( !chr->dead() && is_visible_to_me( chr, /*check_range*/ false ) )
          return true;
      }
    }
  }

  for ( const auto& chr : realm()->getzone_grid( gridp ).npcs )
  {
    // First check if there really is a character blocking
    if ( chr->pos2d() == new_pos.xy() && chr->z() >= z() - 10 && chr->z() <= z() + 10 )
    {
      // Do not allow npcs of same master running on top of each other
      if ( moveBlockMode == Core::MOVEBLOCKMODE_SAME_MASTER )
      {
        NPC* npc = static_cast<NPC*>( chr );
        if ( npc->master() && this->master() == npc->master() && !npc->dead() &&
             is_visible_to_me( npc, /*check_range*/ false ) )
          return true;
      }
      else
      {
        if ( !chr->dead() && is_visible_to_me( chr, /*check_range*/ false ) )
          return true;
      }
    }
  }
  return false;
}

void NPC::printOn( Clib::StreamWriter& sw ) const
{
  sw.begin( classname(), template_name.get() );
  printProperties( sw );
  sw.end();
}

void NPC::printSelfOn( Clib::StreamWriter& sw ) const
{
  printOn( sw );
}

void NPC::printProperties( Clib::StreamWriter& sw ) const
{
  base::printProperties( sw );

  if ( npc_ar_ )
    sw.add( "AR", npc_ar_ );

  if ( !script.get().empty() )
    sw.add( "script", script.get() );

  if ( master_.get() != nullptr )
    sw.add( "master", master_->serial );

  if ( has_speech_color() )
    sw.add( "SpeechColor", speech_color() );

  if ( has_speech_font() )
    sw.add( "SpeechFont", speech_font() );

  if ( run_speed != dexterity() )
    sw.add( "RunSpeed", run_speed );

  if ( use_adjustments() != true )
    sw.add( "UseAdjustments", use_adjustments() );


  if ( has_orig_fire_resist() )
    sw.add( "FireResist", orig_fire_resist() );
  if ( has_orig_cold_resist() )
    sw.add( "ColdResist", orig_cold_resist() );
  if ( has_orig_energy_resist() )
    sw.add( "EnergyResist", orig_energy_resist() );
  if ( has_orig_poison_resist() )
    sw.add( "PoisonResist", orig_poison_resist() );
  if ( has_orig_physical_resist() )
    sw.add( "PhysicalResist", orig_physical_resist() );

  if ( has_orig_fire_damage() )
    sw.add( "FireDamage", orig_fire_damage() );
  if ( has_orig_cold_damage() )
    sw.add( "ColdDamage", orig_cold_damage() );
  if ( has_orig_energy_damage() )
    sw.add( "EnergyDamage", orig_energy_damage() );
  if ( has_orig_poison_damage() )
    sw.add( "PoisonDamage", orig_poison_damage() );
  if ( has_orig_physical_damage() )
    sw.add( "PhysicalDamage", orig_physical_damage() );
  if ( has_orig_lower_reagent_cost() )
    sw.add( "LowerReagentCost", orig_lower_reagent_cost() );
  if ( has_orig_spell_damage_increase() )
    sw.add( "SpellDamageIncrease", orig_spell_damage_increase() );
  if ( has_orig_faster_casting() )
    sw.add( "FasterCasting", orig_faster_casting() );
  if ( has_orig_faster_cast_recovery() )
    sw.add( "FasterCastRecovery", orig_faster_cast_recovery() );
  if ( has_orig_defence_increase() )
    sw.add( "DefenceIncrease", orig_defence_increase() );
  if ( has_orig_defence_increase_cap() )
    sw.add( "DefenceIncreaseCap", orig_defence_increase_cap() );
  if ( has_orig_lower_mana_cost() )
    sw.add( "LowerManaCost", orig_lower_mana_cost() );
  if ( has_orig_hit_chance() )
    sw.add( "HitChance", orig_hit_chance() );
  if ( has_orig_fire_resist_cap() )
    sw.add( "FireResistCap", orig_fire_resist_cap() );
  if ( has_orig_cold_resist_cap() )
    sw.add( "ColdResistCap", orig_cold_resist_cap() );
  if ( has_orig_energy_resist_cap() )
    sw.add( "EnergyResistCap", orig_energy_resist_cap() );
  if ( has_orig_physical_resist_cap() )
    sw.add( "PhysicalResistCap", orig_physical_resist_cap() );
  if ( has_orig_poison_resist_cap() )
    sw.add( "PoisonResistCap", orig_poison_resist_cap() );
  if ( has_orig_luck() )
    sw.add( "Luck", orig_luck() );
  if ( has_swing_speed_increase() )
    sw.add( "SwingSpeedIncrease", orig_swing_speed_increase() );
  if ( has_min_attack_range_increase() )
    sw.add( "MinAttackRangeIncrease", orig_min_attack_range_increase() );
  if ( has_max_attack_range_increase() )
    sw.add( "MaxAttackRangeIncrease", orig_max_attack_range_increase() );
  if ( no_drop_exception() )
    sw.add( "NoDropException", no_drop_exception() );
}

void NPC::printDebugProperties( Clib::StreamWriter& sw ) const
{
  base::printDebugProperties( sw );
  sw.comment( "template: {}", template_->name );
  if ( anchor.enabled )
  {
    sw.comment( "anchor: x={} y={} dstart={} psub={}", anchor.pos.x(), anchor.pos.y(),
                anchor.dstart, anchor.psub );
  }
}

void NPC::readNpcProperties( Clib::ConfigElem& elem )
{
  Items::UWeapon* wpn = static_cast<Items::UWeapon*>(
      Items::find_intrinsic_equipment( elem.rest(), Core::LAYER_HAND1 ) );
  if ( wpn == nullptr )
    wpn = Items::create_intrinsic_weapon_from_npctemplate( elem, template_->pkg );
  if ( wpn != nullptr )
    weapon = wpn;

  Items::UArmor* sld = static_cast<Items::UArmor*>(
      Items::find_intrinsic_equipment( elem.rest(), Core::LAYER_HAND2 ) );
  if ( sld == nullptr )
    sld = Items::create_intrinsic_shield_from_npctemplate( elem, template_->pkg );
  if ( sld != nullptr )
    shield = sld;

  // Load the base, equiping items etc will refresh_ar() to update for reals.
  loadEquipablePropertiesNPC( elem );

  // dave 3/19/3, read templatename only if empty
  if ( template_name.get().empty() )
  {
    template_name = elem.rest();

    if ( template_name.get().empty() )
    {
      std::string tmp;
      if ( getprop( "template", tmp ) )
      {
        template_name = tmp.c_str() + 1;
      }
    }
  }

  unsigned int master_serial;
  if ( elem.remove_prop( "MASTER", &master_serial ) )
  {
    Character* chr = Core::system_find_mobile( master_serial );
    if ( chr != nullptr )
      master_.set( chr );
  }

  script = elem.remove_string( "script", "" );
  if ( !script.get().empty() )
    start_script();

  speech_color( elem.remove_ushort( "SpeechColor", Plib::DEFAULT_TEXT_COLOR ) );
  speech_font( elem.remove_ushort( "SpeechFont", Plib::DEFAULT_TEXT_FONT ) );
  saveonexit( elem.remove_bool( "SaveOnExit", true ) );

  mob_flags_.change( MOB_FLAGS::USE_ADJUSTMENTS, elem.remove_bool( "UseAdjustments", true ) );
  run_speed = elem.remove_ushort( "RunSpeed", dexterity() );

  damaged_sound = elem.remove_ushort( "DamagedSound", 0 );
  no_drop_exception( elem.remove_bool( "NoDropException", false ) );
}

void NPC::loadEquipablePropertiesNPC( Clib::ConfigElem& elem )
{
  // for ar and elemental damage/resist the mod values are loaded before in character code!
  auto diceValue = []( const std::string& dicestr, s16* value ) -> bool
  {
    Core::Dice dice;
    std::string errmsg;
    if ( !dice.load( dicestr.c_str(), &errmsg ) )
      *value = Clib::clamp_convert<s16>( atoi( dicestr.c_str() ) );
    else
      *value = Clib::clamp_convert<s16>( dice.roll() );
    return *value != 0;
  };
  auto apply = []( Core::ValueModPack v, s16 value ) -> Core::ValueModPack
  { return v.addToValue( value ); };

  std::string tmp;
  s16 value;
  if ( elem.remove_prop( "AR", &tmp ) && diceValue( tmp, &value ) )
    npc_ar_ = value;
  if ( elem.remove_prop( "LOWERREAGENTCOST", &tmp ) && diceValue( tmp, &value ) )
  {
    lower_reagent_cost( apply( lower_reagent_cost(), value ) );
    orig_lower_reagent_cost( value );
  }
  if ( elem.remove_prop( "SPELLDAMAGEINCREASE", &tmp ) && diceValue( tmp, &value ) )
  {
    spell_damage_increase( apply( spell_damage_increase(), value ) );
    orig_spell_damage_increase( value );
  }
  if ( elem.remove_prop( "FASTERCASTING", &tmp ) && diceValue( tmp, &value ) )
  {
    faster_casting( apply( faster_casting(), value ) );
    orig_faster_casting( value );
  }
  if ( elem.remove_prop( "FASTERCASTRECOVERY", &tmp ) && diceValue( tmp, &value ) )
  {
    faster_cast_recovery( apply( faster_cast_recovery(), value ) );
    orig_faster_cast_recovery( value );
  }
  if ( elem.remove_prop( "DEFENCEINCREASE", &tmp ) && diceValue( tmp, &value ) )
  {
    defence_increase( apply( defence_increase(), value ) );
    orig_defence_increase( value );
  }
  if ( elem.remove_prop( "DEFENCEINCREASECAP", &tmp ) && diceValue( tmp, &value ) )
  {
    defence_increase_cap( apply( defence_increase_cap(), value ) );
    orig_defence_increase_cap( value );
  }
  if ( elem.remove_prop( "LOWERMANACOST", &tmp ) && diceValue( tmp, &value ) )
  {
    lower_mana_cost( apply( lower_mana_cost(), value ) );
    orig_lower_mana_cost( value );
  }
  if ( elem.remove_prop( "HITCHANCE", &tmp ) && diceValue( tmp, &value ) )
  {
    hit_chance( apply( hit_chance(), value ) );
    orig_hit_chance( value );
  }
  if ( elem.remove_prop( "FIRERESISTCAP", &tmp ) && diceValue( tmp, &value ) )
  {
    fire_resist_cap( apply( fire_resist_cap(), value ) );
    orig_fire_resist_cap( value );
  }
  if ( elem.remove_prop( "COLDRESISTCAP", &tmp ) && diceValue( tmp, &value ) )
  {
    cold_resist_cap( apply( cold_resist_cap(), value ) );
    orig_cold_resist_cap( value );
  }
  if ( elem.remove_prop( "ENERGYRESISTCAP", &tmp ) && diceValue( tmp, &value ) )
  {
    energy_resist_cap( apply( energy_resist_cap(), value ) );
    orig_energy_resist_cap( value );
  }
  if ( elem.remove_prop( "PHYSICALRESISTCAP", &tmp ) && diceValue( tmp, &value ) )
  {
    physical_resist_cap( apply( physical_resist_cap(), value ) );
    orig_physical_resist_cap( value );
  }
  if ( elem.remove_prop( "POISONRESISTCAP", &tmp ) && diceValue( tmp, &value ) )
  {
    poison_resist_cap( apply( poison_resist_cap(), value ) );
    orig_poison_resist_cap( value );
  }
  if ( elem.remove_prop( "LUCK", &tmp ) && diceValue( tmp, &value ) )
  {
    luck( apply( luck(), value ) );
    orig_luck( value );
  }
  if ( elem.remove_prop( "SWINGSPEEDINCREASE", &tmp ) && diceValue( tmp, &value ) )
  {
    swing_speed_increase( apply( swing_speed_increase(), value ) );
    orig_swing_speed_increase( value );
  }
  if ( elem.remove_prop( "MINATTACKRANGEINCREASE", &tmp ) && diceValue( tmp, &value ) )
  {
    min_attack_range_increase( apply( min_attack_range_increase(), value ) );
    orig_min_attack_range_increase( value );
  }
  if ( elem.remove_prop( "MAXATTACKRANGEINCREASE", &tmp ) && diceValue( tmp, &value ) )
  {
    max_attack_range_increase( apply( max_attack_range_increase(), value ) );
    orig_max_attack_range_increase( value );
  }

  // elemental start
  // first apply template value as value and if mod or value exist sum them
  if ( elem.remove_prop( "FIRERESIST", &tmp ) && diceValue( tmp, &value ) )
  {
    fire_resist( apply( fire_resist(), value ) );
    orig_fire_resist( value );
  }
  if ( elem.remove_prop( "COLDRESIST", &tmp ) && diceValue( tmp, &value ) )
  {
    cold_resist( apply( cold_resist(), value ) );
    orig_cold_resist( value );
  }
  if ( elem.remove_prop( "ENERGYRESIST", &tmp ) && diceValue( tmp, &value ) )
  {
    energy_resist( apply( energy_resist(), value ) );
    orig_energy_resist( value );
  }
  if ( elem.remove_prop( "POISONRESIST", &tmp ) && diceValue( tmp, &value ) )
  {
    poison_resist( apply( poison_resist(), value ) );
    orig_poison_resist( value );
  }
  if ( elem.remove_prop( "PHYSICALRESIST", &tmp ) && diceValue( tmp, &value ) )
  {
    physical_resist( apply( physical_resist(), value ) );
    orig_physical_resist( value );
  }

  if ( elem.remove_prop( "FIREDAMAGE", &tmp ) && diceValue( tmp, &value ) )
  {
    fire_damage( apply( fire_damage(), value ) );
    orig_fire_damage( value );
  }
  if ( elem.remove_prop( "COLDDAMAGE", &tmp ) && diceValue( tmp, &value ) )
  {
    cold_damage( apply( cold_damage(), value ) );
    orig_cold_damage( value );
  }
  if ( elem.remove_prop( "ENERGYDAMAGE", &tmp ) && diceValue( tmp, &value ) )
  {
    energy_damage( apply( energy_damage(), value ) );
    orig_energy_damage( value );
  }
  if ( elem.remove_prop( "POISONDAMAGE", &tmp ) && diceValue( tmp, &value ) )
  {
    poison_damage( apply( poison_damage(), value ) );
    orig_poison_damage( value );
  }
  if ( elem.remove_prop( "PHYSICALDAMAGE", &tmp ) && diceValue( tmp, &value ) )
  {
    physical_damage( apply( physical_damage(), value ) );
    orig_physical_damage( value );
  }
}

void NPC::readProperties( Clib::ConfigElem& elem )
{
  // 3/18/3 dave copied this npctemplate code from readNpcProperties, because base::readProperties
  // will call the exported vital functions before npctemplate is set (distro uses npctemplate in
  // the exported funcs).
  template_name = elem.rest();

  if ( template_name.get().empty() )
  {
    std::string tmp;
    if ( getprop( "template", tmp ) )
    {
      template_name = tmp.c_str() + 1;
    }
  }
  base::readProperties( elem );
  readNpcProperties( elem );
}

void NPC::readNewNpcAttributes( Clib::ConfigElem& elem )
{
  std::string diestring;
  Core::Dice dice;
  std::string errmsg;

  for ( Attribute* pAttr : Core::gamestate.attributes )
  {
    AttributeValue& av = attribute( pAttr->attrid );
    for ( unsigned i = 0; i < pAttr->aliases.size(); ++i )
    {
      if ( elem.remove_prop( pAttr->aliases[i].c_str(), &diestring ) )
      {
        if ( !dice.load( diestring.c_str(), &errmsg ) )
        {
          elem.throw_error( "Error reading Attribute " + pAttr->name + ": " + errmsg );
        }
        int base = dice.roll() * 10;
        if ( base > static_cast<int>( ATTRIBUTE_MAX_BASE ) )
          base = ATTRIBUTE_MAX_BASE;

        av.base( static_cast<unsigned short>( base ) );

        break;
      }
    }
  }
}

void NPC::readPropertiesForNewNPC( Clib::ConfigElem& elem )
{
  readCommonProperties( elem );
  readNewNpcAttributes( elem );
  readNpcProperties( elem );
  calc_vital_stuff();
  set_vitals_to_maximum();

  //    readNpcProperties( elem );
}

void NPC::restart_script()
{
  if ( ex != nullptr )
  {
    ex->seterror( true );
    // A Sleeping script would otherwise sit and wait until it wakes up to be killed.
    ex->revive();
    if ( ex->in_debugger_holdlist() )
      ex->revive_debugged();
    ex = nullptr;
    // when the NPC executor module destructs, it checks this NPC to see if it points
    // back at it.  If not, it leaves us alone.
  }
  if ( !script.get().empty() )
    start_script();
}

void NPC::on_death( Items::Item* corpse )
{
  // base::on_death intentionally not called
  send_remove_character_to_nearby( this );


  corpse->setprop( "npctemplate", "s" + template_name.get() );
  if ( Clib::FileExists( "scripts/misc/death.ecl" ) )
    Core::start_script( "misc/death", new Module::EItemRefObjImp( corpse ) );

  ClrCharacterWorldPosition( this, Realms::WorldChangeReason::NpcDeath );
  if ( ex != nullptr )
  {
    // this will force the execution engine to stop running this script immediately
    ex->seterror( true );
    ex->revive();
    if ( ex->in_debugger_holdlist() )
      ex->revive_debugged();
  }

  destroy();
}


void NPC::start_script()
{
  passert( ex == nullptr );
  passert( !script.get().empty() );
  Core::ScriptDef sd( script, template_->pkg, "scripts/ai/" );
  // Log( "NPC script starting: %s\n", sd.name().c_str() );

  ref_ptr<Bscript::EScriptProgram> prog = Core::find_script2( sd );
  // find_script( "ai/" + script );

  if ( prog.get() == nullptr )
  {
    ERROR_PRINTLN( "Unable to read script {} for NPC {}({:#x})", sd.name(), name(), serial );
    throw std::runtime_error( "Error loading NPCs" );
  }

  ex = Core::create_script_executor();
  ex->addModule( new Module::NPCExecutorModule( *ex, *this ) );
  Module::UOExecutorModule* uoemod = new Module::UOExecutorModule( *ex );
  ex->addModule( uoemod );
  if ( ex->setProgram( prog.get() ) == false )
  {
    ERROR_PRINTLN( "There was an error running script {} for NPC {}({:#x})", script.get(), name(),
                   serial );
    throw std::runtime_error( "Error loading NPCs" );
  }

  uoemod->attached_npc_ = this;

  schedule_executor( ex );
}


bool NPC::can_be_renamed_by( const Character* chr ) const
{
  return ( master_.get() == chr );
}

bool NPC::can_be_clothed_by( const Character* chr ) const
{
  return has_paperdoll() && Core::settingsManager.ssopt.master_can_clothe_npcs &&
         master_.get() == chr;
}

void NPC::on_pc_spoke( Character* src_chr, const std::string& speech, u8 texttype,
                       const std::string& lang, Bscript::ObjArray* speechtokens )
{
  if ( ex == nullptr )
    return;

  if ( Core::settingsManager.ssopt.seperate_speechtoken )
  {
    if ( speechtokens != nullptr && ( ( ex->eventmask & Core::EVID_TOKEN_SPOKE ) == 0 ) )
      return;
    else if ( speechtokens == nullptr && ( ( ex->eventmask & Core::EVID_SPOKE ) == 0 ) )
      return;
  }
  if ( ( ( ex->eventmask & Core::EVID_SPOKE ) || ( ex->eventmask & Core::EVID_TOKEN_SPOKE ) ) &&
       in_range( src_chr, ex->speech_size ) && !deafened() )
  {
    if ( ( !Core::settingsManager.ssopt.event_visibility_core_checks ) ||
         is_visible_to_me( src_chr, /*check_range*/ false ) )
    {
      ex->signal_event( new Module::SpeechEvent(
          src_chr, speech, Core::ListenPoint::TextTypeToString( texttype ), lang, speechtokens ) );
    }
  }
}

void NPC::on_ghost_pc_spoke( Character* src_chr, const std::string& speech, u8 texttype,
                             const std::string& lang, Bscript::ObjArray* speechtokens )
{
  if ( ex == nullptr )
    return;

  if ( Core::settingsManager.ssopt.seperate_speechtoken )
  {
    if ( speechtokens != nullptr && ( ( ex->eventmask & Core::EVID_TOKEN_GHOST_SPOKE ) == 0 ) )
      return;
    else if ( speechtokens == nullptr && ( ( ex->eventmask & Core::EVID_GHOST_SPEECH ) == 0 ) )
      return;
  }
  if ( ( ( ex->eventmask & Core::EVID_GHOST_SPEECH ) ||
         ( ex->eventmask & Core::EVID_TOKEN_GHOST_SPOKE ) ) &&
       in_range( src_chr, ex->speech_size ) && !deafened() )
  {
    if ( ( !Core::settingsManager.ssopt.event_visibility_core_checks ) ||
         is_visible_to_me( src_chr, /*check_range*/ false ) )
    {
      ex->signal_event( new Module::SpeechEvent(
          src_chr, speech, Core::ListenPoint::TextTypeToString( texttype ), lang, speechtokens ) );
    }
  }
}

void NPC::inform_engaged( Character* engaged )
{
  // someone has targetted us. Create an event if appropriate.
  if ( ex != nullptr )
  {
    if ( ex->eventmask & Core::EVID_ENGAGED )
    {
      ex->signal_event( new Module::EngageEvent( engaged ) );
    }
  }
  // Note, we don't do the base class thing, 'cause we have no client.
}

void NPC::inform_disengaged( Character* disengaged )
{
  // someone has targetted us. Create an event if appropriate.
  if ( ex != nullptr )
  {
    if ( ex->eventmask & Core::EVID_DISENGAGED )
    {
      ex->signal_event( new Module::DisengageEvent( disengaged ) );
    }
  }
  // Note, we don't do the base class thing, 'cause we have no client.
}

void NPC::inform_criminal( Character* thecriminal )
{
  if ( ex != nullptr )
  {
    if ( ( ex->eventmask & ( Core::EVID_GONE_CRIMINAL ) ) &&
         in_range( thecriminal, ex->area_size ) )
    {
      if ( ( !Core::settingsManager.ssopt.event_visibility_core_checks ) ||
           is_visible_to_me( thecriminal, /*check_range*/ false ) )
        ex->signal_event( new Module::SourcedEvent( Core::EVID_GONE_CRIMINAL, thecriminal ) );
    }
  }
}

void NPC::inform_leftarea( Character* wholeft )
{
  if ( ex != nullptr )
  {
    if ( ( ex->eventmask & ( Core::EVID_LEFTAREA ) ) && can_accept_area_event_by( wholeft ) )
    {
      if ( in_range( wholeft, ex->area_size ) )
      {
        if ( ( !Core::settingsManager.ssopt.event_visibility_core_checks ) ||
             is_visible_to_me( wholeft, /*check_range*/ false ) )
          ex->signal_event( new Module::SourcedEvent( Core::EVID_LEFTAREA, wholeft ) );
      }
    }
  }
}

void NPC::inform_enteredarea( Character* whoentered )
{
  if ( ex != nullptr )
  {
    if ( ( ex->eventmask & ( Core::EVID_ENTEREDAREA ) ) && can_accept_area_event_by( whoentered ) )
    {
      if ( in_range( whoentered, ex->area_size ) )
      {
        if ( ( !Core::settingsManager.ssopt.event_visibility_core_checks ) ||
             is_visible_to_me( whoentered, /*check_range*/ false ) )
          ex->signal_event( new Module::SourcedEvent( Core::EVID_ENTEREDAREA, whoentered ) );
      }
    }
  }
}

void NPC::inform_moved( Character* moved )
{
  if ( ex == nullptr )
    return;
  passert( moved != nullptr );
  if ( ( ex->eventmask & ( Core::EVID_ENTEREDAREA | Core::EVID_LEFTAREA ) ) &&
       can_accept_area_event_by( moved ) )
  {
    bool are_inrange = in_range( moved, ex->area_size );
    bool were_inrange = in_range( moved->lastpos, ex->area_size );

    if ( ( !Core::settingsManager.ssopt.event_visibility_core_checks ) ||
         is_visible_to_me( moved ) )
    {
      if ( are_inrange && !were_inrange && ( ex->eventmask & ( Core::EVID_ENTEREDAREA ) ) )
      {
        ex->signal_event( new Module::SourcedEvent( Core::EVID_ENTEREDAREA, moved ) );
        return;
      }
      else if ( !are_inrange && were_inrange && ( ex->eventmask & ( Core::EVID_LEFTAREA ) ) )
      {
        ex->signal_event( new Module::SourcedEvent( Core::EVID_LEFTAREA, moved ) );
        return;
      }
    }
  }

  // only send moved event if left/enteredarea wasnt send
  if ( ( moved == opponent_ ) && ( ex->eventmask & ( Core::EVID_OPPONENT_MOVED ) ) )
  {
    if ( ( !Core::settingsManager.ssopt.event_visibility_core_checks ) ||
         is_visible_to_me( moved ) )
      ex->signal_event( new Module::SourcedEvent( Core::EVID_OPPONENT_MOVED, moved ) );
  }
}

//
// This NPC moved.  Tell him about other mobiles that have "entered" his area
// (through his own movement)
//
void NPC::inform_imoved( Character* chr )
{
  if ( ex == nullptr )
    return;
  passert( chr != nullptr );
  if ( ex->eventmask & ( Core::EVID_ENTEREDAREA | Core::EVID_LEFTAREA ) &&
       can_accept_area_event_by( chr ) )
  {
    bool are_inrange = in_range( chr, ex->area_size );
    bool were_inrange = lastpos.in_range( chr->pos(), ex->area_size );

    if ( ( !Core::settingsManager.ssopt.event_visibility_core_checks ) || is_visible_to_me( chr ) )
    {
      if ( are_inrange && !were_inrange && ( ex->eventmask & ( Core::EVID_ENTEREDAREA ) ) )
        ex->signal_event( new Module::SourcedEvent( Core::EVID_ENTEREDAREA, chr ) );
      else if ( !are_inrange && were_inrange && ( ex->eventmask & ( Core::EVID_LEFTAREA ) ) )
        ex->signal_event( new Module::SourcedEvent( Core::EVID_LEFTAREA, chr ) );
    }
  }
}

bool NPC::can_accept_area_event_by( const Character* who ) const
{
  if ( !ex->area_mask )
    return true;

  const bool isNPC = who->isa( Core::UOBJ_CLASS::CLASS_NPC );
  if ( ( ex->area_mask & Core::EVMASK_ONLY_PC ) && isNPC )
    return false;

  if ( ( ex->area_mask & Core::EVMASK_ONLY_NPC ) && !isNPC )
    return false;

  return true;
}

bool NPC::can_accept_event( Core::EVENTID eventid )
{
  if ( ex == nullptr )
    return false;
  if ( ex->eventmask & eventid )
    return true;
  else
    return false;
}

bool NPC::send_event( Bscript::BObjectImp* event )
{
  if ( ex != nullptr )
  {
    if ( ex->signal_event( event ) )
      return true;
  }
  else
  {
    // There's no executor, so we must delete it ourselves.
    Bscript::BObject bo( event );
  }
  return false;
}

Bscript::BObjectImp* NPC::send_event_script( Bscript::BObjectImp* event )
{
  if ( ex != nullptr )
  {
    if ( ex->signal_event( event ) )
      return new Bscript::BLong( 1 );
    else
    {
      return new Bscript::BError( "Event queue is full, discarding event" );
    }
  }
  else
  {
    // Because there is no control script, we must delete it ourselves.
    Bscript::BObject bo( event );
    return new Bscript::BError( "That NPC doesn't have a control script" );
  }
}

void NPC::apply_raw_damage_hundredths( unsigned int damage, Character* source, bool userepsys,
                                       bool send_damage_packet )
{
  if ( ex != nullptr )
  {
    if ( ex->eventmask & Core::EVID_DAMAGED )
    {
      ex->signal_event(
          new Module::DamageEvent( source, static_cast<unsigned short>( damage / 100 ) ) );
    }
  }

  base::apply_raw_damage_hundredths( damage, source, userepsys, send_damage_packet );
}

// keep this in sync with Character::armor_absorb_damage
double NPC::armor_absorb_damage( double damage )
{
  if ( !npc_ar_ )
  {
    return base::armor_absorb_damage( damage );
  }
  else
  {
    int blocked = npc_ar_ + ar_mod();
    if ( blocked < 0 )
      blocked = 0;
    int absorbed = blocked / 2;

    blocked -= absorbed;
    absorbed += Clib::random_int( blocked );
    if ( Core::settingsManager.watch.combat )
      INFO_PRINTLN( "{} hits absorbed by NPC armor.", absorbed );
    damage -= absorbed;
    if ( damage < 0 )
      damage = 0;
  }
  return damage;
}

void NPC::get_hitscript_params( double damage, Items::UArmor** parmor, unsigned short* rawdamage )
{
  if ( !npc_ar_ )
  {
    base::get_hitscript_params( damage, parmor, rawdamage );
  }
  else
  {
    *rawdamage = static_cast<unsigned short>( calc_thru_damage( damage, npc_ar_ + ar_mod() ) );
  }
}

Items::UWeapon* NPC::intrinsic_weapon()
{
  if ( template_->intrinsic_weapon )
    return template_->intrinsic_weapon;
  else
    return Core::gamestate.wrestling_weapon;
}

void NPC::refresh_ar()
{
  // Check if the NPC is using intrinsic armor. If yes, use it and set armor to 0
  // regardless of what is equipped. If not, just calculate like a PC.
  // TODO: intrinsic and equipped armor should sum, but many code will need to be
  //       modified for this to work.
  // NOTE: Keep in mind an armor could have just been destroyed when calling this
  //       and it could've been the last piece of armor equipped, so checking for
  //       an equipped armor will not be wise. Intrinsic armor is assumed to never
  //       change instead.
  if ( npc_ar_ )
  {
    for ( unsigned zone = 0; zone < Core::gamestate.armorzones.size(); ++zone )
      armor_[zone] = nullptr;
    ar_ = 0;
    resetEquipablePropertiesNPC();
  }
  else
  {
    base::refresh_ar();
  }
}

void NPC::resetEquipablePropertiesNPC()
{
  if ( has_fire_resist() || has_orig_fire_resist() )
    fire_resist( fire_resist().setAsValue( orig_fire_resist() ) );
  if ( has_cold_resist() || has_orig_cold_resist() )
    cold_resist( cold_resist().setAsValue( orig_cold_resist() ) );
  if ( has_energy_resist() || has_orig_energy_resist() )
    energy_resist( energy_resist().setAsValue( orig_energy_resist() ) );
  if ( has_poison_resist() || has_orig_poison_resist() )
    poison_resist( poison_resist().setAsValue( orig_poison_resist() ) );
  if ( has_physical_resist() || has_orig_physical_resist() )
    physical_resist( physical_resist().setAsValue( orig_physical_resist() ) );

  if ( has_fire_damage() || has_orig_fire_damage() )
    fire_damage( fire_damage().setAsValue( orig_fire_damage() ) );
  if ( has_cold_damage() || has_orig_cold_damage() )
    cold_damage( cold_damage().setAsValue( orig_cold_damage() ) );
  if ( has_energy_damage() || has_orig_energy_damage() )
    energy_damage( energy_damage().setAsValue( orig_energy_damage() ) );
  if ( has_poison_damage() || has_orig_poison_damage() )
    poison_damage( poison_damage().setAsValue( orig_poison_damage() ) );
  if ( has_physical_damage() || has_orig_physical_damage() )
    physical_damage( physical_damage().setAsValue( orig_physical_damage() ) );

  if ( has_lower_reagent_cost() || has_orig_lower_reagent_cost() )
    lower_reagent_cost( lower_reagent_cost().setAsValue( orig_lower_reagent_cost() ) );
  if ( has_spell_damage_increase() || has_orig_spell_damage_increase() )
    spell_damage_increase( spell_damage_increase().setAsValue( orig_spell_damage_increase() ) );
  if ( has_faster_casting() || has_orig_faster_casting() )
    faster_casting( faster_casting().setAsValue( orig_faster_casting() ) );
  if ( has_faster_cast_recovery() || has_orig_faster_cast_recovery() )
    faster_cast_recovery( faster_cast_recovery().setAsValue( orig_faster_cast_recovery() ) );
  if ( has_defence_increase() || has_orig_defence_increase() )
    defence_increase( defence_increase().setAsValue( orig_defence_increase() ) );
  if ( has_defence_increase_cap() || has_orig_defence_increase_cap() )
    defence_increase_cap( defence_increase_cap().setAsValue( orig_defence_increase_cap() ) );
  if ( has_lower_mana_cost() || has_orig_lower_mana_cost() )
    lower_mana_cost( lower_mana_cost().setAsValue( orig_lower_mana_cost() ) );
  if ( has_hit_chance() || has_orig_hit_chance() )
    hit_chance( hit_chance().setAsValue( orig_hit_chance() ) );
  if ( has_fire_resist_cap() || has_orig_fire_resist_cap() )
    fire_resist_cap( fire_resist_cap().setAsValue( orig_fire_resist_cap() ) );
  if ( has_cold_resist_cap() || has_orig_cold_resist_cap() )
    cold_resist_cap( cold_resist_cap().setAsValue( orig_energy_resist_cap() ) );
  if ( has_energy_resist_cap() || has_orig_energy_resist_cap() )
    energy_resist_cap( energy_resist_cap().setAsValue( orig_energy_resist_cap() ) );
  if ( has_physical_resist_cap() || has_orig_physical_resist_cap() )
    physical_resist_cap( physical_resist_cap().setAsValue( orig_physical_resist_cap() ) );
  if ( has_poison_resist_cap() || has_orig_poison_resist_cap() )
    poison_resist_cap( poison_resist_cap().setAsValue( orig_poison_resist_cap() ) );
  if ( has_luck() || has_orig_luck() )
    luck( luck().setAsValue( orig_luck() ) );
  if ( has_swing_speed_increase() || has_orig_swing_speed_increase() )
    swing_speed_increase( swing_speed_increase().setAsValue( orig_swing_speed_increase() ) );
  if ( has_min_attack_range_increase() || has_orig_min_attack_range_increase() )
    min_attack_range_increase(
        min_attack_range_increase().setAsValue( orig_min_attack_range_increase() ) );
  if ( has_max_attack_range_increase() || has_orig_max_attack_range_increase() )
    max_attack_range_increase(
        max_attack_range_increase().setAsValue( orig_max_attack_range_increase() ) );
}

size_t NPC::estimatedSize() const
{
  return base::estimatedSize() + sizeof( unsigned short )     /*damaged_sound*/
         + sizeof( unsigned short )                           /*run_speed*/
         + sizeof( Core::UOExecutor* )                        /*ex*/
         + sizeof( unsigned short )                           /*npc_ar_*/
         + sizeof( Core::CharacterRef )                       /*master_*/
         + sizeof( anchor )                                   /*anchor*/
         + sizeof( boost_utils::script_name_flystring )       /*script*/
         + sizeof( boost_utils::npctemplate_name_flystring ); /*template_name*/
}

u16 NPC::get_damaged_sound() const
{
  if ( damaged_sound != 0 )
    return damaged_sound;
  return base::get_damaged_sound();
}

bool NPC::use_adjustments() const
{
  return mob_flags_.get( MOB_FLAGS::USE_ADJUSTMENTS );
}

void NPC::use_adjustments( bool newvalue )
{
  mob_flags_.change( MOB_FLAGS::USE_ADJUSTMENTS, newvalue );
}

bool NPC::no_drop_exception() const
{
  return flags_.get( Core::OBJ_FLAGS::NO_DROP_EXCEPTION );
}

void NPC::no_drop_exception( bool newvalue )
{
  flags_.change( Core::OBJ_FLAGS::NO_DROP_EXCEPTION, newvalue );
}

std::string NPC::templatename() const
{
  return template_name;
}

bool NPC::get_method_hook( const char* methodname, Bscript::Executor* executor,
                           Core::ExportScript** hook, unsigned int* PC ) const
{
  if ( Core::gamestate.system_hooks.get_method_hook(
           Core::gamestate.system_hooks.npc_method_script.get(), methodname, executor, hook, PC ) )
    return true;
  return base::get_method_hook( methodname, executor, hook, PC );
}
}  // namespace Mobile
}  // namespace Pol
