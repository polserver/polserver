/*
History
=======
2005/06/15 Shinigami: added CanMove - checks if an NPC can move in given direction
                      (IsLegalMove works in a different way and is used for bounding boxes only)
2006/01/18 Shinigami: set Master first and then start Script in NPC::readNpcProperties
2006/01/18 Shinigami: added attached_npc_ - to get attached NPC from AI-Script-Process Obj
2006/09/17 Shinigami: send_event() will return error "Event queue is full, discarding event"
2009/03/27 MuadDib:   NPC::inform_moved() && NPC::inform_imoved()
                      split the left/entered area to fix bug where one would trigger when not enabled.
2009/07/23 MuadDib:   updates for new Enum::Packet Out ID
2009/08/25 Shinigami: STLport-5.2.1 fix: params not used
                      STLport-5.2.1 fix: init order changed of damaged_sound
2009/09/15 MuadDib:   Cleanup from registered houses on destroy
2009/09/18 MuadDib:   Adding save/load of registered house serial
2009/09/22 MuadDib:   Rewrite for Character/NPC to use ar(), ar_mod(), ar_mod(newvalue) virtuals.
2009/10/14 Turley:    Added char.deaf() methods & char.deafened member
2009/10/23 Turley:    fixed OPPONENT_MOVED,LEFTAREA,ENTEREDAREA
2009/11/16 Turley:    added NpcPropagateEnteredArea()/inform_enteredarea() for event on resurrection
2010/01/15 Turley:    (Tomi) SaveOnExit as npcdesc entry

Notes
=======

*/
#include "npc.h"
#include "../npctmpl.h"
#include "../module/npcmod.h"

#include "attribute.h"
#include "wornitems.h" // refresh_ar() is the only one which needs this include...

#include "../containr.h"
#include "../dice.h"
#include "../eventid.h"
#include "../fnsearch.h"
#include "../globals/state.h"
#include "../globals/uvars.h"
#include "../item/armor.h"
#include "../item/weapon.h"
#include "../listenpt.h"
#include "../mdelta.h"
#include "../module/osmod.h"
#include "../module/unimod.h"
#include "../module/uomod.h"
#include "../multi/house.h"
#include "../network/client.h"
#include "../objtype.h"
#include "../pktout.h"
#include "../poltype.h"
#include "../realms.h"
#include "../scrsched.h"
#include "../scrstore.h"
#include "../skilladv.h"
#include "../skills.h"
#include "../sockio.h"
#include "../ufunc.h"
#include "../ufunc.h"
#include "../ufuncinl.h"
#include "../uoexec.h"
#include "../uoexhelp.h"
#include "../uofile.h"
#include "../uoscrobj.h"
#include "../uworld.h"

#include "../../bscript/berror.h"
#include "../../bscript/eprog.h"
#include "../../bscript/execmodl.h"
#include "../../bscript/executor.h"
#include "../../bscript/impstr.h"
#include "../../bscript/modules.h"

#include "../../plib/realm.h"

#include "../../clib/cfgelem.h"
#include "../../clib/clib.h"
#include "../../clib/endian.h"
#include "../../clib/fileutil.h"
#include "../../clib/logfacility.h"
#include "../../clib/passert.h"
#include "../../clib/random.h"
#include "../../clib/stlutil.h"
#include "../../clib/streamsaver.h"
#include "../../clib/strutil.h"
#include "../../clib/unicode.h"

#include <stdexcept>

/* An area definition is as follows:
   pt: (x,y)
   rect: [pt-pt]
   area: rect,rect,...
   So, format is: [(x1,y1)-(x2,y2)],[],[],...
   Well, right now, the format is x1 y1 x2 y2 ... (ick)
*/
namespace Pol {
  namespace Mobile {
    unsigned short calc_thru_damage( double damage, unsigned short ar );

	NPC::NPC( u32 objtype, const Clib::ConfigElem& elem ) :
	  Character( objtype, CLASS_NPC ),
	  // UOBJECT INTERFACE
	  // NPC INTERFACE
	  npc_ar_( 0 ),
	  // MOVEMENT
	  run_speed( dexterity() ),
	  use_adjustments( true ),
	  anchor(),
	  // EVENTS
		speech_color_(Core::DEFAULT_TEXT_COLOR),
		speech_font_(Core::DEFAULT_TEXT_FONT),
	  // SCRIPT
	  script( "" ),
	  ex( NULL ),
	  // MISC
	  damaged_sound( 0 ),
	  template_name(),
	  master_( NULL ),
	  template_( Core::find_npc_template( elem ) )
	{
	  connected = 1;
	  logged_in = true;
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
	  wornitems.destroy_contents();
	  if ( registered_house > 0 )
	  {
			Multi::UMulti* multi = Core::system_find_multi(registered_house);
		if ( multi != NULL )
		{
			multi->unregister_object( ( UObject* )this );
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

	bool NPC::anchor_allows_move(Core::UFACING dir) const
	{
		unsigned short newx = x + Core::move_delta[dir].xmove;
		unsigned short newy = y + Core::move_delta[dir].ymove;

	  if ( anchor.enabled && !warmode )
	  {
			unsigned short curdist = Core::pol_distance(x, y, anchor.x, anchor.y);
			unsigned short newdist = Core::pol_distance(newx, newy, anchor.x, anchor.y);
			if ( newdist > curdist ) // if we're moving further away, see if we can
			{
				if ( newdist > anchor.dstart )
				{
					int perc = 100 - ( newdist - anchor.dstart )*anchor.psub;
					if ( perc < 5 )
						perc = 5;
					if ( Clib::random_int( 99 ) > perc )
						return false;
				}
			}
	  }
	  return true;
	}

	bool NPC::could_move(Core::UFACING dir) const
	{
	  short newz;
	  Multi::UMulti* supporting_multi;
	  Items::Item* walkon_item;
	  // Check for diagonal move - use Nandos change from charactr.cpp -- OWHorus (2011-04-26)
	  if ( dir & 1 ) // check if diagonal movement is allowed -- Nando (2009-02-26)
	  {
		u8 tmp_facing = ( dir + 1 ) & 0x7;
		unsigned short tmp_newx = x + Core::move_delta[tmp_facing].xmove;
		unsigned short tmp_newy = y + Core::move_delta[tmp_facing].ymove;

		// needs to save because if only one direction is blocked, it shouldn't block ;)
		short current_boost = gradual_boost;
		bool walk1 = realm->walkheight( this, tmp_newx, tmp_newy, z, &newz, &supporting_multi, &walkon_item, &current_boost );

		tmp_facing = ( dir - 1 ) & 0x7;
		tmp_newx = x + Core::move_delta[tmp_facing].xmove;
		tmp_newy = y + Core::move_delta[tmp_facing].ymove;
		current_boost = gradual_boost;
		if ( !walk1 && !realm->walkheight( this, tmp_newx, tmp_newy, z, &newz, &supporting_multi, &walkon_item, &current_boost ) )
		  return false;
	  }
	  unsigned short newx = x + Core::move_delta[dir].xmove;
	  unsigned short newy = y + Core::move_delta[dir].ymove;
	  short current_boost = gradual_boost;
	  return realm->walkheight( this, newx, newy, z, &newz, &supporting_multi, &walkon_item, &current_boost ) &&
		!npc_path_blocked( dir ) &&
		anchor_allows_move( dir );
	}

	bool NPC::npc_path_blocked(Core::UFACING dir) const
	{
		if (cached_settings.freemove || (!this->master() && !Core::settingsManager.ssopt.mobiles_block_npc_movement))
		return false;

		unsigned short newx = x + Core::move_delta[dir].xmove;
		unsigned short newy = y + Core::move_delta[dir].ymove;

	  unsigned short wx, wy;
	  Core::zone_convert_clip(newx, newy, realm, &wx, &wy);

	  if (Core::settingsManager.ssopt.mobiles_block_npc_movement)
      {
        for ( const auto &chr : realm->zone[wx][wy].characters )
        {
          // First check if there really is a character blocking
          if ( chr->x == newx &&
               chr->y == newy &&
               chr->z >= z - 10 && chr->z <= z + 10 )
          {
            if ( !chr->dead() && is_visible_to_me( chr ) )
              return true;
          }
        }
      }
      for ( const auto &chr : realm->zone[wx][wy].npcs )
      {
        // First check if there really is a character blocking
        if ( chr->x == newx &&
             chr->y == newy &&
             chr->z >= z - 10 && chr->z <= z + 10 )
        {
          // Check first with the ssopt false to now allow npcs of same master running on top of each other
		  if (!Core::settingsManager.ssopt.mobiles_block_npc_movement)
          {
			NPC* npc = static_cast<NPC*>( chr );
			if ( npc->master() && this->master() == npc->master() && !npc->dead() && is_visible_to_me( npc ) )
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
	  //sw.flush();
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

      if (speech_color_ != Core::DEFAULT_TEXT_COLOR)
		sw() << "\tSpeechColor\t" << speech_color_ << pf_endl;

	  if ( speech_font_ != Core::DEFAULT_TEXT_FONT )
		sw() << "\tSpeechFont\t" << speech_font_ << pf_endl;

	  if ( run_speed != dexterity() )
		sw() << "\tRunSpeed\t" << run_speed << pf_endl;

	  if ( use_adjustments != true )
		sw() << "\tUseAdjustments\t" << use_adjustments << pf_endl;

      s16 value = curr_fire_resist().value;
	  if ( value != 0 )
		sw() << "\tFireResist\t" << static_cast<int>( value ) << pf_endl;
      value = curr_cold_resist().value;
	  if ( value != 0 )
        sw( ) << "\tColdResist\t" << static_cast<int>( value ) << pf_endl;
      value = curr_energy_resist().value;
      if ( value != 0 )
        sw( ) << "\tEnergyResist\t" << static_cast<int>( value ) << pf_endl;
      value = curr_poison_resist().value;
      if ( value != 0 )
        sw( ) << "\tPoisonResist\t" << static_cast<int>( value ) << pf_endl;
      value = curr_physical_resist().value;
      if ( value != 0 )
        sw( ) << "\tPhysicalResist\t" << static_cast<int>( value ) << pf_endl;

      value = curr_fire_damage().value;
      if ( value != 0 )
		sw() << "\tFireDamage\t" << static_cast<int>( value) << pf_endl;
      value = curr_cold_damage().value;
      if ( value != 0 )
        sw( ) << "\tColdDamage\t" << static_cast<int>( value ) << pf_endl;
      value = curr_energy_damage().value;
      if ( value != 0 )
        sw( ) << "\tEnergyDamage\t" << static_cast<int>( value ) << pf_endl;
      value = curr_poison_damage().value;
      if ( value != 0 )
        sw( ) << "\tPoisonDamage\t" << static_cast<int>( value ) << pf_endl;
      value = curr_physical_damage().value;
      if ( value != 0 )
        sw( ) << "\tPhysicalDamage\t" << static_cast<int>( value ) << pf_endl;
	}

    void NPC::printDebugProperties( Clib::StreamWriter& sw ) const
	{
	  base::printDebugProperties( sw );
	  sw() << "# template: " << template_.name << pf_endl;
	  if ( anchor.enabled )
	  {
		sw() << "# anchor: x=" << anchor.x
		  << " y=" << anchor.y
		  << " dstart=" << anchor.dstart
		  << " psub=" << anchor.psub << pf_endl;
	  }
	}

    void NPC::readNpcProperties( Clib::ConfigElem& elem )
	{
	  registered_house = elem.remove_ulong( "REGISTEREDHOUSE", 0 );

	  Items::UWeapon* wpn = Items::find_intrinsic_weapon( elem.rest() );
	  if ( wpn == NULL )
	  {
		wpn = Items::create_intrinsic_weapon_from_npctemplate( elem, template_.pkg );
	  }
	  if ( wpn != NULL )
		weapon = wpn;

	  // Load the base, equiping items etc will refresh_ar() to update for reals.
	  for ( int i = 0; i < 6; i++ )
	  {
		loadResistances( i, elem );
		if ( i > 0 )
		  loadDamages( i, elem );
	  }

	  //dave 3/19/3, read templatename only if empty
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
      if ( !script.get().empty( ) )
		start_script();

	  speech_color_ = elem.remove_ushort( "SpeechColor", Core::DEFAULT_TEXT_COLOR );
	  speech_font_ = elem.remove_ushort( "SpeechFont", Core::DEFAULT_TEXT_FONT );
	  saveonexit_ = elem.remove_bool( "SaveOnExit", true );

	  use_adjustments = elem.remove_bool( "UseAdjustments", true );
	  run_speed = elem.remove_ushort( "RunSpeed", dexterity() );

	  damaged_sound = elem.remove_ushort( "DamagedSound", 0 );
	}

	// This now handles all resistances, including AR to simplify the code.
    void NPC::loadResistances( int resistanceType, Clib::ConfigElem& elem )
	{
      std::string tmp;
	  bool passed = false;
	  // 0 = AR
	  // 1 = Fire
	  // 2 = Cold
	  // 3 = Energy
	  // 4 = Poison
	  // 5 = Physical
	  switch ( resistanceType )
	  {
		case 0: passed = elem.remove_prop( "AR", &tmp ); break;
		case 1: passed = elem.remove_prop( "FIRERESIST", &tmp ); break;
		case 2: passed = elem.remove_prop( "COLDRESIST", &tmp ); break;
		case 3: passed = elem.remove_prop( "ENERGYRESIST", &tmp ); break;
		case 4: passed = elem.remove_prop( "POISONRESIST", &tmp ); break;
		case 5: passed = elem.remove_prop( "PHYSICALRESIST", &tmp ); break;
	  }

      int value=0;
	  if ( passed )
	  {
		Core::Dice dice;
        std::string errmsg;
		if ( !dice.load( tmp.c_str(), &errmsg ) )
          value = atoi( tmp.c_str() );
        else
          value = dice.roll();
      }
      // mod values are loaded before in character code!
	  switch ( resistanceType )
	  {
		case 0: npc_ar_ = static_cast<u16>(value); break;
        case 1:
        {
          if (value != 0)
          {
            fire_resist(fire_resist().addToValue(static_cast<s16>(value)));
            curr_fire_resist(curr_fire_resist().addToValue(static_cast<s16>(value)));
          }
          break;
        }
		case 2: 
        {
          if (value != 0)
          {
            cold_resist(cold_resist().addToValue(static_cast<s16>(value)));
            curr_cold_resist(curr_cold_resist().addToValue(static_cast<s16>(value)));
          }
          break;
        }
		case 3: 
        {
          if (value != 0)
          {
            energy_resist(energy_resist().addToValue(static_cast<s16>(value)));
            curr_energy_resist(curr_energy_resist().addToValue(static_cast<s16>(value)));
          }
          break;
        }
		case 4: 
        {
          if (value != 0)
          {
            poison_resist(poison_resist().addToValue(static_cast<s16>(value)));
            curr_poison_resist(curr_poison_resist().addToValue(static_cast<s16>(value)));
          }
          break;
        }
		case 5: 
        {
          if (value != 0)
          {
            physical_resist(physical_resist().addToValue(static_cast<s16>(value)));
            curr_physical_resist(curr_physical_resist().addToValue(static_cast<s16>(value)));
          }
          break;
        }
	  }

      Core::AosValuePack curr;
	  switch ( resistanceType )
	  {
		case 0: break; // ArMod isnt saved
		case 1:
          curr = fire_resist();
          curr.value += curr.mod;
          fire_resist(curr);
          break;
		case 2: 
          curr = cold_resist();
          curr.value += curr.mod;
          cold_resist(curr);
          break;
		case 3: 
          curr = energy_resist();
          curr.value += curr.mod;
          energy_resist(curr);
          break;
		case 4: 
          curr = poison_resist();
          curr.value += curr.mod;
          poison_resist(curr);
          break;
		case 5: 
          curr = physical_resist();
          curr.value += curr.mod;
          physical_resist(curr);
          break;
	  }
	}

	// This now handles all resistances, including AR to simplify the code.
    void NPC::loadDamages( int damageType, Clib::ConfigElem& elem )
	{
      std::string tmp;
	  bool passed = false;
	  // 1 = Fire
	  // 2 = Cold
	  // 3 = Energy
	  // 4 = Poison
	  // 5 = Physical
	  switch ( damageType )
	  {
		case 1: passed = elem.remove_prop( "FIREDAMAGE", &tmp ); break;
		case 2: passed = elem.remove_prop( "COLDDAMAGE", &tmp ); break;
		case 3: passed = elem.remove_prop( "ENERGYDAMAGE", &tmp ); break;
		case 4: passed = elem.remove_prop( "POISONDAMAGE", &tmp ); break;
		case 5: passed = elem.remove_prop( "PHYSICALDAMAGE", &tmp ); break;
	  }

      s16 value = 0;
	  if ( passed )
	  {
		Core::Dice dice;
        std::string errmsg;
		if ( !dice.load( tmp.c_str(), &errmsg ) )
          value = static_cast<s16>( atoi( tmp.c_str( ) ) );
        else
          value = dice.roll();
      }
      // mod values are loaded before in character code!
	  switch ( damageType )
	  {
		case 1: 
        {
          if (value != 0)
          {
            fire_damage(fire_damage().addToValue(value));
            curr_fire_damage(curr_fire_damage().addToValue(value));
          }
          break;
        }
		case 2: 
        {
          if (value != 0)
          {
            cold_damage(cold_damage().addToValue(value));
            curr_cold_damage(curr_cold_damage().addToValue(value));
          }
          break;
        }
		case 3: 
        {
          if (value != 0)
          {
            energy_damage(energy_damage().addToValue(value));
            curr_energy_damage(curr_energy_damage().addToValue(value));
          }
          break;
        }
		case 4: 
        {
          if (value != 0)
          {
            poison_damage(poison_damage().addToValue(value));
            curr_poison_damage(curr_poison_damage().addToValue(value));
          }
          break;
        }
		case 5: 
        {
          if (value != 0)
          {
            physical_damage(physical_damage().addToValue(value));
            curr_physical_damage(curr_physical_damage().addToValue(value));
          }
          break;
        }
	  }

      Core::AosValuePack curr;
	  switch ( damageType )
	  {
		case 1: 
          curr = fire_damage();
          curr.value += curr.mod;
          fire_damage(curr);
          break;
		case 2: 
          curr = cold_damage();
          curr.value += curr.mod;
          cold_damage(curr);
          break;
		case 3: 
          curr = energy_damage();
          curr.value += curr.mod;
          energy_damage(curr);
          break;
		case 4: 
          curr = poison_damage();
          curr.value += curr.mod;
          poison_damage(curr);
          break;
		case 5: 
          curr = physical_damage();
          curr.value += curr.mod;
          physical_damage(curr);
          break;
	  }
	}

    void NPC::readProperties( Clib::ConfigElem& elem )
	{
	  //3/18/3 dave copied this npctemplate code from readNpcProperties, because base::readProperties 
	  //will call the exported vital functions before npctemplate is set (distro uses npctemplate in the exported funcs).
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
			  elem.throw_error( "Error reading Attribute "
								+ pAttr->name +
								": " + errmsg );
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

	  ClrCharacterWorldPosition( this, Plib::WorldChangeReason::NpcDeath );
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
      passert( !script.get().empty( ) );
	  Core::ScriptDef sd( script, template_.pkg, "scripts/ai/" );
	  // Log( "NPC script starting: %s\n", sd.name().c_str() );

	  ref_ptr<Bscript::EScriptProgram> prog = Core::find_script2( sd );
	  // find_script( "ai/" + script );

	  if ( prog.get() == NULL )
	  {
        ERROR_PRINT << "Unable to read script " << sd.name()
          << " for NPC " << name() << "(0x" << fmt::hexu( serial ) << ")\n";
        throw std::runtime_error("Error loading NPCs");
	  }

	  ex = Core::create_script_executor();
	  ex->addModule( new Module::NPCExecutorModule( *ex, *this ) );
      Module::UOExecutorModule* uoemod = new Module::UOExecutorModule( *ex );
	  ex->addModule( uoemod );
	  if ( ex->setProgram( prog.get() ) == false )
	  {
        ERROR_PRINT << "There was an error running script " << script.get()<< " for NPC "
          << name() << "(0x" << fmt::hexu( serial ) << ")\n";
        throw std::runtime_error("Error loading NPCs");
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
		if ( ( ex->eventmask & Core::EVID_SPOKE ) &&
			 inrangex( this, src_chr, ex->speech_size ) &&
			 !deafened() )
		{
		  if ( ( !Core::settingsManager.ssopt.event_visibility_core_checks ) || is_visible_to_me( src_chr ) )
            ex->os_module->signal_event( new Module::SpeechEvent( src_chr, speech,
			Core::TextTypeToString( texttype ) ) ); //DAVE added texttype
		}
	  }
	}

	void NPC::on_ghost_pc_spoke( Character* src_chr, const char* speech, u8 texttype )
	{
	  if ( ex != NULL )
	  {
		if ( ( ex->eventmask & Core::EVID_GHOST_SPEECH ) &&
			 inrangex( this, src_chr, ex->speech_size ) &&
			 !deafened() )
		{
		  if ( ( !Core::settingsManager.ssopt.event_visibility_core_checks ) || is_visible_to_me( src_chr ) )
            ex->os_module->signal_event( new Module::SpeechEvent( src_chr, speech,
			Core::TextTypeToString( texttype ) ) ); //DAVE added texttype
		}
	  }
	}

	void NPC::on_pc_spoke( Character *src_chr, const char *speech, u8 texttype,
						   const u16* wspeech, const char lang[4], Bscript::ObjArray* speechtokens )
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
			 inrangex( this, src_chr, ex->speech_size ) &&
			 !deafened() )
		{
		  if ( ( !Core::settingsManager.ssopt.event_visibility_core_checks ) || is_visible_to_me( src_chr ) )
		  {
			ex->os_module->signal_event( new Module::UnicodeSpeechEvent( src_chr, speech,
			  Core::TextTypeToString( texttype ),
			  wspeech, lang, speechtokens ) );
		  }
		}
	  }
	}

	void NPC::on_ghost_pc_spoke( Character* src_chr, const char* speech, u8 texttype,
								 const u16* wspeech, const char lang[4], Bscript::ObjArray* speechtokens )
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
		if ( ( ( ex->eventmask & Core::EVID_GHOST_SPEECH ) || ( ex->eventmask & Core::EVID_TOKEN_GHOST_SPOKE ) ) &&
			 inrangex( this, src_chr, ex->speech_size ) &&
			 !deafened() )
		{
		  if ( ( !Core::settingsManager.ssopt.event_visibility_core_checks ) || is_visible_to_me( src_chr ) )
		  {
            ex->os_module->signal_event( new Module::UnicodeSpeechEvent( src_chr, speech,
			  Core::TextTypeToString( texttype ),
			  wspeech, lang, speechtokens ) );
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
		if ( ( ex->eventmask & ( Core::EVID_GONE_CRIMINAL ) ) && inrangex( this, thecriminal, ex->area_size ) )
		{
		  if ( ( !Core::settingsManager.ssopt.event_visibility_core_checks ) || is_visible_to_me( thecriminal ) )
            ex->os_module->signal_event( new Module::SourcedEvent( Core::EVID_GONE_CRIMINAL, thecriminal ) );
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
			if ( ( !Core::settingsManager.ssopt.event_visibility_core_checks ) || is_visible_to_me( wholeft ) )
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
			if ( ( !Core::settingsManager.ssopt.event_visibility_core_checks ) || is_visible_to_me( whoentered ) )
              ex->os_module->signal_event( new Module::SourcedEvent( Core::EVID_ENTEREDAREA, whoentered ) );
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
		  bool are_inrange = ( abs( x - moved->x ) <= ex->area_size ) &&
			( abs( y - moved->y ) <= ex->area_size );

		  // inrangex_inline( this, moved, ex->area_size );
		  bool were_inrange = ( abs( x - moved->lastx ) <= ex->area_size ) &&
			( abs( y - moved->lasty ) <= ex->area_size );

		  if ( ( !Core::settingsManager.ssopt.event_visibility_core_checks ) || is_visible_to_me( moved ) )
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

		if ( !signaled ) // only send moved event if left/enteredarea wasnt send
		{
		  if ( ( moved == opponent_ ) && ( ex->eventmask & ( Core::EVID_OPPONENT_MOVED ) ) )
		  {
			if ( ( !Core::settingsManager.ssopt.event_visibility_core_checks ) || is_visible_to_me( moved ) )
              ex->os_module->signal_event( new Module::SourcedEvent( Core::EVID_OPPONENT_MOVED, moved ) );
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
		  bool are_inrange = ( abs( x - chr->x ) <= ex->area_size ) &&
			( abs( y - chr->y ) <= ex->area_size );

		  // inrangex_inline( this, moved, ex->area_size );
		  bool were_inrange = ( abs( lastx - chr->x ) <= ex->area_size ) &&
			( abs( lasty - chr->y ) <= ex->area_size );

		  if ( ( !Core::settingsManager.ssopt.event_visibility_core_checks ) || is_visible_to_me( chr ) )
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
	  if (ex != NULL)
	  {
		if (ex->os_module->signal_event( event ))
		  return true;
	  }
	  Bscript::BObject bo( event );
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
			Bscript::BObject bo( event ); // to be sure the rawpointer gets deleted, signal_event should guard it (but only currently)
			return new Bscript::BError( "Event queue is full, discarding event" );
		}
	  }
	  else
	  {
        Bscript::BObject bo( event );
        return new Bscript::BError( "That NPC doesn't have a control script" );
	  }
	}

	void NPC::apply_raw_damage_hundredths( unsigned int damage, Character* source, bool userepsys, bool send_damage_packet )
	{
	  if ( ex != NULL )
	  {
		if ( ex->eventmask & Core::EVID_DAMAGED )
		{
          ex->os_module->signal_event( new Module::DamageEvent( source, static_cast<unsigned short>( damage / 100 ) ) );
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
		if ( blocked < 0 ) blocked = 0;
		int absorbed = blocked / 2;

		blocked -= absorbed;
        absorbed += Clib::random_int( blocked );
        if ( Core::settingsManager.watch.combat ) INFO_PRINT << absorbed << " hits absorbed by NPC armor.\n";
		damage -= absorbed;
		if ( damage < 0 )
		  damage = 0;
	  }
	  return damage;
	}

	void NPC::get_hitscript_params( double damage,
                                    Items::UArmor** parmor,
									unsigned short* rawdamage )
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

    Items::UWeapon* NPC::intrinsic_weapon( )
	{
	  if ( template_.intrinsic_weapon )
		return template_.intrinsic_weapon;
	  else
		return Core::gamestate.wrestling_weapon;
	}

	void NPC::refresh_ar()
	{
	  // This is an npc, we need to check to see if any armor is being wore
	  // otherwise we just reset this to the base values from their template.
	  bool hasArmor = false;
	  for ( unsigned layer = Core::LAYER_EQUIP__LOWEST; layer <= Core::LAYER_EQUIP__HIGHEST; ++layer )
	  {
        Items::Item *item = wornitems.GetItemOnLayer( layer );
		if ( item == NULL )
		  continue;
		for ( unsigned element = 0; element <= Core::ELEMENTAL_TYPE_MAX; ++element )
		{
          if ( item->calc_element_resist( (Core::ElementalType)element ) != 0 || item->calc_element_damage( (Core::ElementalType)element ) != 0 )
		  {
			hasArmor = true;
			break;
		  }
		}
	  }

	  if ( !hasArmor )
	  {
		ar_ = 0;
		for ( unsigned element = 0; element <= Core::ELEMENTAL_TYPE_MAX; ++element )
		{
          reset_element_resist( (Core::ElementalType)element );
          reset_element_damage( (Core::ElementalType)element );
		}
		return;
	  }

	  for ( unsigned zone = 0; zone < Core::gamestate.armorzones.size(); ++zone )
		armor_[zone] = NULL;
	  // we need to reset each resist to 0, then add the base back using calc.
	  for ( unsigned element = 0; element <= Core::ELEMENTAL_TYPE_MAX; ++element )
	  {
        refresh_element( (Core::ElementalType)element );
	  }

	  for ( unsigned layer = Core::LAYER_EQUIP__LOWEST; layer <= Core::LAYER_EQUIP__HIGHEST; ++layer )
	  {
        Items::Item *item = wornitems.GetItemOnLayer( layer );
		if ( item == NULL )
		  continue;
		// Let's check all items as base, and handle their element_resists.
		for ( unsigned element = 0; element <= Core::ELEMENTAL_TYPE_MAX; ++element )
		{
          update_element( (Core::ElementalType)element, item );
		}
		if ( item->isa( CLASS_ARMOR ) )
		{
          Items::UArmor* armor = static_cast<Items::UArmor*>( item );
		  std::set<unsigned short> tmplzones = armor->tmplzones();
		  std::set<unsigned short>::iterator itr;
		  for ( itr = tmplzones.begin(); itr != tmplzones.end(); ++itr )
		  {
			if ( ( armor_[*itr] == NULL ) || ( armor->ar() > armor_[*itr]->ar() ) )
			  armor_[*itr] = armor;
		  }
		}
	  }

	  double new_ar = 0.0;
      for ( unsigned zone = 0; zone < Core::gamestate.armorzones.size( ); ++zone )
	  {
		Items::UArmor* armor = armor_[zone];
		if ( armor != NULL )
		{
          new_ar += armor->ar( ) * Core::gamestate.armorzones[zone].chance;
		}
	  }

	  /* add AR due to shield : parry skill / 2 is percent of AR */
	  // FIXME: Should we allow this to be adjustable via a prop? Hrmmmmm
	  if ( shield != NULL )
	  {
		double add = 0.5 * 0.01 * shield->ar() * attribute( Core::gamestate.pAttrParry->attrid ).effective();
		if ( add > 1.0 )
		  new_ar += add;
		else
		  new_ar += 1.0;
	  }

	  new_ar += ar_mod();

	  short s_new_ar = static_cast<short>( new_ar );
	  if ( s_new_ar >= 0 )
		ar_ = s_new_ar;
	  else
		ar_ = 0;

	}

    void NPC::reset_element_resist( Core::ElementalType resist )
	{
      Core::AosValuePack curr;
      switch (resist)
      {
      case Core::ELEMENTAL_FIRE:
        if (has_fire_resist() || has_curr_fire_resist())
        {
          curr = fire_resist();
          curr.value = curr.mod + curr_fire_resist().value;
          fire_resist(curr);
        }
        break;
      case Core::ELEMENTAL_COLD:
        if (has_cold_resist() || has_curr_cold_resist())
        {
          curr = cold_resist();
          curr.value = curr.mod + curr_cold_resist().value;
          cold_resist(curr);
        }
        break;
      case Core::ELEMENTAL_ENERGY:
        if (has_energy_resist() || has_curr_energy_resist())
        {
          curr = energy_resist();
          curr.value = curr.mod + curr_energy_resist().value;
          energy_resist(curr);
        }
        break;
      case Core::ELEMENTAL_POISON:
        if (has_poison_resist() || has_curr_poison_resist())
        {
          curr = poison_resist();
          curr.value = curr.mod + curr_poison_resist().value;
          poison_resist(curr);
        }
        break;
      case Core::ELEMENTAL_PHYSICAL:
        if (has_physical_resist() || has_curr_physical_resist())
        {
          curr = physical_resist();
          curr.value = curr.mod + curr_physical_resist().value;
          physical_resist(curr);
        }
        break;
      }
	}

    void NPC::reset_element_damage( Core::ElementalType damage )
	{
      Core::AosValuePack curr;
      switch (damage)
      {
      case Core::ELEMENTAL_FIRE:
        if (has_fire_damage() || has_curr_fire_damage())
        {
          curr = fire_damage();
          curr.value = curr.mod + curr_fire_damage().value;
          fire_damage(curr);
        }
        break;
      case Core::ELEMENTAL_COLD:
        if (has_cold_damage() || has_curr_cold_damage())
        {
          curr = cold_damage();
          curr.value = curr.mod + curr_cold_damage().value;
          cold_damage(curr);
        }
        break;
      case Core::ELEMENTAL_ENERGY:
        if (has_energy_damage() || has_curr_energy_damage())
        {
          curr = energy_damage();
          curr.value = curr.mod + curr_energy_damage().value;
          energy_damage(curr);
        }
        break;
      case Core::ELEMENTAL_POISON:
        if (has_poison_damage() || has_curr_poison_damage())
        {
          curr = poison_damage();
          curr.value = curr.mod + curr_poison_damage().value;
          poison_damage(curr);
        }
        break;
      case Core::ELEMENTAL_PHYSICAL:
        if (has_physical_damage() || has_curr_physical_damage())
        {
          curr = physical_damage();
          curr.value = curr.mod + curr_physical_damage().value;
          physical_damage(curr);
        }
        break;
      }
	}

    size_t NPC::estimatedSize() const
    {
      return base::estimatedSize()
        + sizeof(unsigned short)/*damaged_sound*/
        +sizeof(bool)/*use_adjustments*/
        +sizeof(unsigned short)/*run_speed*/
        +sizeof(Core::UOExecutor*)/*ex*/
        +sizeof(unsigned short)/*npc_ar_*/
        +sizeof(Core::CharacterRef)/*master_*/
        +sizeof(anchor)/*anchor*/
        +sizeof(unsigned short)/*speech_color_*/
        +sizeof(unsigned short)/*speech_font_*/
        +sizeof( boost_utils::script_name_flystring ) /*script*/
        + sizeof( boost_utils::npctemplate_name_flystring ); /*template_name*/
    }

	u16 NPC::get_damaged_sound() const
	{
	  if (damaged_sound != 0)
		return damaged_sound;
	  return base::get_damaged_sound();
	}
  }
}