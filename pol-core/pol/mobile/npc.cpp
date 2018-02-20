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
#include "../mdelta.h"
#include "../module/npcmod.h"
#include "../module/osmod.h"
#include "../module/uomod.h"
#include "../multi/multi.h"
#include "../npctmpl.h"
#include "../scrdef.h"
#include "../scrsched.h"
#include "../scrstore.h"
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
      ex( NULL ),
      // MISC
      damaged_sound( 0 ),
      template_name(),
      master_( NULL ),
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
  if ( ex != NULL )
  {
    // this will force the execution engine to stop running this script immediately
    // dont delete the executor here, since it could currently run
    ex->seterror( true );
    ex->os_module->revive();
    if ( ex->os_module->in_debugger_holdlist() )
      ex->os_module->revive_debugged();
  }
}

void NPC::destroy()
{
  // stop_scripts();
  wornitems->destroy_contents();
  if ( registered_house > 0 )
  {
    Multi::UMulti* multi = Core::system_find_multi( registered_house );
    if ( multi != NULL )
    {
      multi->unregister_object( (UObject*)this );
    }
    registered_house = 0;
  }
  base::destroy();
}

const char* NPC::classname() const
{
  return "NPC";
}


// 8-25-05 Austin
// Moved unsigned short pol_distance( unsigned short x1, unsigned short y1,
//									unsigned short x2, unsigned short y2 )
// to ufunc.cpp

bool NPC::anchor_allows_move( Core::UFACING fdir ) const
{
  unsigned short newx = x + Core::move_delta[fdir].xmove;
  unsigned short newy = y + Core::move_delta[fdir].ymove;

  if ( anchor.enabled && !warmode() )
  {
    unsigned short curdist = Core::pol_distance( x, y, anchor.x, anchor.y );
    unsigned short newdist = Core::pol_distance( newx, newy, anchor.x, anchor.y );
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
    unsigned short tmp_newx = x + Core::move_delta[tmp_facing].xmove;
    unsigned short tmp_newy = y + Core::move_delta[tmp_facing].ymove;

    // needs to save because if only one direction is blocked, it shouldn't block ;)
    short current_boost = gradual_boost;
    bool walk1 = realm->walkheight( this, tmp_newx, tmp_newy, z, &newz, &supporting_multi,
                                    &walkon_item, &current_boost );

    tmp_facing = ( fdir - 1 ) & 0x7;
    tmp_newx = x + Core::move_delta[tmp_facing].xmove;
    tmp_newy = y + Core::move_delta[tmp_facing].ymove;
    current_boost = gradual_boost;
    if ( !walk1 && !realm->walkheight( this, tmp_newx, tmp_newy, z, &newz, &supporting_multi,
                                       &walkon_item, &current_boost ) )
      return false;
  }
  unsigned short newx = x + Core::move_delta[fdir].xmove;
  unsigned short newy = y + Core::move_delta[fdir].ymove;
  short current_boost = gradual_boost;
  return realm->walkheight( this, newx, newy, z, &newz, &supporting_multi, &walkon_item,
                            &current_boost ) &&
         !npc_path_blocked( fdir ) && anchor_allows_move( fdir );
}

bool NPC::npc_path_blocked( Core::UFACING fdir ) const
{
  if ( can_freemove() ||
       ( !this->master() && !Core::settingsManager.ssopt.mobiles_block_npc_movement ) )
    return false;

  unsigned short newx = x + Core::move_delta[fdir].xmove;
  unsigned short newy = y + Core::move_delta[fdir].ymove;

  unsigned short wx, wy;
  Core::zone_convert_clip( newx, newy, realm, &wx, &wy );

  if ( Core::settingsManager.ssopt.mobiles_block_npc_movement )
  {
    for ( const auto& chr : realm->zone[wx][wy].characters )
    {
      // First check if there really is a character blocking
      if ( chr->x == newx && chr->y == newy && chr->z >= z - 10 && chr->z <= z + 10 )
      {
        if ( !chr->dead() && is_visible_to_me( chr ) )
          return true;
      }
    }
  }
  for ( const auto& chr : realm->zone[wx][wy].npcs )
  {
    // First check if there really is a character blocking
    if ( chr->x == newx && chr->y == newy && chr->z >= z - 10 && chr->z <= z + 10 )
    {
      // Check first with the ssopt false to now allow npcs of same master running on top of
      // each other
      if ( !Core::settingsManager.ssopt.mobiles_block_npc_movement )
      {
        NPC* npc = static_cast<NPC*>( chr );
        if ( npc->master() && this->master() == npc->master() && !npc->dead() &&
             is_visible_to_me( npc ) )
          return true;
      }
      else
      {
        if ( !chr->dead() && is_visible_to_me( chr ) )
          return true;
      }
    }
  }
  return false;
}

void NPC::printOn( Clib::StreamWriter& sw ) const
{
  sw() << classname() << " " << template_name.get() << pf_endl;
  sw() << "{" << pf_endl;
  printProperties( sw );
  sw() << "}" << pf_endl;
  sw() << pf_endl;
  // sw.flush();
}

void NPC::printSelfOn( Clib::StreamWriter& sw ) const
{
  printOn( sw );
}

void NPC::printProperties( Clib::StreamWriter& sw ) const
{
  using namespace fmt;

  base::printProperties( sw );

  if ( registered_house )
    sw() << "\tRegisteredHouse\t0x" << hex( registered_house ) << pf_endl;

  if ( npc_ar_ )
    sw() << "\tAR\t" << npc_ar_ << pf_endl;

  if ( !script.get().empty() )
    sw() << "\tscript\t" << script.get() << pf_endl;

  if ( master_.get() != NULL )
    sw() << "\tmaster\t" << master_->serial << pf_endl;

  if ( has_speech_color() )
    sw() << "\tSpeechColor\t" << speech_color() << pf_endl;

  if ( has_speech_font() )
    sw() << "\tSpeechFont\t" << speech_font() << pf_endl;

  if ( run_speed != dexterity() )
    sw() << "\tRunSpeed\t" << run_speed << pf_endl;

  if ( use_adjustments() != true )
    sw() << "\tUseAdjustments\t" << use_adjustments() << pf_endl;

  if ( has_orig_fire_resist() )
    sw() << "\tFireResist\t" << orig_fire_resist() << pf_endl;
  if ( has_orig_cold_resist() )
    sw() << "\tColdResist\t" << orig_cold_resist() << pf_endl;
  if ( has_orig_energy_resist() )
    sw() << "\tEnergyResist\t" << orig_energy_resist() << pf_endl;
  if ( has_orig_poison_resist() )
    sw() << "\tPoisonResist\t" << orig_poison_resist() << pf_endl;
  if ( has_orig_physical_resist() )
    sw() << "\tPhysicalResist\t" << orig_physical_resist() << pf_endl;

  if ( has_orig_fire_damage() )
    sw() << "\tFireDamage\t" << orig_fire_damage() << pf_endl;
  if ( has_orig_cold_damage() )
    sw() << "\tColdDamage\t" << orig_cold_damage() << pf_endl;
  if ( has_orig_energy_damage() )
    sw() << "\tEnergyDamage\t" << orig_energy_damage() << pf_endl;
  if ( has_orig_poison_damage() )
    sw() << "\tPoisonDamage\t" << orig_poison_damage() << pf_endl;
  if ( has_orig_physical_damage() )
    sw() << "\tPhysicalDamage\t" << orig_physical_damage() << pf_endl;
  if ( no_drop_exception() )
    sw() << "\tNoDropException\t" << no_drop_exception() << pf_endl;
}

void NPC::printDebugProperties( Clib::StreamWriter& sw ) const
{
  base::printDebugProperties( sw );
  sw() << "# template: " << template_.name << pf_endl;
  if ( anchor.enabled )
  {
    sw() << "# anchor: x=" << anchor.x << " y=" << anchor.y << " dstart=" << anchor.dstart
         << " psub=" << anchor.psub << pf_endl;
  }
}

void NPC::readNpcProperties( Clib::ConfigElem& elem )
{
  registered_house = elem.remove_ulong( "REGISTEREDHOUSE", 0 );

  Items::UWeapon* wpn = static_cast<Items::UWeapon*>(
      Items::find_intrinsic_equipment( elem.rest(), Core::LAYER_HAND1 ) );
  if ( wpn == NULL )
    wpn = Items::create_intrinsic_weapon_from_npctemplate( elem, template_.pkg );
  if ( wpn != NULL )
    weapon = wpn;

  Items::UArmor* sld = static_cast<Items::UArmor*>(
      Items::find_intrinsic_equipment( elem.rest(), Core::LAYER_HAND2 ) );
  if ( sld == NULL )
    sld = Items::create_intrinsic_shield_from_npctemplate( elem, template_.pkg );
  if ( sld != NULL )
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
    if ( chr != NULL )
      master_.set( chr );
  }

  script = elem.remove_string( "script", "" );
  if ( !script.get().empty() )
    start_script();

  speech_color( elem.remove_ushort( "SpeechColor", Core::DEFAULT_TEXT_COLOR ) );
  speech_font( elem.remove_ushort( "SpeechFont", Core::DEFAULT_TEXT_FONT ) );
  saveonexit( elem.remove_bool( "SaveOnExit", true ) );

  mob_flags_.change( MOB_FLAGS::USE_ADJUSTMENTS, elem.remove_bool( "UseAdjustments", true ) );
  run_speed = elem.remove_ushort( "RunSpeed", dexterity() );

  damaged_sound = elem.remove_ushort( "DamagedSound", 0 );
  no_drop_exception( elem.remove_bool( "NoDropException", false ) );
}

void NPC::loadEquipablePropertiesNPC( Clib::ConfigElem& elem )
{
  // for ar and elemental damage/resist the mod values are loaded before in character code!
  auto diceValue = []( const std::string& dicestr, int* value ) -> bool {
    Core::Dice dice;
    std::string errmsg;
    if ( !dice.load( dicestr.c_str(), &errmsg ) )
      *value = atoi( dicestr.c_str() );
    else
      *value = dice.roll();
    return *value != 0;
  };
  auto apply = []( Core::ValueModPack v, int value ) -> Core::ValueModPack {
    return v.addToValue( static_cast<s16>( value ) );
  };
  auto refresh = []( Core::ValueModPack v ) -> Core::ValueModPack { return v.addToValue( v.mod ); };

  std::string tmp;
  int value;
  if ( elem.remove_prop( "AR", &tmp ) && diceValue( tmp, &value ) )
    npc_ar_ = static_cast<u16>( value );

  // elemental start
  // first apply template value as value and if mod or value exist sum them
  if ( elem.remove_prop( "FIRERESIST", &tmp ) && diceValue( tmp, &value ) )
  {
    fire_resist( apply( fire_resist(), value ) );
    orig_fire_resist( static_cast<s16>( value ) );
  }
  if ( has_fire_resist() )
    fire_resist( refresh( fire_resist() ) );
  if ( elem.remove_prop( "COLDRESIST", &tmp ) && diceValue( tmp, &value ) )
  {
    cold_resist( apply( cold_resist(), value ) );
    orig_cold_resist( static_cast<s16>( value ) );
  }
  if ( has_cold_resist() )
    cold_resist( refresh( cold_resist() ) );
  if ( elem.remove_prop( "ENERGYRESIST", &tmp ) && diceValue( tmp, &value ) )
  {
    energy_resist( apply( energy_resist(), value ) );
    orig_energy_resist( static_cast<s16>( value ) );
  }
  if ( has_energy_resist() )
    energy_resist( refresh( energy_resist() ) );
  if ( elem.remove_prop( "POISONRESIST", &tmp ) && diceValue( tmp, &value ) )
  {
    poison_resist( apply( poison_resist(), value ) );
    orig_poison_resist( static_cast<s16>( value ) );
  }
  if ( has_poison_resist() )
    poison_resist( refresh( poison_resist() ) );
  if ( elem.remove_prop( "PHYSICALRESIST", &tmp ) && diceValue( tmp, &value ) )
  {
    physical_resist( apply( physical_resist(), value ) );
    orig_physical_resist( static_cast<s16>( value ) );
  }
  if ( has_physical_resist() )
    physical_resist( refresh( physical_resist() ) );

  if ( elem.remove_prop( "FIREDAMAGE", &tmp ) && diceValue( tmp, &value ) )
  {
    fire_damage( apply( fire_damage(), value ) );
    orig_fire_damage( static_cast<s16>( value ) );
  }
  if ( has_fire_damage() )
    fire_damage( refresh( fire_damage() ) );
  if ( elem.remove_prop( "COLDDAMAGE", &tmp ) && diceValue( tmp, &value ) )
  {
    cold_damage( apply( cold_damage(), value ) );
    orig_cold_damage( static_cast<s16>( value ) );
  }
  if ( has_cold_damage() )
    cold_damage( refresh( cold_damage() ) );
  if ( elem.remove_prop( "ENERGYDAMAGE", &tmp ) && diceValue( tmp, &value ) )
  {
    energy_damage( apply( energy_damage(), value ) );
    orig_energy_damage( static_cast<s16>( value ) );
  }
  if ( has_energy_damage() )
    energy_damage( refresh( energy_damage() ) );
  if ( elem.remove_prop( "POISONDAMAGE", &tmp ) && diceValue( tmp, &value ) )
  {
    poison_damage( apply( poison_damage(), value ) );
    orig_poison_damage( static_cast<s16>( value ) );
  }
  if ( has_poison_damage() )
    poison_damage( refresh( poison_damage() ) );
  if ( elem.remove_prop( "PHYSICALDAMAGE", &tmp ) && diceValue( tmp, &value ) )
  {
    physical_damage( apply( physical_damage(), value ) );
    orig_physical_damage( static_cast<s16>( value ) );
  }
  if ( has_physical_damage() )
    physical_damage( refresh( physical_damage() ) );
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

  for ( Attribute* pAttr = Attribute::FindAttribute( 0 ); pAttr; pAttr = pAttr->next )
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
  if ( ex != NULL )
  {
    ex->seterror( true );
    // A Sleeping script would otherwise sit and wait until it wakes up to be killed.
    ex->os_module->revive();
    if ( ex->os_module->in_debugger_holdlist() )
      ex->os_module->revive_debugged();
    ex = NULL;
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
  if ( ex != NULL )
  {
    // this will force the execution engine to stop running this script immediately
    ex->seterror( true );
    ex->os_module->revive();
    if ( ex->os_module->in_debugger_holdlist() )
      ex->os_module->revive_debugged();
  }

  destroy();
}


void NPC::start_script()
{
  passert( ex == NULL );
  passert( !script.get().empty() );
  Core::ScriptDef sd( script, template_.pkg, "scripts/ai/" );
  // Log( "NPC script starting: %s\n", sd.name().c_str() );

  ref_ptr<Bscript::EScriptProgram> prog = Core::find_script2( sd );
  // find_script( "ai/" + script );

  if ( prog.get() == NULL )
  {
    ERROR_PRINT << "Unable to read script " << sd.name() << " for NPC " << name() << "(0x"
                << fmt::hexu( serial ) << ")\n";
    throw std::runtime_error( "Error loading NPCs" );
  }

  ex = Core::create_script_executor();
  ex->addModule( new Module::NPCExecutorModule( *ex, *this ) );
  Module::UOExecutorModule* uoemod = new Module::UOExecutorModule( *ex );
  ex->addModule( uoemod );
  if ( ex->setProgram( prog.get() ) == false )
  {
    ERROR_PRINT << "There was an error running script " << script.get() << " for NPC " << name()
                << "(0x" << fmt::hexu( serial ) << ")\n";
    throw std::runtime_error( "Error loading NPCs" );
  }

  uoemod->attached_npc_ = this;

  schedule_executor( ex );
}


bool NPC::can_be_renamed_by( const Character* chr ) const
{
  return ( master_.get() == chr );
}


void NPC::on_pc_spoke( Character* src_chr, const char* speech, u8 texttype )
{
  /*
  cerr << "PC " << src_chr->name()
  << " spoke in range of NPC " << name()
  << ": '" << speech << "'" << endl;
  */

  if ( ex != NULL )
  {
    if ( ( ex->eventmask & Core::EVID_SPOKE ) && inrangex( this, src_chr, ex->speech_size ) &&
         !deafened() )
    {
      if ( ( !Core::settingsManager.ssopt.event_visibility_core_checks ) ||
           is_visible_to_me( src_chr ) )
        ex->os_module->signal_event(
            new Module::SpeechEvent( src_chr, speech,
                                     Core::TextTypeToString( texttype ) ) );  // DAVE added texttype
    }
  }
}

void NPC::on_ghost_pc_spoke( Character* src_chr, const char* speech, u8 texttype )
{
  if ( ex != NULL )
  {
    if ( ( ex->eventmask & Core::EVID_GHOST_SPEECH ) &&
         inrangex( this, src_chr, ex->speech_size ) && !deafened() )
    {
      if ( ( !Core::settingsManager.ssopt.event_visibility_core_checks ) ||
           is_visible_to_me( src_chr ) )
        ex->os_module->signal_event(
            new Module::SpeechEvent( src_chr, speech,
                                     Core::TextTypeToString( texttype ) ) );  // DAVE added texttype
    }
  }
}

void NPC::on_pc_spoke( Character* src_chr, const char* speech, u8 texttype, const u16* wspeech,
                       const char lang[4], Bscript::ObjArray* speechtokens )
{
  if ( ex != NULL )
  {
    if ( Core::settingsManager.ssopt.seperate_speechtoken )
    {
      if ( speechtokens != NULL && ( ( ex->eventmask & Core::EVID_TOKEN_SPOKE ) == 0 ) )
        return;
      else if ( speechtokens == NULL && ( ( ex->eventmask & Core::EVID_SPOKE ) == 0 ) )
        return;
    }
    if ( ( ( ex->eventmask & Core::EVID_SPOKE ) || ( ex->eventmask & Core::EVID_TOKEN_SPOKE ) ) &&
         inrangex( this, src_chr, ex->speech_size ) && !deafened() )
    {
      if ( ( !Core::settingsManager.ssopt.event_visibility_core_checks ) ||
           is_visible_to_me( src_chr ) )
      {
        ex->os_module->signal_event( new Module::UnicodeSpeechEvent(
            src_chr, speech, Core::TextTypeToString( texttype ), wspeech, lang, speechtokens ) );
      }
    }
  }
}

void NPC::on_ghost_pc_spoke( Character* src_chr, const char* speech, u8 texttype,
                             const u16* wspeech, const char lang[4],
                             Bscript::ObjArray* speechtokens )
{
  if ( ex != NULL )
  {
    if ( Core::settingsManager.ssopt.seperate_speechtoken )
    {
      if ( speechtokens != NULL && ( ( ex->eventmask & Core::EVID_TOKEN_GHOST_SPOKE ) == 0 ) )
        return;
      else if ( speechtokens == NULL && ( ( ex->eventmask & Core::EVID_GHOST_SPEECH ) == 0 ) )
        return;
    }
    if ( ( ( ex->eventmask & Core::EVID_GHOST_SPEECH ) ||
           ( ex->eventmask & Core::EVID_TOKEN_GHOST_SPOKE ) ) &&
         inrangex( this, src_chr, ex->speech_size ) && !deafened() )
    {
      if ( ( !Core::settingsManager.ssopt.event_visibility_core_checks ) ||
           is_visible_to_me( src_chr ) )
      {
        ex->os_module->signal_event( new Module::UnicodeSpeechEvent(
            src_chr, speech, Core::TextTypeToString( texttype ), wspeech, lang, speechtokens ) );
      }
    }
  }
}

void NPC::inform_engaged( Character* engaged )
{
  // someone has targetted us. Create an event if appropriate.
  if ( ex != NULL )
  {
    if ( ex->eventmask & Core::EVID_ENGAGED )
    {
      ex->os_module->signal_event( new Module::EngageEvent( engaged ) );
    }
  }
  // Note, we don't do the base class thing, 'cause we have no client.
}

void NPC::inform_disengaged( Character* disengaged )
{
  // someone has targetted us. Create an event if appropriate.
  if ( ex != NULL )
  {
    if ( ex->eventmask & Core::EVID_DISENGAGED )
    {
      ex->os_module->signal_event( new Module::DisengageEvent( disengaged ) );
    }
  }
  // Note, we don't do the base class thing, 'cause we have no client.
}

void NPC::inform_criminal( Character* thecriminal )
{
  if ( ex != NULL )
  {
    if ( ( ex->eventmask & ( Core::EVID_GONE_CRIMINAL ) ) &&
         inrangex( this, thecriminal, ex->area_size ) )
    {
      if ( ( !Core::settingsManager.ssopt.event_visibility_core_checks ) ||
           is_visible_to_me( thecriminal ) )
        ex->os_module->signal_event(
            new Module::SourcedEvent( Core::EVID_GONE_CRIMINAL, thecriminal ) );
    }
  }
}

void NPC::inform_leftarea( Character* wholeft )
{
  if ( ex != NULL )
  {
    if ( ex->eventmask & ( Core::EVID_LEFTAREA ) )
    {
      if ( pol_distance( this, wholeft ) <= ex->area_size )
      {
        if ( ( !Core::settingsManager.ssopt.event_visibility_core_checks ) ||
             is_visible_to_me( wholeft ) )
          ex->os_module->signal_event( new Module::SourcedEvent( Core::EVID_LEFTAREA, wholeft ) );
      }
    }
  }
}

void NPC::inform_enteredarea( Character* whoentered )
{
  if ( ex != NULL )
  {
    if ( ex->eventmask & ( Core::EVID_ENTEREDAREA ) )
    {
      if ( pol_distance( this, whoentered ) <= ex->area_size )
      {
        if ( ( !Core::settingsManager.ssopt.event_visibility_core_checks ) ||
             is_visible_to_me( whoentered ) )
          ex->os_module->signal_event(
              new Module::SourcedEvent( Core::EVID_ENTEREDAREA, whoentered ) );
      }
    }
  }
}

void NPC::inform_moved( Character* moved )
{
  // 8-26-05    Austin
  // Note: This does not look at realms at all, just X Y coords.
  // ^is_visible_to_me checks realm - Turley

  if ( ex != NULL )
  {
    bool signaled = false;
    passert( this != NULL );
    passert( moved != NULL );
    if ( ex->eventmask & ( Core::EVID_ENTEREDAREA | Core::EVID_LEFTAREA ) )
    {
      // egcs may have a compiler bug when calling these as inlines
      bool are_inrange =
          ( abs( x - moved->x ) <= ex->area_size ) && ( abs( y - moved->y ) <= ex->area_size );

      // inrangex_inline( this, moved, ex->area_size );
      bool were_inrange = ( abs( x - moved->lastx ) <= ex->area_size ) &&
                          ( abs( y - moved->lasty ) <= ex->area_size );

      if ( ( !Core::settingsManager.ssopt.event_visibility_core_checks ) ||
           is_visible_to_me( moved ) )
      {
        if ( are_inrange && !were_inrange && ( ex->eventmask & ( Core::EVID_ENTEREDAREA ) ) )
        {
          ex->os_module->signal_event( new Module::SourcedEvent( Core::EVID_ENTEREDAREA, moved ) );
          signaled = true;
        }
        else if ( !are_inrange && were_inrange && ( ex->eventmask & ( Core::EVID_LEFTAREA ) ) )
        {
          ex->os_module->signal_event( new Module::SourcedEvent( Core::EVID_LEFTAREA, moved ) );
          signaled = true;
        }
      }
    }

    if ( !signaled )  // only send moved event if left/enteredarea wasnt send
    {
      if ( ( moved == opponent_ ) && ( ex->eventmask & ( Core::EVID_OPPONENT_MOVED ) ) )
      {
        if ( ( !Core::settingsManager.ssopt.event_visibility_core_checks ) ||
             is_visible_to_me( moved ) )
          ex->os_module->signal_event(
              new Module::SourcedEvent( Core::EVID_OPPONENT_MOVED, moved ) );
      }
    }
  }
}

//
// This NPC moved.  Tell him about other mobiles that have "entered" his area
// (through his own movement)
//

void NPC::inform_imoved( Character* chr )
{
  if ( ex != NULL )
  {
    passert( this != NULL );
    passert( chr != NULL );
    if ( ex->eventmask & ( Core::EVID_ENTEREDAREA | Core::EVID_LEFTAREA ) )
    {
      // egcs may have a compiler bug when calling these as inlines
      bool are_inrange =
          ( abs( x - chr->x ) <= ex->area_size ) && ( abs( y - chr->y ) <= ex->area_size );

      // inrangex_inline( this, moved, ex->area_size );
      bool were_inrange =
          ( abs( lastx - chr->x ) <= ex->area_size ) && ( abs( lasty - chr->y ) <= ex->area_size );

      if ( ( !Core::settingsManager.ssopt.event_visibility_core_checks ) ||
           is_visible_to_me( chr ) )
      {
        if ( are_inrange && !were_inrange && ( ex->eventmask & ( Core::EVID_ENTEREDAREA ) ) )
          ex->os_module->signal_event( new Module::SourcedEvent( Core::EVID_ENTEREDAREA, chr ) );
        else if ( !are_inrange && were_inrange && ( ex->eventmask & ( Core::EVID_LEFTAREA ) ) )
          ex->os_module->signal_event( new Module::SourcedEvent( Core::EVID_LEFTAREA, chr ) );
      }
    }
  }
}

bool NPC::can_accept_event( Core::EVENTID eventid )
{
  if ( ex == NULL )
    return false;
  if ( ex->eventmask & eventid )
    return true;
  else
    return false;
}

bool NPC::send_event( Bscript::BObjectImp* event )
{
  if ( ex != NULL )
  {
    if ( ex->os_module->signal_event( event ) )
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
  if ( ex != NULL )
  {
    if ( ex->os_module->signal_event( event ) )
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
  if ( ex != NULL )
  {
    if ( ex->eventmask & Core::EVID_DAMAGED )
    {
      ex->os_module->signal_event(
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
      INFO_PRINT << absorbed << " hits absorbed by NPC armor.\n";
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
  if ( template_.intrinsic_weapon )
    return template_.intrinsic_weapon;
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
      armor_[zone] = NULL;
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
    fire_resist( fire_resist().resetModAsValue().addToValue( orig_fire_resist() ) );
  if ( has_cold_resist() || has_orig_cold_resist() )
    cold_resist( cold_resist().resetModAsValue().addToValue( orig_cold_resist() ) );
  if ( has_energy_resist() || has_orig_energy_resist() )
    energy_resist( energy_resist().resetModAsValue().addToValue( orig_energy_resist() ) );
  if ( has_poison_resist() || has_orig_poison_resist() )
    poison_resist( poison_resist().resetModAsValue().addToValue( orig_poison_resist() ) );
  if ( has_physical_resist() || has_orig_physical_resist() )
    physical_resist( physical_resist().resetModAsValue().addToValue( orig_physical_resist() ) );

  if ( has_fire_damage() || has_orig_fire_damage() )
    fire_damage( fire_damage().resetModAsValue().addToValue( orig_fire_damage() ) );
  if ( has_cold_damage() || has_orig_cold_damage() )
    cold_damage( cold_damage().resetModAsValue().addToValue( orig_cold_damage() ) );
  if ( has_energy_damage() || has_orig_energy_damage() )
    energy_damage( energy_damage().resetModAsValue().addToValue( orig_energy_damage() ) );
  if ( has_poison_damage() || has_orig_poison_damage() )
    poison_damage( poison_damage().resetModAsValue().addToValue( orig_poison_damage() ) );
  if ( has_physical_damage() || has_orig_physical_damage() )
    physical_damage( physical_damage().resetModAsValue().addToValue( orig_physical_damage() ) );
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
}
}
