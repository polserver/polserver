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

Notes
=======

*/

#include "../clib/stl_inc.h"

#ifdef _MSC_VER
#	pragma warning(disable:4786)
#endif

#include "../clib/cfgelem.h"
#include "../clib/clib.h"
#include "../clib/endian.h"
#include "../clib/fileutil.h"
#include "../clib/logfile.h"
#include "../clib/mlog.h"
#include "../clib/passert.h"
#include "../clib/random.h"
#include "../clib/stlutil.h"
#include "../clib/strutil.h"
#include "../clib/unicode.h"

#include "../bscript/berror.h"
#include "../bscript/eprog.h"
#include "../bscript/executor.h"
#include "../bscript/execmodl.h"
#include "../bscript/modules.h"
#include "../bscript/impstr.h"

#include "../plib/realm.h"

#include "item/armor.h"
#include "mobile/attribute.h"
#include "network/client.h"
#include "dice.h"
#include "eventid.h"
#include "fnsearch.h"
#include "realms.h"
#include "scrsched.h"
#include "listenpt.h"
#include "npc.h"
#include "module/npcmod.h"
#include "npctmpl.h"
#include "poltype.h"
#include "pktout.h"
#include "ufunc.h"
#include "ufuncinl.h"
#include "scrstore.h"
#include "skilladv.h"
#include "skills.h"
#include "sockio.h"
#include "ssopt.h"
#include "uvars.h"
#include "module/osmod.h"
#include "uoexec.h"
#include "module/uomod.h"
#include "objtype.h"
#include "ufunc.h"
#include "module/unimod.h"
#include "uoexhelp.h"
#include "uoscrobj.h"
#include "watch.h"
#include "item/weapon.h"
#include "wrldsize.h"
#include "multi/house.h"

/* An area definition is as follows:
   pt: (x,y)
   rect: [pt-pt]
   area: rect,rect,...
   So, format is: [(x1,y1)-(x2,y2)],[],[],...
   Well, right now, the format is x1 y1 x2 y2 ... (ick)
*/

NPC::NPC( u16 objtype, const ConfigElem& elem ) :
    Character(objtype, CLASS_NPC),
	damaged_sound(0),
	ex(NULL),
	give_item_ex(NULL),
    script(""),
    npc_ar_(0),
    master_(NULL),
    template_( find_npc_template( elem ) ),
    speech_color_(DEFAULT_TEXT_COLOR),
    speech_font_(DEFAULT_TEXT_FONT)
{
	connected = 1;
    logged_in = true;
    anchor.enabled = false;
    ++npc_count;
}
 
NPC::~NPC()
{
	stop_scripts();
    --npc_count;
}

void NPC::stop_scripts()
{
    if (ex != NULL)
	{
		deschedule_executor( ex );
    	delete ex;
    	ex = NULL;
	}
}

void NPC::destroy()
{
   // stop_scripts();
    wornitems.destroy_contents();
	if ( registered_house > 0 )
	{
		UMulti* multi = system_find_multi(registered_house);
		if(multi != NULL)
		{
			UHouse* house = multi->as_house();
			if(house != NULL)
				house->unregister_object((UObject*)this);
		}
		registered_house = 0;
	}
    base::destroy();
}

const char* NPC::classname() const
{
    return "NPC";
}
#include "mdelta.h"
#include "uofile.h"
#include "uworld.h"

// 8-25-05 Austin
// Moved unsigned short pol_distance( unsigned short x1, unsigned short y1, 
//									unsigned short x2, unsigned short y2 )
// to ufunc.cpp

bool NPC::anchor_allows_move( UFACING dir ) const
{
    unsigned short newx = x + move_delta[ dir ].xmove;
    unsigned short newy = y + move_delta[ dir ].ymove;

    if (anchor.enabled && !warmode)
    {
        unsigned short curdist = pol_distance( x,    y,    anchor.x, anchor.y );
        unsigned short newdist = pol_distance( newx, newy, anchor.x, anchor.y );
        if (newdist > curdist) // if we're moving further away, see if we can
        {
            if (newdist > anchor.dstart)
            {
                int perc = 100 - (newdist-anchor.dstart)*anchor.psub;
                if (perc < 5)
                    perc = 5;
                if (random_int( 100 ) > perc)
                    return false;
            }
        }
    }
    return true;
}

bool NPC::could_move( UFACING dir ) const
{
    unsigned short newx = x + move_delta[ dir ].xmove;
    unsigned short newy = y + move_delta[ dir ].ymove;
    
    short newz;
    UMulti* supporting_multi;
    Item* walkon_item;
    return realm->walkheight( this, newx, newy, z, &newz, &supporting_multi, &walkon_item ) &&
           !npc_path_blocked( dir ) &&
           anchor_allows_move( dir );
}

bool NPC::npc_path_blocked( UFACING dir ) const
{
	if (cached_settings.freemove)
		return false;
    unsigned short newx = x + move_delta[ dir ].xmove;
    unsigned short newy = y + move_delta[ dir ].ymove;
        
    unsigned short wx, wy;
    zone_convert_clip( newx, newy, realm, wx, wy );

    ZoneCharacters& wchr = realm->zone[wx][wy].characters;
    for( ZoneCharacters::iterator itr = wchr.begin(), end = wchr.end(); itr != end; ++itr )
    {
        Character* chr = *itr;
        if (chr->x == newx &&
            chr->y == newy &&
            chr->z >= z-10 && chr->z <= z+10 &&
            !chr->dead() &&
            is_visible_to_me(chr))
        {
            return true;
        }
    }
    return false;
}

void NPC::printOn( ostream& os ) const
{ 
    os << classname() << " " << template_name << pf_endl;
    os << "{" << pf_endl;
    printProperties( os );
    os << "}" << pf_endl;
    os << pf_endl;
}

void NPC::printSelfOn( ostream& os ) const
{
   printOn( os );
}

void NPC::printProperties( std::ostream& os ) const
{
    base::printProperties( os );

	if (registered_house)
		os << "\tRegisteredHouse\t0x" << hex << registered_house << dec << pf_endl;

    if (npc_ar_)
        os << "\tAR\t" << npc_ar_ << pf_endl;

    if (script != "")
        os << "\tscript\t" << script << pf_endl;

    if (master_.get() != NULL)
        os << "\tmaster\t" << master_->serial << pf_endl;

    if (speech_color_ != DEFAULT_TEXT_COLOR)
        os << "\tSpeechColor\t" << speech_color_ << pf_endl;

    if (speech_font_ != DEFAULT_TEXT_FONT)
        os << "\tSpeechFont\t" << speech_font_ << pf_endl;

	if (run_speed != dexterity())
        os << "\tRunSpeed\t" << run_speed << pf_endl;

	if (use_adjustments != true)
        os << "\tUseAdjustments\t" << use_adjustments << pf_endl;

	if (element_resist_.fire != 0)
		os << "\tFireResist\t" << static_cast<int>(element_resist.fire) << pf_endl;
	if (element_resist_.cold  != 0)
		os << "\tColdResist\t" << static_cast<int>(element_resist.cold) << pf_endl;
	if (element_resist_.energy != 0)
		os << "\tEnergyResist\t" << static_cast<int>(element_resist.energy) << pf_endl;
	if (element_resist_.poison != 0)
		os << "\tPoisonResist\t" << static_cast<int>(element_resist.poison) << pf_endl;
	if (element_resist_.physical != 0)
		os << "\tPhysicalResist\t" << static_cast<int>(element_resist.physical) << pf_endl;

	if (element_damage_.fire != 0)
		os << "\tFireDamage\t" << static_cast<int>(element_damage.fire) << pf_endl;
	if (element_damage_.cold  != 0)
		os << "\tColdDamage\t" << static_cast<int>(element_damage.cold) << pf_endl;
	if (element_damage_.energy != 0)
		os << "\tEnergyDamage\t" << static_cast<int>(element_damage.energy) << pf_endl;
	if (element_damage_.poison != 0)
		os << "\tPoisonDamage\t" << static_cast<int>(element_damage.poison) << pf_endl;
	if (element_damage_.physical != 0)
		os << "\tPhysicalDamage\t" << static_cast<int>(element_damage.physical) << pf_endl;

}

void NPC::printDebugProperties( std::ostream& os ) const
{
    base::printDebugProperties( os );
    os << "# template: " << template_.name << pf_endl;
    if (anchor.enabled)
    {
        os << "# anchor: x=" << anchor.x 
           << " y=" << anchor.y 
           << " dstart=" << anchor.dstart 
           << " psub=" << anchor.psub << pf_endl;
    }
}

void NPC::readNpcProperties( ConfigElem& elem )
{
	registered_house = elem.remove_ulong( "REGISTEREDHOUSE", 0 );

    UWeapon* wpn = find_intrinsic_weapon( elem.rest() );
    if (wpn == NULL)
    {
        wpn = create_intrinsic_weapon_from_npctemplate( elem, template_.pkg );
    }
    if ( wpn != NULL )
        weapon = wpn;
    
	// Load the base, equiping items etc will refresh_ar() to update for reals.
	for (int i = 0; i < 6; i++)
	{
		loadResistances( i, elem);
		if ( i > 0)
			loadDamages( i, elem);
	}

	//dave 3/19/3, read templatename only if empty
	if(template_name.empty())
	{
		template_name = elem.rest();
    
		if (template_name == "")
		{
			string tmp;
			if (getprop( "template", tmp))
			{
				template_name = tmp.c_str()+1; 
			}
		}
	}

    unsigned long master_serial;
    if (elem.remove_prop( "MASTER", &master_serial ))
    {
		Character* chr = system_find_mobile(  master_serial );
        if (chr != NULL)
            master_.set( chr );
    }

    script = elem.remove_string( "script", "" );
    if (!script.empty())
        start_script();

    speech_color_ = elem.remove_ushort( "SpeechColor", DEFAULT_TEXT_COLOR );
    speech_font_ = elem.remove_ushort( "SpeechFont", DEFAULT_TEXT_FONT );

	use_adjustments = elem.remove_bool( "UseAdjustments", true );
	run_speed = elem.remove_ushort( "RunSpeed", dexterity() );

	damaged_sound = elem.remove_unsigned("DamagedSound", 0);
}

// This now handles all resistances, including AR to simplify the code.
void NPC::loadResistances( int resistanceType, ConfigElem& elem )
{
	string tmp;
	bool passed = false;
    Dice dice;
    string errmsg;
	// 0 = AR
	// 1 = Fire
	// 2 = Cold
	// 3 = Energy
	// 4 = Poison
	// 5 = Physical
	switch(resistanceType)
	{
		case 0: passed = elem.remove_prop( "AR", &tmp ); break;
		case 1: passed = elem.remove_prop( "FIRERESIST", &tmp ); break;
		case 2: passed = elem.remove_prop( "COLDRESIST", &tmp ); break;
		case 3: passed = elem.remove_prop( "ENERGYRESIST", &tmp ); break;
		case 4: passed = elem.remove_prop( "POISONRESIST", &tmp ); break;
		case 5: passed = elem.remove_prop( "PHYSICALRESIST", &tmp ); break;
	}

    if (passed)
    {
        if (!dice.load( tmp.c_str(), &errmsg ))
        {
            cerr << "Error reading resistance "<< resistanceType << " for NPC: " << errmsg << endl;
            throw runtime_error( "Error loading NPC" );
        }
		switch(resistanceType)
		{
			case 0: npc_ar_ = dice.roll(); break;
			case 1: element_resist_.fire = element_resist.fire = dice.roll(); break;
			case 2: element_resist_.cold = element_resist.cold = dice.roll(); break;
			case 3: element_resist_.energy = element_resist.energy = dice.roll(); break;
			case 4: element_resist_.poison = element_resist.poison = dice.roll(); break;
			case 5: element_resist_.physical = element_resist.physical = dice.roll(); break;
		}
    }
    else
    {
		switch(resistanceType)
		{
			case 0: npc_ar_ = 0; break;
			case 1: element_resist_.fire = element_resist.fire = 0; break;
			case 2: element_resist_.cold = element_resist.cold = 0; break;
			case 3: element_resist_.energy = element_resist.energy = 0; break;
			case 4: element_resist_.poison = element_resist.poison = 0; break;
			case 5: element_resist_.physical = element_resist.physical = 0; break;
		}
    }
}

// This now handles all resistances, including AR to simplify the code.
void NPC::loadDamages( int damageType, ConfigElem& elem )
{
	string tmp;
	bool passed = false;
    Dice dice;
    string errmsg;
	// 1 = Fire
	// 2 = Cold
	// 3 = Energy
	// 4 = Poison
	// 5 = Physical
	switch(damageType)
	{
		case 1: passed = elem.remove_prop( "FIREDAMAGE", &tmp ); break;
		case 2: passed = elem.remove_prop( "COLDDAMAGE", &tmp ); break;
		case 3: passed = elem.remove_prop( "ENERGYDAMAGE", &tmp ); break;
		case 4: passed = elem.remove_prop( "POISONDAMAGE", &tmp ); break;
		case 5: passed = elem.remove_prop( "PHYSICALDAMAGE", &tmp ); break;
	}

    if (passed)
    {
        if (!dice.load( tmp.c_str(), &errmsg ))
        {
            cerr << "Error reading damage "<< damageType << " for NPC: " << errmsg << endl;
            throw runtime_error( "Error loading NPC" );
        }
		switch(damageType)
		{
			case 1: element_damage_.fire = element_damage.fire = dice.roll(); break;
			case 2: element_damage_.cold = element_damage.cold = dice.roll(); break;
			case 3: element_damage_.energy = element_damage.energy = dice.roll(); break;
			case 4: element_damage_.poison = element_damage.poison = dice.roll(); break;
			case 5: element_damage_.physical = element_damage.physical = dice.roll(); break;
		}
    }
    else
    {
		switch(damageType)
		{
			case 1: element_damage_.fire = element_damage.fire = 0; break;
			case 2: element_damage_.cold = element_damage.cold = 0; break;
			case 3: element_damage_.energy = element_damage.energy = 0; break;
			case 4: element_damage_.poison = element_damage.poison = 0; break;
			case 5: element_damage_.physical = element_damage.physical = 0; break;
		}
    }
}

void NPC::readProperties( ConfigElem& elem )
{
	//3/18/3 dave copied this npctemplate code from readNpcProperties, because base::readProperties 
	//will call the exported vital functions before npctemplate is set (distro uses npctemplate in the exported funcs).
    template_name = elem.rest();
    
    if (template_name == "")
    {
        string tmp;
        if (getprop( "template", tmp))
        {
            template_name = tmp.c_str()+1; 
        }
    }
    base::readProperties( elem );
    readNpcProperties( elem );
}

void NPC::readNewNpcAttributes( ConfigElem& elem )
{
    string diestring;
    Dice dice;
    string errmsg;

    for( Attribute* pAttr = FindAttribute(0); pAttr; pAttr = pAttr->next )
    {
        AttributeValue& av = attribute(pAttr->attrid);
        for( unsigned i = 0; i < pAttr->aliases.size(); ++i )
        {
            if (elem.remove_prop( pAttr->aliases[i].c_str(), &diestring ))
            {
                if (!dice.load( diestring.c_str(), &errmsg ) )
                {
                    elem.throw_error( "Error reading Attribute "
                                + pAttr->name +
                                ": " + errmsg );
                }
                long base = dice.roll() * 10;
                if (base > static_cast<long>(ATTRIBUTE_MAX_BASE))
                    base = ATTRIBUTE_MAX_BASE;

                av.base( static_cast<unsigned short>(base) );

                break;
            }
        }
    }
}

void NPC::readPropertiesForNewNPC( ConfigElem& elem )
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
    if (ex != NULL)
    {
        ex->seterror( true );
        ex = NULL;
        // when the NPC executor module destructs, it checks this NPC to see if it points
        // back at it.  If not, it leaves us alone.
    }
    if (!script.empty())
        start_script();
}

void NPC::on_death( Item* corpse )
{
    // base::on_death intentionally not called
    send_remove_character_to_nearby( this );


    corpse->setprop( "npctemplate", "s" + template_name );
	if (FileExists("scripts/misc/death.ecl"))
		::start_script( "misc/death", new EItemRefObjImp( corpse ) );
    
    ClrCharacterWorldPosition( this, "NPC death" );
    if (ex != NULL)
    {
        // this will force the execution engine to stop running this script immediately
        ex->seterror(true);
    }
    
    destroy();
}


void NPC::start_script()
{
    passert( ex == NULL );
    passert( !script.empty() );
    ScriptDef sd( script, template_.pkg, "scripts/ai/" );
    // Log( "NPC script starting: %s\n", sd.name().c_str() );

    ref_ptr<EScriptProgram> prog = find_script2( sd );
        // find_script( "ai/" + script );
    
    if (prog.get() == NULL)
    {
        cerr << "Unable to read script " << sd.name() 
             << " for NPC " << name() << "(" << hexint(serial) << ")" << endl;
        throw runtime_error( "Error loading NPCs" );
    }

	ex = create_script_executor();
	ex->addModule( new NPCExecutorModule( *ex, *this ) );
    UOExecutorModule* uoemod = new UOExecutorModule( *ex );
    ex->addModule( uoemod );
	if (ex->setProgram( prog.get() ) == false)
    {
        cerr << "There was an error running script " << script << " for NPC "
            << name() << "(0x" << hex << serial << dec << ")" << endl;
        throw runtime_error( "Error loading NPCs" );
    }

	uoemod->attached_npc_ = this;

	schedule_executor( ex );
}


bool NPC::can_be_renamed_by( const Character* chr ) const
{
    return (master_.get() == chr);
}

void NPC::on_give_item()
{
}



void NPC::on_pc_spoke( Character* src_chr, const char* speech, u8 texttype)
{
    /*
    cerr << "PC " << src_chr->name()
         << " spoke in range of NPC " << name() 
         << ": '" << speech << "'" << endl;
         */

    if (ex != NULL)
    {
		if ((ex->eventmask & EVID_SPOKE) &&
            inrangex( this, src_chr, ex->speech_size ))
        {
            if ( (!ssopt.event_visibility_core_checks) || is_visible_to_me( src_chr ) )
				ex->os_module->signal_event( new SpeechEvent( src_chr, speech,
															  TextTypeToString(texttype) ) ); //DAVE added texttype
        }
    }
}

void NPC::on_ghost_pc_spoke( Character* src_chr, const char* speech, u8 texttype)
{
    if (ex != NULL)
    {
        if ((ex->eventmask & EVID_GHOST_SPEECH) &&
            inrangex( this, src_chr, ex->speech_size ))
        {
            if ( (!ssopt.event_visibility_core_checks) || is_visible_to_me( src_chr ) )
				ex->os_module->signal_event( new SpeechEvent( src_chr, speech,
															  TextTypeToString(texttype) ) ); //DAVE added texttype
        }
    }
}

void NPC::on_pc_spoke( Character *src_chr, const char *speech, u8 texttype,
					   const u16* wspeech, const char lang[4])
{
    if (ex != NULL)
    {
        if ((ex->eventmask & EVID_SPOKE) &&
            inrangex( this, src_chr, ex->speech_size ))
        {
            if ( (!ssopt.event_visibility_core_checks) || is_visible_to_me( src_chr ))
				ex->os_module->signal_event( new UnicodeSpeechEvent( src_chr, speech,
																	 TextTypeToString(texttype),
																	 wspeech, lang ) );
        }
    }
}

void NPC::on_ghost_pc_spoke( Character* src_chr, const char* speech, u8 texttype,
							 const u16* wspeech, const char lang[4])
{
    if (ex != NULL)
    {
        if ((ex->eventmask & EVID_GHOST_SPEECH) &&
            inrangex( this, src_chr, ex->speech_size ))
        {
            if ( (!ssopt.event_visibility_core_checks) || is_visible_to_me( src_chr ))
				ex->os_module->signal_event( new UnicodeSpeechEvent( src_chr, speech,
																	 TextTypeToString(texttype),
																	 wspeech, lang ) );
        }
    }
}

void NPC::inform_engaged( Character* engaged )
{
    // someone has targetted us. Create an event if appropriate.
    if (ex != NULL)
    {
        if (ex->eventmask & EVID_ENGAGED)
        {
            ex->os_module->signal_event( new EngageEvent( engaged ) );
        }
    }
    // Note, we don't do the base class thing, 'cause we have no client.
}

void NPC::inform_disengaged( Character* disengaged )
{
    // someone has targetted us. Create an event if appropriate.
    if (ex != NULL)
    {
        if (ex->eventmask & EVID_DISENGAGED)
        {
            ex->os_module->signal_event( new DisengageEvent( disengaged ) );
        }
    }
    // Note, we don't do the base class thing, 'cause we have no client.
}

void NPC::inform_criminal( Character* thecriminal )
{
    if (ex != NULL)
    {
        if ((ex->eventmask & (EVID_GONE_CRIMINAL)) && inrangex( this, thecriminal, ex->area_size ))
        {
            if ( (!ssopt.event_visibility_core_checks) || is_visible_to_me( thecriminal ))
                ex->os_module->signal_event( new SourcedEvent( EVID_GONE_CRIMINAL, thecriminal ) );
        }
	}
}

void NPC::inform_leftarea( Character* wholeft )
{
	if (ex != NULL)
    {
        if ( ex->eventmask & (EVID_LEFTAREA) )
        {
			if ( pol_distance(this, wholeft) <= ex->area_size )
                ex->os_module->signal_event( new SourcedEvent( EVID_LEFTAREA, wholeft ) );
        }
	}
}

void NPC::inform_moved( Character* moved )
{
	// 8-26-05    Austin
	// Note: This does not look at realms at all, just X Y coords.

    if (ex != NULL)
    {
        if ((moved == opponent_) && (ex->eventmask & (EVID_OPPONENT_MOVED)))
        {
            if ( (!ssopt.event_visibility_core_checks) || is_visible_to_me( moved ) )
                ex->os_module->signal_event( new SourcedEvent( EVID_OPPONENT_MOVED, moved ) );
        }
        else if ( ex->eventmask & (EVID_ENTEREDAREA) )
        {
            passert( this != NULL );
            passert( moved != NULL );
                // egcs may have a compiler bug when calling these as inlines
            bool are_inrange = (abs( x - moved->x ) <= ex->area_size) &&
                               (abs( y - moved->y ) <= ex->area_size);

                // inrangex_inline( this, moved, ex->area_size );
            bool were_inrange =(abs( x - moved->lastx ) <= ex->area_size) &&
		                       (abs( y - moved->lasty ) <= ex->area_size);
                
                // inrangex_inline( this, moved->lastx, moved->lasty, ex->area_size );
            if ( are_inrange && !were_inrange && (ex->eventmask & (EVID_ENTEREDAREA)) )
            {
                if ( (!ssopt.event_visibility_core_checks) || is_visible_to_me( moved ) )
                        ex->os_module->signal_event( new SourcedEvent( EVID_ENTEREDAREA, moved ) );
            }
        }
        else if ( ex->eventmask & (EVID_LEFTAREA) )
        {
            passert( this != NULL );
            passert( moved != NULL );
                // egcs may have a compiler bug when calling these as inlines
            bool are_inrange = (abs( x - moved->x ) <= ex->area_size) &&
                               (abs( y - moved->y ) <= ex->area_size);

                // inrangex_inline( this, moved, ex->area_size );
            bool were_inrange =(abs( x - moved->lastx ) <= ex->area_size) &&
		                       (abs( y - moved->lasty ) <= ex->area_size);
                
            if ( !are_inrange && were_inrange && (ex->eventmask & (EVID_LEFTAREA)) )
            {
				if ( (!ssopt.event_visibility_core_checks) || is_visible_to_me( moved ) )
                        ex->os_module->signal_event( new SourcedEvent( EVID_LEFTAREA, moved ) );
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

		// egcs may have a compiler bug when calling these as inlines
        bool are_inrange = (abs( x - chr->x ) <= ex->area_size) &&
							(abs( y - chr->y ) <= ex->area_size);
            
        bool were_inrange =(abs( lastx - chr->x ) <= ex->area_size) &&
							(abs( lasty - chr->y ) <= ex->area_size);

		if ( ex->eventmask & (EVID_ENTEREDAREA) )
        {
			if ( are_inrange && !were_inrange )
            {
               if ( (!ssopt.event_visibility_core_checks) || is_visible_to_me( chr ) )
                        ex->os_module->signal_event( new SourcedEvent( EVID_ENTEREDAREA, chr ) );
            }
        }
        else if ( ex->eventmask & (EVID_LEFTAREA) )
        {
            if (!are_inrange && were_inrange)
            {
                if ( (!ssopt.event_visibility_core_checks) || is_visible_to_me( chr ))
                        ex->os_module->signal_event( new SourcedEvent( EVID_LEFTAREA, chr ) );
            }
        }
	}
}

bool NPC::can_accept_event( EVENTID eventid )
{
    if (ex == NULL)
        return false;
    if (ex->eventmask & eventid)
        return true;
    else
        return false;
}

BObjectImp* NPC::send_event( BObjectImp* event )
{
    if (ex != NULL)
    {
        if (ex->os_module->signal_event( event ))
			return new BLong(1);
		else
			return new BError( "Event queue is full, discarding event" );
    }
    else
    {
        BObject bo( event );
        return new BError( "That NPC doesn't have a control script" );
    }
}

void NPC::apply_raw_damage_hundredths( unsigned long damage, Character* source, bool userepsys )
{
    if (ex != NULL)
    {
        if (ex->eventmask & EVID_DAMAGED)
        {
            ex->os_module->signal_event( new DamageEvent( source, static_cast<unsigned short>(damage/100) ) );
        }
    }

    base::apply_raw_damage_hundredths( damage, source, userepsys );
}

/*
void NPC::on_swing_failure( Character* attacker )
{
    base::on_swing_failure(attacker);
}
*/

// keep this in sync with Character::armor_absorb_damage
double NPC::armor_absorb_damage( double damage )
{
    if (!npc_ar_)
    {
        return base::armor_absorb_damage( damage );
    }
    else
    {
        int blocked = npc_ar_ + ar_mod();
        if (blocked < 0) blocked = 0;
        int absorbed = blocked / 2;
        
        blocked -= absorbed;
        absorbed += random_int( blocked+1 );
        if (watch.combat) cout << absorbed << " hits absorbed by NPC armor." << endl;
        damage -= absorbed;
        if (damage < 0)
            damage = 0;
    }
    return damage;
}

unsigned short calc_thru_damage( double damage, unsigned short ar );

void NPC::get_hitscript_params( double damage,
                                UArmor** parmor,
                                unsigned short* rawdamage )
{
    if (!npc_ar_)
    {
        base::get_hitscript_params( damage, parmor, rawdamage );
    }
    else
    {
        *rawdamage = static_cast<unsigned short>(calc_thru_damage( damage, npc_ar_ + ar_mod() ));
    }
}

UWeapon* NPC::intrinsic_weapon()
{
    if (template_.intrinsic_weapon)
        return template_.intrinsic_weapon;
    else
        return wrestling_weapon;
}

struct ArmorZone {
	string name;
	double chance;
	vector<unsigned short> layers;
};
typedef vector<ArmorZone> ArmorZones;
extern ArmorZones armorzones;

void NPC::refresh_ar()
{
	// This is an npc, we need to check to see if any armor is being wore
	// otherwise we just reset this to the base values from their template.
	bool hasArmor = false;
	for( unsigned layer = LAYER_EQUIP__LOWEST; layer <= LAYER_EQUIP__HIGHEST; ++layer )
	{
		Item *item = wornitems.GetItemOnLayer( layer );
		if (item == NULL)
			continue;
		for( unsigned element = 0; element <= ELEMENTAL_TYPE_MAX; ++element )
		{
			if (item->calc_element_resist( element ) != 0 || item->calc_element_damage( element )!= 0 )
			{
				hasArmor = true;
				break;
				}
		}
	}

	if ( !hasArmor )
	{
		ar_ = 0;
		for( unsigned element = 0; element <= ELEMENTAL_TYPE_MAX; ++element )
		{
			reset_element_resist(element);
			reset_element_damage(element);
		}
		return;
	}

	for( unsigned zone = 0; zone < armorzones.size(); ++zone )
		armor_[ zone ] = NULL;
	// we need to reset each resist to 0, then add the base back using calc.
	for( unsigned element = 0; element <= ELEMENTAL_TYPE_MAX; ++element )
	{
		refresh_element(element);
	}

	for( unsigned layer = LAYER_EQUIP__LOWEST; layer <= LAYER_EQUIP__HIGHEST; ++layer )
	{
		Item *item = wornitems.GetItemOnLayer( layer );
		if (item == NULL)
			continue;
		// Let's check all items as base, and handle their element_resists.
		for( unsigned element = 0; element <= ELEMENTAL_TYPE_MAX; ++element )
		{
			update_element(element, item);
		}
		if (item->isa( CLASS_ARMOR ))
		{
			UArmor* armor = static_cast<UArmor*>(item);
			std::set<unsigned short> tmplzones = armor->tmplzones();
			std::set<unsigned short>::iterator itr;
			for ( itr = tmplzones.begin(); itr != tmplzones.end(); ++itr )
			{
				if ((armor_[*itr] == NULL) || (armor->ar() > armor_[*itr]->ar()))
					armor_[*itr] = armor;
			}
		}
	}

	double new_ar = 0.0;
	for( unsigned zone = 0; zone < armorzones.size(); ++zone )
	{
		UArmor* armor = armor_[ zone ];
		if (armor != NULL)
		{
			new_ar += armor->ar() * armorzones[ zone ].chance;
		}
	}

	/* add AR due to shield : parry skill / 2 is percent of AR */
	// FIXME: Should we allow this to be adjustable via a prop? Hrmmmmm
	if (shield != NULL)
	{
		double add = shield->ar() * attribute(pAttrParry->attrid).effective() * 0.5 * 0.01;
		if (add > 1.0)
			new_ar += add;
		else
			new_ar += 1.0;
	}

	new_ar += ar_mod();

	short s_new_ar = static_cast<short>(new_ar);
	if (s_new_ar >= 0)
		ar_ = s_new_ar;
	else
		ar_ = 0;

}

void NPC::reset_element_resist( unsigned resist )
{
	switch(resist)
	{
	case ELEMENTAL_FIRE: element_resist.fire = element_resist_.fire + element_resist_mod.fire; break;
	case ELEMENTAL_COLD: element_resist.cold = element_resist_.cold + element_resist_mod.cold; break;
	case ELEMENTAL_ENERGY: element_resist.energy = element_resist_.energy + element_resist_mod.energy; break;
	case ELEMENTAL_POISON: element_resist.poison = element_resist_.poison + element_resist_mod.poison; break;
	case ELEMENTAL_PHYSICAL: element_resist.physical = element_resist_.physical + element_resist_mod.physical; break;
	}
}

void NPC::reset_element_damage( unsigned damage )
{
	switch(damage)
	{
	case ELEMENTAL_FIRE: element_damage.fire = element_damage_.fire + element_damage_mod.fire; break;
	case ELEMENTAL_COLD: element_damage.cold = element_damage_.cold + element_damage_mod.cold; break;
	case ELEMENTAL_ENERGY: element_damage.energy = element_damage_.energy + element_damage_mod.energy; break;
	case ELEMENTAL_POISON: element_damage.poison = element_damage_.poison + element_damage_mod.poison; break;
	case ELEMENTAL_PHYSICAL: element_damage.physical = element_damage_.physical + element_damage_mod.physical; break;
	}
}