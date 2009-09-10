/*
History
=======
2003/12/24 Dave:      on_poison_changed() changed to keep life bars from disappearing when you poison someone.
2005/03/09 Shinigami: Added Prop Delay_Mod [ms] for WeaponDelay (see schedule_attack too)
2005/06/01 Shinigami: Added Walking_Mounted and Running_Mounted movecosts
2005/09/14 Shinigami: Character::resurrect() - Vital.regen_while_dead implemented
2005/10/14 Shinigami: fixed missing init of Character::dblclick_wait
2005/11/23 MuadDib:	  Added warmode_wait object for characters.
2005/11/25 MuadDib:   Added realm check to is_visible_to_me.
2005/12/06 MuadDib:   Added uclang member for storing UC language from client.
2006/03/10 MuadDib:   Added NoCombat support to checking of justice region.
2006/05/04 MuadDib:   Removed get_legal_item for no use.
2006/05/04 MuadDib:   SkillValue() removed for no use.
2006/05/16 Shinigami: UOBJ_*MALE_GHOST renamed to UOBJ_HUMAN_*MALE_GHOST
                      added Prop Race (RACE_* Constants) to support Elfs
                      Character::die(), Character::doors_block() and Character::resurrect() updated
2008/07/08 Turley:    get_flag1() changed to show WarMode of other player again
                      get_flag1_aos() removed
2009/01/14 Nando:     setgraphic() changed to allow graphics up to 2048 (0x800).
2009/02/01 MuadDib:   Resistance storage added.
2009/02/25 MuadDib:   on_poison_changed() added UOKR Status bar update for poisoned. Booyah!
2009/07/20 MuadDib:   Slot checks added to Character::Die()
2009/07/25 MuadDib:   equippable() now checks if a twohanded is intrinsic or not also. Intrinsic gets ignored
2009/07/31 Turley:    added check for cmbtcfg::send_swing_packet & reset_swing_onturn
2009/08/04 MuadDib:   calc_vital_stuff() now checks to see if a vital changed, before using tell_vital_changed()
2009/08/06 MuadDib:   Addeed gotten_by code for items.
2009/08/07 MuadDib:   Added new Corpse Layer code to character Die() to put equipped items on correct layer with
                      corpse. Ignores items from pack, because in death we only want the items it had equipped in
					  life, showing up on the corpse when dead.
2009/08/09 MuadDib:   on_poison_changed() rewritten for better KR poison support.
2009/08/16 MuadDib:   fix for die() where checking corpse and not item for slot_index().
2009/08/25 Shinigami: STLport-5.2.1 fix: corpseSlot not used
                      STLport-5.2.1 fix: init order changed of party_can_loot_, party_decline_timeout_ and skillcap_
2009/08/28 Turley:    Crashfix for Character::on_poison_changed()
2009/09/03 MuadDib:   Changed combat related ssopt stuff to combat_config.
                      Changes for account related source file relocation
					  Changes for multi related source file relocation
2009/09/09 Turley:    ServSpecOpt CarryingCapacityMod as * modifier for Character::carrying_capacity()

Notes
=======

*/

#include "../../clib/stl_inc.h"

#ifdef _MSC_VER
#pragma warning( disable: 4786 )
#endif

#include <assert.h>

#include "../../clib/endian.h"
#include "../../clib/cfgelem.h"
#include "../../clib/cfgfile.h"
#include "../../clib/esignal.h"
#include "../../clib/fileutil.h"
#include "../../clib/logfile.h"
#include "../../clib/passert.h"
#include "../../clib/random.h"
#include "../../clib/stlutil.h"
#include "../../clib/strutil.h"

#include "../../plib/mapcell.h"
#include "../uconst.h"
#include "../../plib/realm.h"

#include "../accounts/account.h"
#include "../action.h"
#include "../anim.h"
#include "../item/armor.h"
#include "attribute.h"
#include "../multi/boat.h"
#include "../network/cgdata.h"
#include "../checkpnt.h"
#include "../clidata.h"
#include "../network/client.h"
#include "../network/cliface.h"
#include "../cmdlevel.h"
#include "../cmbtcfg.h"
#include "../dtrace.h"
#include "../fnsearch.h"
#include "../gflag.h"
#include "../module/guildmod.h"
#include "../los.h"
#include "../mkscrobj.h"
#include "../movecost.h"
#include "../objtype.h"
#include "../module/osmod.h"
#include "../party.h"
#include "../pktout.h"
#include "../pktboth.h"
#include "../polcfg.h"
#include "../polclass.h"
#include "../polclock.h"
#include "../polsig.h"
#include "../polvar.h"
#include "../profile.h"
#include "../realms.h"
#include "../schedule.h"
#include "../scrsched.h"
#include "../scrstore.h"
#include "../sfx.h"
#include "../skilladv.h"
#include "../skills.h"
#include "../spelbook.h"
#include "../ssopt.h"
#include "../statmsg.h"
#include "../syshook.h"
#include "../target.h"
#include "../uconst.h"
#include "../ufunc.h"
#include "../ufuncstd.h"
#include "../umanip.h"
#include "../uobjcnt.h"
#include "../module/uomod.h"
#include "../uoexec.h"
#include "../uofile.h"
#include "../uoscrobj.h"
#include "../uvars.h"
#include "../uworld.h"
#include "../vital.h"
#include "../watch.h"
#include "../item/weapon.h"
#include "../item/wepntmpl.h"

#include "charactr.h"


bool DecodeFacing( const char* dir, UFACING& facing )
{
    if (stricmp(dir, "N") == 0)			facing = FACING_N;
    else if (stricmp(dir, "S") == 0)	facing = FACING_S;
    else if (stricmp(dir, "E") == 0)	facing = FACING_E;
    else if (stricmp(dir, "W") == 0)	facing = FACING_W;
    else if (stricmp(dir, "NE") == 0)	facing = FACING_NE;
    else if (stricmp(dir, "SE") == 0)	facing = FACING_SE;
    else if (stricmp(dir, "SW") == 0)	facing = FACING_SW;
    else if (stricmp(dir, "NW") == 0)	facing = FACING_NW;
    else return false;
    return true;
}

struct ArmorZone {
	string name;
	double chance;
	vector<unsigned short> layers;
};
typedef vector<ArmorZone> ArmorZones;
ArmorZones armorzones;
double armor_zone_chance_sum;

unsigned short layer_to_zone( unsigned short layer )
{
	for( unsigned short zone = 0; zone < armorzones.size(); ++zone )
	{
		for( unsigned i = 0; i < armorzones[zone].layers.size(); ++i )
		{
			if (armorzones[zone].layers[i] == layer)
				return zone;
		}
	}
	cerr << "Couldn't find an Armor Zone in armrzone.cfg for layer " << layer << endl;	
	throw runtime_error( "Configuration file error" );
	return 0;
}

const char* zone_to_zone_name( unsigned short zone )
{
	return armorzones[zone].name.c_str();
}

unsigned short zone_name_to_zone( const char *zname )
{
	for( unsigned short zone = 0; zone < armorzones.size(); ++zone )
	{
		if ( stricmp( armorzones[zone].name.c_str(), zname ) == 0)
		{
			return zone;
		}
	}
	cerr << "Couldn't find an armrzone.cfg config elem named '" << zname << "'" << endl;
	
	throw runtime_error( "Configuration file error" );
	return 0;
}
void load_armor_zones()
{
	if ( !FileExists("config/armrzone.cfg") )
	{
		if ( config.loglevel > 1 )
			cout << "File config/armrzone.cfg not found, skipping.\n";
		return;
	}
	ConfigFile cf("config/armrzone.cfg");
	ConfigElem elem;

	armor_zone_chance_sum = 0;
	armorzones.clear();

	while ( cf.read(elem) )
	{
		// armorzones.push_back( ArmorZone( elem ) );
		ArmorZone az;
		az.name = elem.remove_string("NAME");
		az.chance = static_cast<double>(elem.remove_ushort("CHANCE")) / 100.0;
		unsigned short in_layer;
		while ( elem.remove_prop("LAYER", &in_layer) )
		{
			if ( in_layer < LOWEST_LAYER || in_layer > HIGHEST_LAYER )
			{
				cerr << "ArmorZone " << az.name 
					 << ": Layer " << in_layer 
					 << " is out of range." 
					 << endl;
				cerr << "Valid range is " << LOWEST_LAYER
					 << " to " << HIGHEST_LAYER
					 << endl;
				throw runtime_error("Configuration file error");
			}
			az.layers.push_back(in_layer);
		}

		armorzones.push_back(az);
		armor_zone_chance_sum += az.chance;
	}
}

void unload_armor_zones()
{
	armorzones.clear();
	armor_zone_chance_sum = 0;
}

Character::Character( u16 objtype, UOBJ_CLASS uobj_class ) :
	UObject( objtype, uobj_class ),
	warmode_wait(0),
	trading_with(NULL),
	trade_accepted(false),
	acct(NULL),
	client(NULL),
	cmdlevel(0),
	dir(0),
	warmode(false),
	logged_in(true), // so initialization scripts etc can see
	connected(false), // but EScript "check"value false
	lastx(0),
	lasty(0),
	lastz(0),
	move_reason(OTHER),
	movemode( MOVEMODE_LAND ),
	disable_regeneration_until(0),
	disable_skills_until(0),
	truecolor(0),
	trueobjtype(0),
		// Note, Item uses the named constructor idiom, but here, it is not used.
		// this is probably okay, but something to keep in mind.
	gender(GENDER_MALE),
	race(RACE_HUMAN),
	poisoned(false),
	last_corpse(0),
	dblclick_wait(0),
	gotten_item(NULL),
	gotten_item_source(0),
	attributes(numAttributes),
	vitals(numVitals),

	//target_cursor_uoemod(NULL),
	//menu_selection_uoemod(NULL),
	//prompt_uoemod(NULL),
	uclang("enu"),
	tcursor2(NULL),
	menu(NULL),
	on_menu_selection(NULL),
	lightoverride(-1),
	lightoverride_until(0),

	skillcap_(700),

	wornitems_ref( new WornItemsContainer ),// default objtype is in containr.cpp, WornItemsContainer class
	wornitems( *wornitems_ref ), 
	
	ar_mod_(0),
	delay_mod_(0),
	hitchance_mod_(0),
	evasionchance_mod_(0),

	weapon(wrestling_weapon),
	shield(NULL),
	armor_( armorzones.size() ),
	
	dead_(false),
	hidden_(false),
	concealed_(0), //Dave 11/25 changed from "false" - concealed_ is a char not bool
	frozen_(false),
	paralyzed_(false),

	stealthsteps_(0),

	// private_items_ default
	// additional_legal_items default
	// privs default
	// settings default
	// cached_settings struct

	ar_(0),
	script_ex(NULL),
	opponent_(NULL),
	// opponent_of default

	swing_timer_start_clock_(0),
	ready_to_swing(false),
	swing_task(NULL),
	spell_task(NULL),
	created_at(0),
	squelched_until(0),

	criminal_until_(0),
	repsys_task_(NULL),
	to_be_reportable_(),
	reportable_(),
	guild_(NULL),
	party_(NULL),
	candidate_of_(NULL),
	offline_mem_of_(NULL),
	party_can_loot_(false),
	party_decline_timeout_(NULL),
	murderer_(false),
	langid_(0)
{
	gradual_boost = 0;
	height = PLAYER_CHARACTER_HEIGHT; //this gets overwritten in UObject::readProperties!
	wornitems.chr_owner = this; //FIXME, dangerous.
	wornitems.SetInitialSize();

	set_caps_to_default();

	expanded_statbar.statcap = 225;
	expanded_statbar.luck = 0;
	expanded_statbar.dmg_min = 0;
	expanded_statbar.dmg_max = 0;
	expanded_statbar.followers_max = 0;
	expanded_statbar.tithing = 0;
	expanded_statbar.followers = 0;

	load_default_elements();

	// vector
	refresh_cached_settings();
	++ucharacter_count;
}

Character::~Character()
{
	if (acct.get())
	{
		if (acct->active_character == this)
			acct->active_character = NULL;
		for( int i = 0; i < config.character_slots; i++ )
		{
			if (acct->get_character( i ) == this)
			{
				acct->clear_character( i );
			}
		}
	}
	acct.clear();

	if (client && (client->chr == this))
		client->chr = NULL;
	client = NULL;

	// It might be nice to do this only when the system isn't shutting down...
	//if (!opponent_of.empty())
	//{
	//	Log( "Destroying character with nonempty opponent_of! (But cleaning up..)\n" );
	//}

	removal_cleanup();

	//
	// this lets normal destroy() stuff happen (like destroying stuff in containers)
	// but still lets the Character destructor delete the wornitems object.
	//
	//wornitems.ref_counted_add_ref();
	wornitems.destroy();
	//wornitems.ref_counted_release();

	if (repsys_task_ != NULL)
		repsys_task_->cancel();

	if (party_decline_timeout_ != NULL)
		party_decline_timeout_->cancel();

	--ucharacter_count;
}

void Character::removal_cleanup()
{
	clear_opponent_of();

	/* This used to be a call to set_opponent(NULL),
	   which was slick, 
	   but that was sending disengage events, which were
	   trying to resurrect this object. (C++)
	 */
	if (opponent_ != NULL)
	{
		opponent_->opponent_of.erase( this );
//		This is cleanup, wtf we doing trying to send highlights?!
//		opponent_->send_highlight();
//		opponent_->schedule_attack();
		opponent_ = NULL;
	}

	if (swing_task != NULL)
		swing_task->cancel();

	disconnect_cleanup();
}

void cancel_trade( Character* chr1 );

void Character::disconnect_cleanup()
{
	if (is_trading())
		cancel_trade( this );

	tcursor2 = NULL;

	stop_skill_script();
	on_loggoff_party( this );
}

bool Character::has_active_client() const
{
	return (client && client->ready && !client->disconnect);
}

bool Character::has_active_prompt() const
{
	return (client && client->gd &&
			(client->gd->prompt_uniemod || client->gd->prompt_uoemod));
}

bool Character::has_active_gump() const
{
	return (client && client->gd && !client->gd->gumpmods.empty());
}

void Character::clear_gotten_item()
{
	if (gotten_item != NULL)
	{
		Item* item = gotten_item;
		gotten_item = NULL;
		item->inuse( false );
		item->gotten_by = NULL;
		if (connected)
			send_item_move_failure( client, MOVE_ITEM_FAILURE_UNKNOWN );
		undo_get_item( this, item );
	}
}

void Character::destroy()
{
	stop_skill_script();
	base::destroy();
}

void Character::stop_skill_script()
{
	if (script_ex != NULL)
	{
		deschedule_executor( script_ex );
		delete script_ex;
		passert( script_ex == NULL );
	}
}



u8 Character::los_height() const
{
	return height;
}

///
/// A Mobile's weight is 10 stones + the weight of their equipment.
///
unsigned long Character::weight() const
{
	unsigned long wt = 10 + wornitems.weight();
	if (gotten_item)
		wt += gotten_item->weight();
	if (trading_cont.get())
		wt += trading_cont->weight();
	return wt;
}

///
/// A Mobile's carrying capacity is 40 + STR * 3.5 stones.
///
unsigned short Character::carrying_capacity() const
{
	return static_cast<u16>(floor( (40 + strength() * 7 / 2) * ssopt.carrying_capacity_mod ));
}

int Character::charindex() const
{
	if (acct == NULL)
		return -1;

	for( int i = 0; i < config.character_slots; i++ )
	{
		if (acct->get_character( i ) == this)
			return i;
	}

	Log2( "Warning: Can't find charidx for Character 0x%lx\n", serial );
	return -1;
}



void Character::printProperties( ostream& os ) const
{
	if (acct != NULL)
	{
		os << "\tAccount\t" << acct->name() << pf_endl;
		os << "\tCharIdx\t" << charindex() << pf_endl;
	}

	base::printProperties(os);
 
	if (cmdlevel)
	{
		os << "\tCmdLevel\t" << cmdlevels2[cmdlevel].name << pf_endl;
	}
	if (concealed_)
	{
		os << "\tConcealed\t" << int(concealed_) << pf_endl;
	}
	os << "\tTrueColor\t0x" << hex << truecolor << dec << pf_endl;
	os << "\tTrueObjtype\t0x" << hex << trueobjtype << dec << pf_endl;

	os << "\tGender\t" << static_cast<int>(gender) << pf_endl;
	os << "\tRace\t" << static_cast<int>(race) << pf_endl;
	
	if (dead_)
		os << "\tDead\t" << static_cast<int>(dead_) << pf_endl;

	if (hidden_)
		os << "\tHidden\t" << static_cast<int>(hidden_) << pf_endl;

	if (frozen_)
		os << "\tFrozen\t" << static_cast<int>(frozen_) << pf_endl;

	if (element_resist_mod.fire != 0)
		os << "\tFireResistMod\t" << static_cast<int>(element_resist_mod.fire) << pf_endl;
	if (element_resist_mod.cold  != 0)
		os << "\tColdResistMod\t" << static_cast<int>(element_resist_mod.cold) << pf_endl;
	if (element_resist_mod.energy != 0)
		os << "\tEnergyResistMod\t" << static_cast<int>(element_resist_mod.energy) << pf_endl;
	if (element_resist_mod.poison != 0)
		os << "\tPoisonResistMod\t" << static_cast<int>(element_resist_mod.poison) << pf_endl;
	if (element_resist_mod.physical != 0)
		os << "\tPhysicalResistMod\t" << static_cast<int>(element_resist_mod.physical) << pf_endl;

	if (element_damage_mod.fire != 0)
		os << "\tFireDamageMod\t" << static_cast<int>(element_damage_mod.fire) << pf_endl;
	if (element_damage_mod.cold  != 0)
		os << "\tColdDamageMod\t" << static_cast<int>(element_damage_mod.cold) << pf_endl;
	if (element_damage_mod.energy != 0)
		os << "\tEnergyDamageMod\t" << static_cast<int>(element_damage_mod.energy) << pf_endl;
	if (element_damage_mod.poison != 0)
		os << "\tPoisonDamageMod\t" << static_cast<int>(element_damage_mod.poison) << pf_endl;
	if (element_damage_mod.physical != 0)
		os << "\tPhysicalDamageMod\t" << static_cast<int>(element_damage_mod.physical) << pf_endl;

	if (movement_cost.walk != 1.0)
		os << "\tMovementWalkMod\t" << static_cast<double>(movement_cost.walk) << pf_endl;
	if (movement_cost.run != 1.0)
		os << "\tMovementRunMod\t" << static_cast<double>(movement_cost.run) << pf_endl;
	if (movement_cost.walk_mounted != 1.0)
		os << "\tMovementWalkMountedMod\t" << static_cast<double>(movement_cost.walk_mounted) << pf_endl;
	if (movement_cost.run_mounted != 1.0)
		os << "\tMovementRunMountedMod\t" << static_cast<double>(movement_cost.run_mounted) << pf_endl;


	// output Attributes
	for( Attribute* pAttr = FindAttribute(0); pAttr != NULL; pAttr = pAttr->next )
	{
		const AttributeValue& av = attribute(pAttr->attrid);
		short lock = av.lock();
		unsigned cap = av.cap();

		if (av.base() || lock || cap != pAttr->default_cap) // it kind of floods the file... but... :/ (Nando)
		{
			unsigned ones = av.base()/10;
			unsigned tenths = av.base()%10;
			os << "\t" << pAttr->name << "\t" << ones;
			if (tenths)
				os << "." << tenths;
			
			if (cap != pAttr->default_cap) {
				unsigned cap_ones = cap/10;
				unsigned cap_tenths = cap%10;

				os << ":" << cap_ones;
				if (tenths)
					os << "." << cap_tenths;
			}

			if (lock)
				os << ";" << lock;

			os << pf_endl;
		}
	}

	if (skillcap_ != 0)
		os << "\tSkillcap\t" << static_cast<int>(skillcap_) << pf_endl;
	
	// output Vitals
	for( Vital* pVital = FindVital(0); pVital != NULL; pVital = pVital->next )
	{
		const VitalValue& vv = vital(pVital->vitalid);
		if (vv.current_ones())
		{
			os << "\t" << pVital->name << "\t" << vv.current_ones() << pf_endl;
		}
	}

	if (expanded_statbar.statcap != 0)
		os << "\tStatcap\t" << static_cast<int>(expanded_statbar.statcap) << pf_endl;

	if (expanded_statbar.luck != 0)
		os << "\tLuck\t" << static_cast<int>(expanded_statbar.luck) << pf_endl;
	if (expanded_statbar.followers_max != 0)
		os << "\tFollowersMax\t" << static_cast<int>(expanded_statbar.followers_max) << pf_endl;
	if (expanded_statbar.tithing != 0)
		os << "\tTithing\t" << static_cast<int>(expanded_statbar.tithing) << pf_endl;
	if (expanded_statbar.followers != 0)
		os << "\tFollowers\t" << static_cast<int>(expanded_statbar.followers) << pf_endl;

	if (movemode != MOVEMODE_LAND)
		os << "\tMoveMode\t" << encode_movemode( movemode ) << pf_endl;

	if (!privs.empty())
	{
		os << "\tPrivs\t" << privs.extract() << pf_endl;
	}
	if (!settings.empty())
	{
		os << "\tSettings\t" << settings.extract() << pf_endl;
	}

	os << "\tCreatedAt\t" << created_at << pf_endl;

	if (squelched_until)
		os << "\tSquelchedUntil\t" << squelched_until << pf_endl;

	if (!title_prefix.empty())
		os << "\tTitlePrefix\t" << getencodedquotedstring(title_prefix) << pf_endl;
	if (!title_suffix.empty())
		os << "\tTitleSuffix\t" << getencodedquotedstring(title_suffix) << pf_endl;
	if (!title_guild.empty())
		os << "\tTitleGuild\t" << getencodedquotedstring(title_guild) << pf_endl;
	if (!title_race.empty())
		os << "\tTitleRace\t" << getencodedquotedstring(title_race) << pf_endl;

//	if (guildid_)
//		os << "\tGuildId\t" << guildid_ << pf_endl;
	if (murderer_)
		os << "\tMurderer\t" << murderer_ << pf_endl;
	if (party_can_loot_)
		os << "\tPartyCanLoot\t" << party_can_loot_ << pf_endl;
	for(ReportableList::const_iterator itr = reportable_.begin();
		itr != reportable_.end();
		++itr)
	{
		const reportable_t& rt = (*itr);
		os << "\tReportable\t" << hexint(rt.serial) << " " << rt.polclock << pf_endl;
	}
	
	UCorpse* corpse_obj = static_cast<UCorpse*>(system_find_item(last_corpse));
	if ( corpse_obj != NULL && !corpse_obj->orphan() )
		os << "\tLastCorpse\t" << last_corpse << pf_endl;
}
void Character::printDebugProperties( std::ostream& os ) const
{
	base::printDebugProperties( os );
}

const char *Character::classname() const
{
	return "Character";
}

void Character::printSelfOn( ostream& os ) const
{
	base::printOn( os );
}

void Character::printOn( ostream& os ) const
{ 
	base::printOn( os );
}
void Character::printWornItems( ostream& ofs_pc, ostream& ofs_equip ) const
{
	wornitems.print( ofs_pc, ofs_equip );
}

MOVEMODE Character::decode_movemode( const string& str )
{
	MOVEMODE mm = MOVEMODE_NONE;
	if (str.find( 'L' ) != string::npos)
		mm = static_cast<MOVEMODE>(mm + MOVEMODE_LAND);
	if (str.find( 'S' ) != string::npos)
		mm = static_cast<MOVEMODE>(mm + MOVEMODE_SEA);
	if (str.find( 'A' ) != string::npos)
		mm = static_cast<MOVEMODE>(mm + MOVEMODE_AIR);
	return mm;
}

string Character::encode_movemode( MOVEMODE mm )
{
	string res;
	if (mm & MOVEMODE_LAND)
		res += "L";
	if (mm & MOVEMODE_SEA)
		res += "S";
	if (mm & MOVEMODE_AIR)
		res += "A";
	return res;
}

void Character::readCommonProperties( ConfigElem& elem )
{
	serial = elem.remove_ulong( "SERIAL" );

	if (config.check_integrity)
	{
		if (system_find_mobile( serial ))
		{
			cerr << "Character " << hexint(serial) << " defined more than once." << endl;
			throw runtime_error( "Data integrity error" );
		}
	}
	serial_ext = ctBEu32( serial );
	UseCharSerialNumber( serial );

	string acctname;
	if (elem.remove_prop( "ACCOUNT", &acctname ))
	{
		unsigned short charindex;
		charindex = elem.remove_ushort( "CHARIDX" );

		if (charindex >= config.character_slots)
		{
			cerr << "Account " << acctname << ": "
				 << "CHARIDX of " << charindex 
				 << " is too high for character serial ("<< hexint(serial) <<")" << endl;

			throw runtime_error( "Data integrity error" );
		}
		Account *search_acct = find_account( acctname.c_str() );
		if (search_acct == NULL)
		{
			cerr << "Character '" << name() << "': "
				 << "Account '" << acctname << "' doesn't exist."
				 << endl;
			throw runtime_error( "Data integrity error" );
		}
		if (search_acct->get_character( charindex ) != NULL)
		{
			cerr << "Account " << acctname 
				 << " has two characters with CHARIDX of "
				 << charindex
				 << endl;
			throw runtime_error( "Data integrity error" );
		}
		
		acct.set( search_acct );
		acct->set_character( charindex, this );
	}

	trueobjtype = elem.remove_ushort( "TRUEOBJTYPE", objtype_ ); //dave 1/30/3
	graphic = objtype_;
	graphic_ext = ctBEu16( graphic );

	base::readProperties( elem );

	if (name_ == "")
	{
		cerr << "Character '" << hexint(serial) << "' has no name!"
			 << endl;
		throw runtime_error( "Data integrity error" );
	}
	wornitems.serial = serial;
	wornitems.serial_ext = serial_ext;
	position_changed();

	string cmdaccstr = elem.remove_string( "CMDLEVEL", "player" );
	CmdLevel* cmdlevel_search = find_cmdlevel( cmdaccstr.c_str() );
	if (cmdlevel_search == NULL)
			elem.throw_error("Didn't understand cmdlevel of '" + cmdaccstr + "'");
	cmdlevel = static_cast<unsigned char>(cmdlevel_search->cmdlevel);

	movemode = decode_movemode( elem.remove_string( "MOVEMODE", "L" ) );
	concealed_ = static_cast<unsigned char>(elem.remove_ushort( "CONCEALED", 0 )); //DAVE changed from remove_bool 11/25. concealed is a char, not a bool!
//	if (concealed_ > cmdlevel)
//		concealed_ = cmdlevel;

   	truecolor = elem.remove_ushort( "TRUECOLOR" );

	gender = static_cast<UGENDER>(elem.remove_ushort( "GENDER" ));
	race = static_cast<URACE>(elem.remove_ushort( "RACE", RACE_HUMAN));

	dead_ = elem.remove_bool( "DEAD", false );
	hidden_ = elem.remove_bool( "HIDDEN", false );
	frozen_ = elem.remove_bool( "FROZEN", false );
	
	element_resist_mod.fire = static_cast<s16>(elem.remove_int( "FIRERESISTMOD", 0 ));
	element_resist_mod.cold = static_cast<s16>(elem.remove_int( "COLDRESISTMOD", 0 ));
	element_resist_mod.energy = static_cast<s16>(elem.remove_int( "ENERGYRESISTMOD", 0 ));
	element_resist_mod.poison = static_cast<s16>(elem.remove_int( "POISONRESISTMOD", 0 ));
	element_resist_mod.physical = static_cast<s16>(elem.remove_int( "PHYSICALRESISTMOD", 0 ));

	element_damage_mod.fire = static_cast<s16>(elem.remove_int( "FIREDAMAGEMOD", 0 ));
	element_damage_mod.cold = static_cast<s16>(elem.remove_int( "COLDDAMAGEMOD", 0 ));
	element_damage_mod.energy = static_cast<s16>(elem.remove_int( "ENERGYDAMAGEMOD", 0 ));
	element_damage_mod.poison = static_cast<s16>(elem.remove_int( "POISONDAMAGEMOD", 0 ));
	element_damage_mod.physical = static_cast<s16>(elem.remove_int( "PHYSICALDAMAGEMOD", 0 ));

	movement_cost.walk = elem.remove_double( "MovementWalkMod", 1.0 );
	movement_cost.run = elem.remove_double( "MovementRunMod", 1.0 );
	movement_cost.walk_mounted = elem.remove_double( "MovementWalkMountedMod", 1.0 );
	movement_cost.run_mounted = elem.remove_double( "MovementRunMountedMod", 1.0 );

	height = PLAYER_CHARACTER_HEIGHT; //no really, height is 9
   
	created_at = elem.remove_ulong( "CreatedAt", 0 );
	squelched_until = elem.remove_ulong( "SquelchedUntil", 0 );

	title_prefix = elem.remove_string( "TITLEPREFIX", "" );
	title_suffix = elem.remove_string( "TITLESUFFIX", "" );
	title_guild = elem.remove_string( "TITLEGUILD", "" );
	title_race = elem.remove_string( "TITLERACE", "" );

	unsigned long tmp_guildid;
	if (elem.remove_prop( "GUILDID", &tmp_guildid))
		guild_ = FindOrCreateGuild( tmp_guildid, serial );
	//guildid_ = elem.remove_ulong( "GUILDID", 0 );
	murderer_ = elem.remove_bool( "MURDERER", false );
	party_can_loot_ = elem.remove_bool( "PARTYCANLOOT", false );
	string rt;
	while (elem.remove_prop( "REPORTABLE", &rt ))
	{
		ISTRINGSTREAM is(rt);
		reportable_t rt;
		if (is >> rt.serial >> rt.polclock)
		{
			reportable_.insert(rt);
		}
	}

	uclang = elem.remove_string( "UCLang", "enu" );
	expanded_statbar.statcap = static_cast<s16>(elem.remove_int( "STATCAP", 225 ));
	skillcap_ = static_cast<u16>(elem.remove_int( "SKILLCAP", 700 ));
	expanded_statbar.luck = static_cast<s16>(elem.remove_int( "LUCK", 0 ));
	expanded_statbar.followers = static_cast<s8>(elem.remove_int( "FOLLOWERS", 0 ));
	expanded_statbar.followers_max = static_cast<s8>(elem.remove_int( "FOLLOWERSMAX", 0 ));
	expanded_statbar.tithing = elem.remove_int( "TITHING", 0 );

	privs.readfrom( elem.remove_string( "Privs", "" ) );
	settings.readfrom( elem.remove_string( "Settings", "" ) );
	refresh_cached_settings();
}

// these are read here to allow NPCs to have "die-roll" type values
void Character::readAttributesAndVitals( ConfigElem& elem )
{
	// read Attributes
	for( Attribute* pAttr = FindAttribute(0); pAttr != NULL; pAttr = pAttr->next )
	{
		AttributeValue& av = attribute(pAttr->attrid);

		for( unsigned i = 0; i < pAttr->aliases.size(); ++i )
		{
			std::string temp;
			if (elem.remove_prop( pAttr->aliases[i].c_str(), &temp ))
			{
				// read
				unsigned long base;
				unsigned long cap = pAttr->default_cap;
				unsigned char lock = 0;
				if (polvar.DataWrittenBy == 93 &&
					gflag_in_system_load)
				{
					unsigned long raw = strtoul( temp.c_str(), NULL, 10 );
					base = raw_to_base(raw);
				}
				else
				{
					if (!polvar.DataWrittenBy && gflag_in_system_load)
					{
						elem.throw_error( "Pol.txt 'System' element needs to specify CoreVersion" );
					}

					const char* pval = temp.c_str();
					char* pdot = NULL;
					unsigned long ones = strtoul( pval, &pdot, 10 );
					unsigned long tenths = 0;
					if (pdot && *pdot == '.')
						tenths = strtoul( pdot+1, &pdot, 10 );
					base = ones*10+tenths;

					// Do we have caps?
					if (pdot && *pdot == ':') {
						unsigned long cap_ones = strtoul( pdot+1, &pdot, 10 );
						unsigned long cap_tenths = 0;

						// Tenths in cap?
						if (pdot && *pdot == '.')
							cap_tenths = strtoul( pdot+1, &pdot, 10 );
						
						cap = cap_ones*10 + cap_tenths; 
					}

					if (pdot && *pdot == ';')
						lock = (unsigned char)strtoul( pdot+1, NULL, 10 );

				}
				if (base < ATTRIBUTE_MIN_BASE ||
					base > ATTRIBUTE_MAX_BASE)
					elem.throw_error( "Character " + hexint(serial)
										+ " attribute "
										+ pAttr->aliases[i]
										+ " is out of range" );
				av.base( static_cast<unsigned short>(base) );

				if (cap < ATTRIBUTE_MIN_BASE ||
					cap > ATTRIBUTE_MAX_BASE)
					elem.throw_error( "Character " + hexint(serial)
										+ " attribute cap from "
										+ pAttr->aliases[i]
										+ " is out of range" );
				av.cap( static_cast<unsigned short>(cap) );


				if (lock > 2) // FIXME: HardCoded Value
					elem.throw_error( "Character " + hexint(serial) + " attribute " + pAttr->aliases[i] + " has illegal lock state!" ); 
				
				av.lock(lock);

				break;
			}
		}
	}

	calc_vital_stuff();

	// read Vitals
	for( Vital* pVital = FindVital(0); pVital != NULL; pVital = pVital->next )
	{
		VitalValue& vv = vital(pVital->vitalid);
		for( unsigned i = 0; i < pVital->aliases.size(); ++i )
		{
			unsigned long temp;
			if (elem.remove_prop( pVital->aliases[i].c_str(), &temp ))
			{
				// read
				// these are always just stored as points
				if (temp < VITAL_MIN_VALUE ||
					temp > VITAL_MAX_VALUE)
					elem.throw_error( "Character " + hexint(serial)
										+ " vital "
										+ pVital->aliases[i]
										+ " is out of range" );
				vv.current_ones( temp );
				break;
			}
		}
	}
}

void Character::readProperties( ConfigElem& elem )
{
	readCommonProperties( elem );
	readAttributesAndVitals( elem );
	
	last_corpse = elem.remove_ulong("LastCorpse", 0);
}


bool Character::has_privilege( const char* priv ) const
{
	return privs.contains( priv ) || privs.contains( "all" );
}

bool Character::setting_enabled( const char* setting ) const
{
	return settings.contains( setting );
}

void Character::load_default_elements()
{
	// Let's build the resistances defaults.
	element_resist.cold = 0;
	element_resist.energy = 0;
	element_resist.fire = 0;
	element_resist.poison = 0;
	element_resist.physical = 0;
	// Mods
	element_resist_mod.cold = 0;
	element_resist_mod.energy = 0;
	element_resist_mod.fire = 0;
	element_resist_mod.poison = 0;
	element_resist_mod.physical = 0;

	// Let's build the damages defaults.
	element_damage.cold = 0;
	element_damage.energy = 0;
	element_damage.fire = 0;
	element_damage.poison = 0;
	element_damage.physical = 0;
	// Mods
	element_damage_mod.cold = 0;
	element_damage_mod.energy = 0;
	element_damage_mod.fire = 0;
	element_damage_mod.poison = 0;
	element_damage_mod.physical = 0;

	//Movementcost defaults
	movement_cost.walk = 1.0;
	movement_cost.run = 1.0;
	movement_cost.walk_mounted = 1.0;
	movement_cost.run_mounted = 1.0;
}

void Character::refresh_cached_settings()
{
	cached_settings.clotheany	     = setting_enabled( "clotheany" );
	cached_settings.dblclickany	     = setting_enabled( "dblclickany" );
	cached_settings.hearghosts	     = setting_enabled( "hearghosts" );
	cached_settings.invul		     = setting_enabled( "invul" );
	cached_settings.losany		     = setting_enabled( "losany" );
	cached_settings.moveany		     = setting_enabled( "moveany" );
	cached_settings.renameany	     = setting_enabled( "renameany" );
	cached_settings.seeghosts	     = setting_enabled( "seeghosts" );
	cached_settings.seehidden	     = setting_enabled( "seehidden" );
	cached_settings.seeinvisitems    = setting_enabled( "seeinvisitems" );
	cached_settings.ignoredoors	     = setting_enabled( "ignoredoors" );
	cached_settings.freemove		 = setting_enabled( "freemove" );
	cached_settings.firewhilemoving = setting_enabled( "firewhilemoving" );
	cached_settings.attackhidden     = setting_enabled( "attackhidden" );
	cached_settings.hiddenattack     = setting_enabled( "hiddenattack" );
	cached_settings.plogany		     = setting_enabled( "plogany" );
	cached_settings.moveanydist		 = setting_enabled( "moveanydist" );
}

void Character::set_setting( const char* setting, bool value )
{
	set_dirty();
	if (value == false)
		settings.remove( setting );
	else if (has_privilege(setting))
		settings.add( setting );

	refresh_cached_settings();
}

string Character::all_settings() const
{
	return settings.extract();
}

string Character::all_privs() const
{
	return privs.extract();
}

void Character::set_privs( const string& privlist )
{
	set_dirty();
	privs.readfrom( privlist );
}

void Character::grant_privilege( const char* priv )
{
	if (!privs.contains(priv))
	{
		set_dirty();
		privs.add( priv );
	}
}

void Character::revoke_privilege( const char* priv )
{
	set_dirty();
	privs.remove( priv );
	settings.remove( priv );
	refresh_cached_settings();
}

bool Character::can_move( const Item* item ) const
{
	if (item->objtype_ != UOBJ_CORPSE)
	{
		return cached_settings.moveany || item->movable();
	} else {
		return false;
	}
}

bool Character::can_be_renamed_by( const Character* chr ) const
{
		// consider command levels?
	return false;
}

bool Character::can_rename( const Character* chr ) const
{
	return cached_settings.renameany ||
		   chr->can_be_renamed_by( this );
}

bool Character::can_clothe( const Character* chr ) const
{
	return ( (chr == this) ||
			 cached_settings.clotheany );
}

bool Character::can_hearghosts() const
{
	return cached_settings.hearghosts;
}

bool Character::can_moveanydist() const
{
	return cached_settings.moveanydist;
}

UContainer* Character::backpack() const
{
	return static_cast<UContainer*>(wornitems.GetItemOnLayer( LAYER_BACKPACK ) ); 
}

Spellbook* Character::spellbook(int school) const
{
	Item* item = wornitem( LAYER_HAND1 );
	if (item != NULL && item->script_isa(POLCLASS_SPELLBOOK) )
	{
		Spellbook* book = static_cast<Spellbook*>(item);
		if(book->spell_school == school)
			return book;
	}

	UContainer* cont = backpack();
	if (cont != NULL)
	{
		for( UContainer::const_iterator itr = cont->begin(); itr !=  cont->end(); ++itr )
		{
			const Item* item = GET_ITEM_PTR( itr );
			
			if(item != NULL && item->script_isa(POLCLASS_SPELLBOOK))
			{
				const Spellbook* book = static_cast<const Spellbook*>(item);
				if(book->spell_school == school)
					return const_cast<Spellbook*>(book);
			}
		}
	}
	return NULL;
}

unsigned long Character::gold_carried() const
{
	UContainer* bp = backpack();
	if (bp != NULL)
		return bp->find_sumof_objtype_noninuse( UOBJ_GOLD_COIN );
	else
		return 0;
}

// TODO: This could be more efficient, by inlining 'spend' logic
// in a recursive function

void Character::spend_gold( unsigned long amount )
{
	passert( gold_carried() >= amount );

	UContainer* bp = backpack();
	if (bp != NULL)
		bp->consume_sumof_objtype_noninuse( UOBJ_GOLD_COIN, amount );
	if (client != NULL)
		send_full_statmsg( client, this );
}

Item* Character::wornitem( int layer ) const
{
	return wornitems.GetItemOnLayer( layer );
}

bool Character::layer_is_equipped( int layer ) const 
{ 
	return (wornitems.GetItemOnLayer( layer ) != NULL); 
}

bool Character::is_equipped( const Item* item ) const
{
	if (!valid_equip_layer(item))
		return false;
	
	return (wornitems.GetItemOnLayer( item->tile_layer ) == item);
}

bool Character::valid_equip_layer( const Item* item )
{
	return (item->tile_layer >= 1 && item->tile_layer <= HIGHEST_LAYER);
}

bool Character::strong_enough_to_equip( const Item* item ) const
{
	const ItemDesc& desc = find_itemdesc( item->objtype_ );
	return ( attribute(pAttrStrength->attrid).base() >= desc.base_str_req );
}

bool Character::equippable( const Item* item ) const
{
	if (item->objtype_ == EXTOBJ_MOUNT)
	{
		return (!layer_is_equipped( LAYER_MOUNT ));
	}

	const ItemDesc& desc = find_itemdesc( item->objtype_ );

	if (!valid_equip_layer( item ))
	{
		return false;
	}
	if (layer_is_equipped( item->tile_layer ))
	{
		return false;
	}
	if ((item->tile_layer == LAYER_BACKPACK) && !item->isa( CLASS_CONTAINER ))
	{
		return false;
	}
	if (~tile_flags( item->graphic ) & FLAG::EQUIPPABLE)
	{
		return false;
	}
	// redundant sanity check
	if (tilelayer( item->graphic ) != item->tile_layer)
	{
		return false;
	}
	
	if ( attribute(pAttrStrength->attrid).base() < desc.base_str_req )
	{
		return false;
	}
	if (item->tile_layer == LAYER_HAND1 || item->tile_layer == LAYER_HAND2)
	{
		if (weapon != NULL)
		{
			if (weapon->descriptor().two_handed && !weapon->is_intrinsic())
			{
				return false;
			}
		}
		if (item->isa( CLASS_WEAPON ))
		{
			const UWeapon* wpn_item = static_cast<const UWeapon*>(item);
			if (wpn_item->descriptor().two_handed)
			{
				if (layer_is_equipped(LAYER_HAND1) ||
					layer_is_equipped(LAYER_HAND2))
				{
					return false;
				}
			}
		}
	}
	
	return true;
}

void Character::equip( Item *item )
{
	passert_r( equippable( item ),
	  "It is impossible to equip Item with ObjType " + hexint(item->objtype_));

	wornitems.PutItemOnLayer( item );

	// PutItemOnLayer sets the layer, so we can go on now
	// checking item->layer instead of item->tile_layer
	if (item->isa( CLASS_WEAPON ) && 
		(item->layer == LAYER_HAND1 ||
		 item->layer == LAYER_HAND2))
	{
		weapon = static_cast<UWeapon*>(item);
		reset_swing_timer();
	}
	else if (item->isa( CLASS_ARMOR ))
	{
		if (item->layer == LAYER_HAND1 ||
			 item->layer == LAYER_HAND2)
		{
			shield = static_cast<UArmor*>(item);
		}
		reset_swing_timer();
	}
	refresh_ar();
}

UWeapon* Character::intrinsic_weapon()
{
	return wrestling_weapon;
}

void Character::unequip( Item *item )
{
	passert( valid_equip_layer( item ) );
		// assume any item being de-equipped is in fact being worn.
	passert( item->container == &wornitems );		
	passert( is_equipped( item ) );
	
	wornitems.RemoveItemFromLayer( item );

	if (item == weapon)
	{
		weapon = intrinsic_weapon();
		/* we don't reset the swing timer here, 'cause
		   you can switch from weapon to H2H quickly.
		   Note, this _could_ let you use a faster weapon
		   to wrestle faster, hmm.
		*/
	}
	else if (item == shield)
	{
		shield = NULL;
		reset_swing_timer();
	}
	refresh_ar();
}

bool Character::on_mount() const
{
	return layer_is_equipped( LAYER_MOUNT );
}


Item *Character::find_wornitem( u32 serial ) const
{
	for( unsigned layer = LAYER_EQUIP__LOWEST; layer <= LAYER_EQUIP__HIGHEST; layer++ )
	{
		Item *item = wornitems.GetItemOnLayer( layer );
		if (item)
		{
			if (item->serial == serial)
				return item;
			// 4/2007 - MuadDib
			// Added cont check and using cont->find to check here
			// for equipped cont items like the ML added Quivers.
			// Using redundant null check.
			if (item != NULL && item->script_isa(POLCLASS_CONTAINER) )
			{
				if (layer != LAYER_HAIR && layer != LAYER_BACKPACK_WTF && layer != LAYER_BEARD
					&& layer != LAYER_BACKPACK && layer != LAYER_MOUNT)
				{
					UContainer* cont = static_cast<UContainer*>(item);
					item = NULL;
					item = cont->find( serial );
					if (item != NULL) 
						return item;
				}
			}
		}
	}
	return NULL;
}

void Character::produce( const Vital* pVital, VitalValue& vv, unsigned long amt )
{
	long start_ones = vv.current_ones();
	set_dirty();
	vv.produce( amt );
	if (start_ones != vv.current_ones())
	{
		ClientInterface::tell_vital_changed( this, pVital );
	}
}

bool Character::consume( const Vital* pVital, VitalValue& vv, unsigned long amt )
{
	bool res;
	long start_ones = vv.current_ones();
	set_dirty();
	res = vv.consume( amt );
	if (start_ones != vv.current_ones())
	{
		ClientInterface::tell_vital_changed( this, pVital );
	}
	return res;
}

void Character::set_current_ones( const Vital* pVital, VitalValue& vv, unsigned long ones )
{
	set_dirty();
	vv.current_ones( ones );
	ClientInterface::tell_vital_changed( this, pVital );
}

void Character::set_current( const Vital* pVital, VitalValue& vv, unsigned long ones )
{
	set_dirty();
	vv.current( ones );
	ClientInterface::tell_vital_changed( this, pVital );
}

void Character::regen_vital( const Vital* pVital )
{
	VitalValue& vv = vital( pVital->vitalid );
	long rr = vv.regenrate();
	if (rr > 0)
	{
		produce( pVital, vv, rr/12 );
	}
	else if (rr < 0)
	{
		consume( pVital, vv, -rr/12 );
	}
}

void Character::calc_vital_stuff()
{
	for( unsigned ai = 0; ai < numAttributes; ++ai )
	{
		Attribute* pAttr = ::attributes[ai];
		AttributeValue& av = attribute(ai);

		if (pAttr->getintrinsicmod_func)
		{
			long im = pAttr->getintrinsicmod_func->call_long( new ECharacterRefObjImp(this) );
			if (im < ATTRIBUTE_MIN_INTRINSIC_MOD)
				im = ATTRIBUTE_MIN_INTRINSIC_MOD;
			if (im > ATTRIBUTE_MAX_INTRINSIC_MOD)
				im = ATTRIBUTE_MAX_INTRINSIC_MOD;

			av.intrinsic_mod(static_cast<short>(im));
		}
	}
	
	for( unsigned vi = 0; vi < numVitals; ++vi )
	{
		Vital* pVital = ::vitals[vi];
		VitalValue& vv = vital(vi);

		long start_ones = vv.current_ones();

		//dave change the order of maximum and regen function 3/19/3
		long mv = pVital->get_maximum_func->call_long( new ECharacterRefObjImp(this) );
		if (mv < static_cast<long>(VITAL_LOWEST_MAX_HUNDREDTHS))
			mv = VITAL_LOWEST_MAX_HUNDREDTHS;
		if (mv > static_cast<long>(VITAL_HIGHEST_MAX_HUNDREDTHS))
			mv = VITAL_HIGHEST_MAX_HUNDREDTHS;
		vv.maximum(mv);

		long rr = pVital->get_regenrate_func->call_long( new ECharacterRefObjImp(this) );
		if (rr < VITAL_LOWEST_REGENRATE)
			rr = VITAL_LOWEST_REGENRATE;
		if (rr > VITAL_HIGHEST_REGENRATE)
			rr = VITAL_HIGHEST_REGENRATE;
		vv.regenrate(rr);

		if (start_ones != vv.current_ones())
		{
			ClientInterface::tell_vital_changed( this, pVital );
		}
	}

}

void Character::set_vitals_to_maximum() // throw()
{
	set_dirty();
	for( unsigned vi = 0; vi < numVitals; ++vi )
	{
		VitalValue& vv = vital(vi);
		vv.current( vv.maximum() );

		ClientInterface::tell_vital_changed( this, ::vitals[vi] );
	}
}

bool Character::setgraphic( u16 newgraphic )
{
	if (newgraphic < 1 || newgraphic > 0x800) // Maximum graphic: 2048, changed to allow new graphics -- Nando - 2009-01-14
		return false;

	set_dirty();
	graphic = newgraphic;
	graphic_ext = ctBEu16( graphic );
	send_remove_character_to_nearby_cansee( this );
	if (client) send_goxyz( client, client->chr );
	send_create_mobile_to_nearby_cansee( this );
	
	return true;
}

void Character::on_color_changed()
{
	send_remove_character_to_nearby_cansee( this );
	if (client) send_goxyz( client, client->chr );
	send_create_mobile_to_nearby_cansee( this );
}

void Character::on_poison_changed()
{
	send_move_mobile_to_nearby_cansee( this );

	// only if client is active or for npcs
	if ( ( client ) || ( this->isa(UObject::CLASS_NPC) ) )
	{
		PKTOUT_17 msg;
		msg.msgtype = PKTOUT_17_ID;
		msg.msglen = ctBEu16(sizeof msg);
		msg.serial = this->serial_ext;
		msg.unk = ctBEu16(1);
		msg.status_type = ctBEu16(1);
		msg.flag = ( this->poisoned ) ? 1 : 0;

		if ( client )
		{
			send_goxyz( client, client->chr );
			if ( client->isUOKR )
			{
				client->transmit(&msg, sizeof msg);
			}
		}
		// This is a KR only packet, so transmit it only to KR clients
		// who are in range. 
		transmit_to_inrange( this, &msg, sizeof msg, false, true );
	}
}

void Character::on_facing_changed()
{
	if (client) send_goxyz( client, client->chr );
	send_move_mobile_to_nearby_cansee( this );
}

void Character::on_aos_ext_stat_changed()
{
	if (client)
	{
		if ( client->UOExpansionFlag & AOS && has_active_client() )
		{
			send_full_statmsg(client, client->chr);
		}
	}
}

void Character::setfacing( u8 newfacing )
{
	facing = newfacing & 7;
}

u8 Character::get_flag1() const
{
  // Breaks paperdoll 
	u8 flag1 = 0;
	if (gender)
	  flag1 |= CHAR_FLAG1_GENDER;
	if (poisoned)
		flag1 |= CHAR_FLAG1_POISONED;
	if ((ssopt.invul_tag == 2) && (invul()))
	  flag1 |= CHAR_FLAG1_YELLOWHEALTH;
	if (warmode)
		flag1 |= CHAR_FLAG1_WARMODE;
	if (!is_visible())
		flag1 |= CHAR_FLAG1_INVISIBLE;

	return flag1;
}

void Character::apply_raw_damage_hundredths( unsigned long amount, Character* source )
{
	if (dead())
	{
		cerr << "Waah! " << name() << " " << hexint(serial) << " is dead, but taking damage?" << endl;
		return;
	}

	if (source)
		source->repsys_on_attack( this );

	if ((amount == 0) || cached_settings.invul)
		return;
	
	set_dirty();
	if (hidden())
		unhide();
	
	paralyzed_ = false;

	disable_regeneration_for( 2 );// FIXME depend on amount?

	// 0.xx is still 0
	VitalValue& vv = vital(pVitalLife->vitalid);
	if (vv.current()-amount<=99)
		amount=vv.current(); // be greedy with that last point
	consume( pVitalLife, vv, amount );

	if (source)
		source->repsys_on_damage( this );

	send_update_hits_to_inrange( this );

	if (vv.current() == 0)
		die();
}

// keep this in sync with NPC::armor_absorb_damage

						   

unsigned short calc_thru_damage( double damage, unsigned short ar )
{
	int blocked = ar;
	if (blocked < 0) blocked = 0;

	int absorbed = blocked / 2;
	
	blocked -= absorbed;
	absorbed += random_int( blocked+1 );
	damage -= absorbed;

	if (damage >= 2.0)
	{
		return static_cast<unsigned short>(damage * 0.5);
	}
	else
	{
		int dmg = static_cast<int>(damage);
		if (dmg >= 0)
			return static_cast<unsigned short>(dmg);
		else
			return 0;
	}
}


double Character::armor_absorb_damage( double damage )
{
	UArmor* armor = choose_armor();
	if (armor != NULL)
	{
		damage = calc_thru_damage( damage, armor->ar() + ar_mod_ );
		
		armor->reduce_hp_from_hit();

	}
	return damage;
}

double Character::apply_damage( double damage, Character* source )
{
	damage = armor_absorb_damage( damage );
	if (watch.combat) cout << "Final damage: " << damage << endl;
	do_imhit_effects();
	apply_raw_damage_hundredths( static_cast<unsigned long>(damage*100), source );

	return damage;
}

void Character::get_hitscript_params( double damage,
									  UArmor** parmor,
									  unsigned short* rawdamage )
{
	UArmor* armor = choose_armor();
	if (armor)
	{
		*rawdamage = calc_thru_damage( damage, armor->ar() + ar_mod_ );
	}
	else
	{
		*rawdamage = static_cast<unsigned short>(damage);
	}
	*parmor = armor;
}

void Character::run_hit_script( Character* defender, double damage )
{
//	cout << "run_hit_script" << endl;
	ref_ptr<EScriptProgram> prog = find_script2( weapon->hit_script(), true, config.cache_interactive_scripts );
	if (prog.get() == NULL)
		return;

	auto_ptr<UOExecutor> ex( create_script_executor() );
	UOExecutorModule* uoemod = new UOExecutorModule( *ex );
	ex->addModule( uoemod );

	unsigned short rawdamage = 0;
	unsigned short basedamage = static_cast<unsigned short>(damage);

	UArmor* armor = NULL;

	defender->get_hitscript_params( damage, &armor, &rawdamage );


	ex->pushArg( new BLong( rawdamage ) );   
	ex->pushArg( new BLong( basedamage ) );	
	if (armor)
		ex->pushArg( new EItemRefObjImp( armor ) );	   
	else
		ex->pushArg( new BLong(0) );
	ex->pushArg( new EItemRefObjImp( weapon ) );
	ex->pushArg( new ECharacterRefObjImp( defender ) ); 
	ex->pushArg( new ECharacterRefObjImp( this ) );	 

	ex->os_module->priority = 100;

	if (ex->setProgram( prog.get() ))
	{
		uoemod->controller_.set( this );
		schedule_executor( ex.release() );
	}
	else
	{
		Log( "Blech, couldn't start hitscript %s\n", weapon->hit_script().name().c_str() );
	}
}

///
/// Clear a Mobile's ToBeReportable list when all of the following are true:
///	 1) hits are at maximum
///	 2) mobile is not poisoned
///	 3) mobile is not paralyzed
///
void Character::check_undamaged()
{
	if (vital(pVitalLife->vitalid).is_at_maximum() && 
		!poisoned &&
		!paralyzed())
	{
		clear_to_be_reportables();
	}
}


///
/// When a Mobile is Healed
///
///   if Amy's hits are at maximum,
///	   Clear Amy's ToBeReportable list
///
///	 (note, poisoned and paralyzed flags are not checked)
///
void Character::heal_damage_hundredths( unsigned long amount )
{
	if (dead())
	{
		cerr << "Waah! " << name() << " is dead, but healing damage?" << endl;
		return;
	}

	if (amount == 0)
		return;
	
	produce( pVitalLife, vital(pVitalLife->vitalid), amount );
	 
	check_undamaged();

	send_update_hits_to_inrange( this );
}

Item* create_death_shroud()
{
	Item *item = Item::create( UOBJ_DEATH_SHROUD );
	item->layer = LAYER_ROBE_DRESS;
	return item;
}
Item* create_death_robe()
{
	Item *item = Item::create( UOBJ_DEATH_ROBE );
	item->layer = LAYER_ROBE_DRESS;
	return item;
}
Item* create_backpack()
{
	Item *item = Item::create( UOBJ_BACKPACK );
	item->layer = LAYER_BACKPACK;
	return item;
}
#include "../sockio.h"

void Character::send_warmode()
{
	PKTBI_72 msg;
	msg.msgtype = PKTBI_72_ID;
	msg.warmode = warmode ? 1 : 0;
	msg.unk2 = 0;
	msg.unk3_32 = 0x32;
	msg.unk4 = 0;
	transmit( client, &msg, sizeof msg );
}

void send_remove_if_hidden_ghost( Character* chr, Client* client )
{
	if (!inrange( chr, client->chr ))
		return;

	if (!client->chr->is_visible_to_me( chr ))
	{
		send_remove_character( client, chr );
	}
}
void send_create_ghost( Character* chr, Client* client )
{
	if (!inrange( chr, client->chr ))
		return;

	if (chr->dead() && client->chr->is_visible_to_me( chr ))
	{
		send_owncreate( client, chr );
	}
}

void Character::resurrect()
{
	if (!dead())
	{
		cerr << "uh, trying to resurrect " << name() << ", who isn't dead." << endl;
		return;
	}
	set_dirty();
	if (graphic == UOBJ_HUMAN_MALE_GHOST)
		graphic = UOBJ_HUMAN_MALE;
	else if (graphic == UOBJ_HUMAN_FEMALE_GHOST)
		graphic = UOBJ_HUMAN_FEMALE;
	else if (graphic == UOBJ_ELF_MALE_GHOST)
		graphic = UOBJ_ELF_MALE;
	else if (graphic == UOBJ_ELF_FEMALE_GHOST)
		graphic = UOBJ_ELF_FEMALE;
	graphic_ext = ctBEu16( graphic );

	dead_ = false;
	warmode = false;
	frozen_ = false;
	paralyzed_ = false;

	color = truecolor;
	color_ext = ctBEu16(truecolor);

	if (pVitalLife->regen_while_dead)
	{
		VitalValue& vv = vital(pVitalLife->vitalid);
		if (vv._current == 0) // set in die()
			set_current_ones( pVitalLife, vv, 1 );
	}
	else
		set_current_ones( pVitalLife, vital(pVitalLife->vitalid), 1 );

	if (!pVitalMana->regen_while_dead)
		set_current_ones( pVitalMana, vital(pVitalMana->vitalid), 0 );

	if (!pVitalStamina->regen_while_dead)
		set_current_ones( pVitalStamina, vital(pVitalStamina->vitalid), 1 );

	// replace the death shroud with a death robe
	if (layer_is_equipped( LAYER_ROBE_DRESS ))
	{
		Item* death_shroud = wornitems.GetItemOnLayer( LAYER_ROBE_DRESS );
		unequip( death_shroud );
		death_shroud->destroy();
		death_shroud = NULL;
	}
	Item* death_robe = create_death_robe();
	death_robe->realm = realm;
	equip( death_robe );
   
	// equip( create_backpack() );

	//if this has a connected client, tell him his new position. 
	if (client)	
	{
		client->pause();
		send_warmode();
		send_goxyz( client, this );
		send_owncreate( client, this );
		ForEachMobileInVisualRange( client->chr, send_remove_if_hidden_ghost, client );
		client->restart();
	}

	// Tell other connected players, if in range, about this character.
	send_remove_character_to_nearby_cansee( this );
	send_create_mobile_to_nearby_cansee( this );
}

void Character::on_death( Item* corpse )
{
	Item *death_shroud = create_death_shroud();
	death_shroud->realm = realm;
	equip( death_shroud );
	send_wornitem_to_inrange( this, death_shroud );

	if (client != NULL)
	{
		if ( opponent_ )
			opponent_->inform_disengaged(this);

		client->pause();
		send_warmode();
		
		send_goxyz( client, this );
		ForEachMobileInVisualRange( client->chr, send_create_ghost, client );
	}
	
	// change self to ghost for ghosts, remove self for living
	send_remove_character_to_nearby_cantsee( this );

	send_create_mobile_to_nearby_cansee( this );

	if (FileExists("scripts/misc/chrdeath.ecl"))
		::start_script( "misc/chrdeath", new EItemRefObjImp( corpse ), new ECharacterRefObjImp( this ) );
}

void Character::clear_opponent_of()
{
	while (!opponent_of.empty())
	{
		Character* chr = *opponent_of.begin();
			// note that chr->set_opponent is going to remove
			// its entry from our opponent_of collection,
			// so eventually this loop will exit.
		chr->set_opponent( NULL, false);
	}
}

void Character::die()
{
	set_current_ones( pVitalLife, vital(pVitalLife->vitalid), 0);
	clear_my_aggressors(); 
	clear_my_lawful_damagers();
	commit_to_reportables();


	u16 save_graphic = graphic;

	if (graphic == UOBJ_HUMAN_MALE || graphic == UOBJ_HUMAN_FEMALE ||
		graphic == UOBJ_ELF_MALE || graphic == UOBJ_ELF_FEMALE)
	{
		if (gender == GENDER_MALE)
		{
			if (race == RACE_HUMAN)
			{
				graphic = UOBJ_HUMAN_MALE_GHOST;
				graphic_ext = ctBEu16( UOBJ_HUMAN_MALE_GHOST );
			}
			else
			{
				graphic = UOBJ_ELF_MALE_GHOST;
				graphic_ext = ctBEu16( UOBJ_ELF_MALE_GHOST );
			}
		}
		else
		{
			if (race == RACE_HUMAN)
			{
				graphic = UOBJ_HUMAN_FEMALE_GHOST;
				graphic_ext = ctBEu16( UOBJ_HUMAN_FEMALE_GHOST );
			}
			else
			{
				graphic = UOBJ_ELF_FEMALE_GHOST;
				graphic_ext = ctBEu16( UOBJ_ELF_FEMALE_GHOST );
			}
		}
	}
	DECLARE_CHECKPOINT;

	dead_ = true;
	warmode = false;
	frozen_ = false;
	paralyzed_ = false;

	UPDATE_CHECKPOINT();
	/* FIXME: corpse container difficulties.
		
	   What am I gonna do about the corpse.  I've said that 
	   MAX_CONTAINER_ITEMS can be in a container, but if I put
	   worn items, plus what's in the backpack, in the
	   corpse 'container' I'm going to violate this.  If
	   I put everything in a backpack in the corpse,
	   it's too easy to grab everything.
	   Maybe I have to limit the backpack to MAX_CONTAINER_ITEM
	   - NUM_LAYERS or something, or grow the "send container"
	   message to be able to hold a full backpack, plus 
	   all worn items.  That doesn't work, tho, 'cause it
	   overfills the corpse container. ick.

	   The current solution is to put the backpack
	   on the corpse.
	 */ 

	UCorpse* corpse = static_cast<UCorpse*>(Item::create( UOBJ_CORPSE ));
	this->last_corpse = corpse->serial;

	corpse->ownerserial = this->serial;
	corpse->setname( "A corpse of " + name_ );
	corpse->take_contents_to_grave = (acct == NULL);

	UPDATE_CHECKPOINT();

	corpse->color = truecolor;
	corpse->color_ext = ctBEu16(truecolor);
	corpse->x = x;
	corpse->y = y;
	corpse->z = z;
	corpse->realm = realm;
	corpse->facing = facing;
	corpse->corpsetype = save_graphic;
	// corpse->dir = dir;
	UPDATE_CHECKPOINT();
	
	register_with_supporting_multi( corpse );
	if (is_trading())
		cancel_trade( this );
	clear_gotten_item();
	corpse->copyprops( *this );
	UPDATE_CHECKPOINT();

	// Change the character's color to grey
	color = 0;
	color_ext = 0;
	UPDATE_CHECKPOINT();

	for( unsigned layer = LAYER_EQUIP__LOWEST; layer <= LAYER_EQUIP__HIGHEST; ++layer )
	{
		Item *item = wornitems.GetItemOnLayer( layer );
		if (item == NULL)
			continue;

		if (item->layer == LAYER_BEARD || item->layer == LAYER_HAIR)
		{
			//Copies hair items onto the corpse
			Item* copy = item->clone();
			copy->invisible(true);
			copy->movable(false);
			corpse->add( copy );
			continue;
		}

		if (item->newbie() || item->layer == LAYER_BACKPACK)
			continue;

		///
		/// Unequip scripts aren't honored when moving a dead mobile's equipment
		/// onto a corpse
		///
		UPDATE_CHECKPOINT();
		item->check_unequip_script();
		UPDATE_CHECKPOINT();
		unequip( item );
		UPDATE_CHECKPOINT();

		u8 newSlot = 1;
		if ( !corpse->can_add_to_slot(newSlot) || !item->slot_index(newSlot) )
		{
			item->x = corpse->x;
			item->y = corpse->y;
			item->z = corpse->z;
			add_item_to_world( item );
			register_with_supporting_multi( item );
			move_item( item, corpse->x, corpse->y, corpse->z, NULL );
		}
		else
		{
			corpse->add_at_random_location( item );
			// now let's reset the layer property since this is a Death.
			if ( corpse->GetItemOnLayer(item->tile_layer) == NULL )
			{
				corpse->PutItemOnLayer(item);
			}
		}
		UPDATE_CHECKPOINT();
	}
	UPDATE_CHECKPOINT();

	// For some reason, the backpack shows up as a small child.. So change its layer.
	UContainer* bp = backpack();
	if (bp)
	{
		UContainer::Contents tmp;
		UPDATE_CHECKPOINT();
		bp->extract( tmp );
		UPDATE_CHECKPOINT();
		// We set slot to 1 outside the loop. As it cycles through, this will continue
		// to increase. This will reduce the amount of checks to find next available
		// slots
		u8 packSlot = 1;
		//u8 corpseSlot = 1;
		while (!tmp.empty())
		{
			Item* bp_item = ITEM_ELEM_PTR( tmp.back() );
			tmp.pop_back();
			bp_item->container = NULL;
			bp_item->layer = 0;
			UPDATE_CHECKPOINT();
			if (bp_item->newbie() && bp->can_add( *bp_item ))
			{
				if ( !bp->can_add_to_slot(packSlot) || !bp_item->slot_index(packSlot) )
				{
					bp_item->x = corpse->x;
					bp_item->y = corpse->y;
					bp_item->z = corpse->z;
					add_item_to_world( bp_item );
					register_with_supporting_multi( bp_item );
					move_item( bp_item, corpse->x, corpse->y, corpse->z, NULL );
				}
				else
				{
					bp->add( bp_item );
				}
				UPDATE_CHECKPOINT();
			}
			else if (corpse->can_add( *bp_item ))
			{
				if ( !corpse->can_add_to_slot(packSlot) || !bp_item->slot_index(packSlot) )
				{
					bp_item->x = corpse->x;
					bp_item->y = corpse->y;
					bp_item->z = corpse->z;
					add_item_to_world( bp_item );
					register_with_supporting_multi( bp_item );
					move_item( bp_item, corpse->x, corpse->y, corpse->z, NULL );
				}
				else
				{
					corpse->add_at_random_location( bp_item );
				}
				UPDATE_CHECKPOINT();
			}
			else
			{
				UPDATE_CHECKPOINT();
				bp_item->x = corpse->x;
				bp_item->y = corpse->y;
				// move_item calls MoveItemWorldLocation, so this gets it
				// in the right place to start with.
				add_item_to_world( bp_item );
				register_with_supporting_multi( bp_item );
				move_item( bp_item, corpse->x, corpse->y, corpse->z, NULL );
			}
			UPDATE_CHECKPOINT();

		}

		UPDATE_CHECKPOINT();

		for( unsigned layer = LAYER_EQUIP__LOWEST; layer <= LAYER_EQUIP__HIGHEST; ++layer )
		{
			Item *item = wornitems.GetItemOnLayer( layer );
			if (item == NULL)
				continue;

			if (item->layer == LAYER_BEARD || 
				item->layer == LAYER_HAIR || 
				item->layer == LAYER_BACKPACK)
			{
				continue;
			}

			if (item->newbie() && bp->can_add(*item))
			{
				UPDATE_CHECKPOINT();
				item->check_unequip_script();
				UPDATE_CHECKPOINT();
				unequip(item);
				item->layer = 0;
				UPDATE_CHECKPOINT();
				if ( !bp->can_add_to_slot(packSlot) || !item->slot_index(packSlot) )
				{
					item->x = corpse->x;
					item->y = corpse->y;
					item->z = corpse->z;
					add_item_to_world( item );
					register_with_supporting_multi( item );
					move_item( item, corpse->x, corpse->y, corpse->z, NULL );
				}
				else
				{
					bp->add_at_random_location(item);
					update_item_to_inrange( item );
				}
				UPDATE_CHECKPOINT();
			}
		}
		UPDATE_CHECKPOINT();
	}

	
	UPDATE_CHECKPOINT();
	send_death_message( this, corpse );

	UPDATE_CHECKPOINT();
	corpse->restart_decay_timer();
	UPDATE_CHECKPOINT();
	add_item_to_world( corpse );
	UPDATE_CHECKPOINT();
	send_item_to_inrange( corpse );

	UPDATE_CHECKPOINT();
	
	clear_opponent_of();

	set_opponent( NULL );

	UPDATE_CHECKPOINT();

	on_death( corpse );

	CLEAR_CHECKPOINT();
}

void Character::recalc_all_skills( void )
{
}

void Character::refresh_ar()
{
//	find_armor(); <-- MuadDib commented out, put code inside here to cut down on iter.
	// Figure out what's in each zone 
	//   FIXME? NZONES * NLAYERS (5 * 24 = 124) iterations.
	// okay, reverse, for each wornitem, for each coverage area, upgrade.	
	// Turley: should be fixed now only iterators over armor's coverage zones instead of all zones
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

//	calculate_ar();	<-- MuadDib Commented out, mixed code within ported find_armor to reduce iter.
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

	new_ar += ar_mod_;

	short s_new_ar = static_cast<short>(new_ar);
	if (s_new_ar >= 0)
		ar_ = s_new_ar;
	else
		ar_ = 0;

	if (client != NULL)
	{   // CHECKME consider sending less frequently
		send_full_statmsg( client, this );
	}
}

void Character::update_element(unsigned element, Item *item)
{
	switch(element)
	{
		case ELEMENTAL_FIRE: element_resist.fire = element_resist.fire + item->calc_element_resist( ELEMENTAL_FIRE );
							 element_damage.fire = element_damage.fire + item->calc_element_damage( ELEMENTAL_FIRE ); break;
		case ELEMENTAL_COLD: element_resist.cold = element_resist.cold + item->calc_element_resist( ELEMENTAL_COLD );
							 element_damage.cold = element_damage.cold + item->calc_element_damage( ELEMENTAL_COLD ); break;
		case ELEMENTAL_ENERGY: element_resist.energy = element_resist.energy + item->calc_element_resist( ELEMENTAL_ENERGY );
							   element_damage.energy = element_damage.energy + item->calc_element_damage( ELEMENTAL_ENERGY ); break;
		case ELEMENTAL_POISON: element_resist.poison = element_resist.poison + item->calc_element_resist( ELEMENTAL_POISON );
							   element_damage.poison = element_damage.poison + item->calc_element_damage( ELEMENTAL_POISON ); break;
		case ELEMENTAL_PHYSICAL: element_resist.physical = element_resist.physical + item->calc_element_resist( ELEMENTAL_PHYSICAL );
								 element_damage.physical = element_damage.physical + item->calc_element_damage( ELEMENTAL_PHYSICAL ); break;
	}
}

void Character::refresh_element(unsigned element)
{
	switch(element)
	{
		case ELEMENTAL_FIRE: element_resist.fire = element_resist_mod.fire;
							 element_damage.fire = element_damage_mod.fire; break;
		case ELEMENTAL_COLD: element_resist.cold = element_resist_mod.cold;
							 element_damage.cold = element_damage_mod.cold; break;
		case ELEMENTAL_ENERGY: element_resist.energy = element_resist_mod.energy;
							   element_damage.energy = element_damage_mod.energy; break;
		case ELEMENTAL_POISON: element_resist.poison = element_resist_mod.poison;
							   element_damage.poison = element_damage_mod.poison; break;
		case ELEMENTAL_PHYSICAL: element_resist.physical = element_resist_mod.physical;
								 element_damage.physical = element_damage_mod.physical; break;
	}
}

s16 Character::calc_element_resist( unsigned resist ) const
{
	switch(resist)
	{
		case ELEMENTAL_FIRE: return (element_resist.fire + element_resist_mod.fire); break;
		case ELEMENTAL_COLD: return (element_resist.cold + element_resist_mod.cold); break;
		case ELEMENTAL_ENERGY: return (element_resist.energy + element_resist_mod.energy); break;
		case ELEMENTAL_POISON: return (element_resist.poison + element_resist_mod.poison); break;
		case ELEMENTAL_PHYSICAL: return (element_resist.physical + element_resist_mod.physical); break;
	}
	return 0;
}

s16 Character::calc_element_damage( unsigned element ) const
{
	switch(element)
	{
		case ELEMENTAL_FIRE: return (element_damage.fire + element_damage_mod.fire); break;
		case ELEMENTAL_COLD: return (element_damage.cold + element_damage_mod.cold); break;
		case ELEMENTAL_ENERGY: return (element_damage.energy + element_damage_mod.energy); break;
		case ELEMENTAL_POISON: return (element_damage.poison + element_damage_mod.poison); break;
		case ELEMENTAL_PHYSICAL: return (element_damage.physical + element_damage_mod.physical); break;
	}
	return 0;
}

void Character::showarmor() const
{
	if (client != NULL)
	{
		send_sysmessage( client, "Your armor coverage:" );
		for( unsigned i = 0; i < armor_.size(); ++i )
		{
			string text = armorzones[i].name + ": ";
			if (armor_[i] == NULL)
				text += "Nothing";
			else
				text += armor_[i]->name();
			send_sysmessage( client, text );
		}
	}
}

/* check skill: test skill, advance, reset atrophy timers, blah blah..
   obviously, needs work, and more parameters.
*/
#include "../syshook.h"
bool Character::check_skill( USKILLID skillid, int difficulty, unsigned short pointvalue )
{
	INC_PROFILEVAR( skill_checks );
	static bool in_here = false;
	if (!in_here && system_hooks.check_skill_hook != NULL)
	{
		in_here = true;
		bool res = system_hooks.check_skill_hook->call( new ECharacterRefObjImp(this),
													new BLong(skillid),
													new BLong(difficulty),
													new BLong(pointvalue) );
		in_here = false;
		return res;
	}
	else
	{
		return false;
	}
}

// FIXME shouldn't be using PKTIN_02_FACING_MASK here
#include "../mdelta.h"
MoveDelta move_delta[ 8 ] = {
	{  0, -1 },	// 0 is N
	{ +1, -1 }, // 1 is NE
	{ +1,  0 }, // ...
	{ +1, +1 },
	{  0, +1 },
	{ -1, +1 },
	{ -1,  0 },
	{ -1, -1 }
};

void Character::check_concealment_level()
{
	if (concealed_ > cmdlevel)
		concealed_ = cmdlevel;
}

			// you can only be concealed from
			// those of lower stature
bool Character::is_concealed_from_me( const Character* chr ) const
{
	return (chr->concealed_ > cmdlevel);
}

bool Character::is_visible_to_me( const Character* chr ) const
{
	if(chr == this) return true; //I can always see myself (?)
	if (is_concealed_from_me(chr)) return false;

	if (chr->realm != this->realm) return false;  // noone can see across different realms.
	if (!chr->logged_in) return false;
	if (chr->hidden() && !cached_settings.seehidden) return false;	// noone can see anyone hidden.
	if (dead()) return true;			// If I'm dead, I can see anything
	if (!chr->dead() || cached_settings.seeghosts) return true;	  // Anyone can see the living
	if (chr->warmode) return true;	  // Anyone can see someone in warmode
	return false;
};

// NOTE: chr is at new position, lastx/lasty have old position.
void PropagateMove( Client *client, Character *chr )
{
	if (!client->ready)
		return;
	
	if (client->chr == NULL || client->chr == chr)
		return;
	
	int are_inrange;
	int were_inrange;

	if ( !client->chr->is_visible_to_me( chr ))
	{
		return;
	}


	are_inrange = inrange( client->chr, chr );
	were_inrange = inrange( client->chr->x, client->chr->y,	chr->lastx, chr->lasty );
	
	if (are_inrange)
	{
		/* The two characters exist, and are in range of each other.
		   Character 'chr''s lastx and lasty coordinates are valid.
		   SO, if lastx/lasty are out of range of client->chr, we
		   should send a 'create' type message.  If they are in range,
		   we should just send a move.
		*/
		if (chr->move_reason == Character::MULTIMOVE)
		{
			// NOTE: uncomment this line to make movement smoother (no stepping anims)
			// but basically makes it very difficult to talk while the ship
			// is moving.
			#ifdef PERGON
			send_remove_character( client, chr );
			#else
			//send_remove_character( client, chr );
			#endif
			send_owncreate( client, chr );
		}
		else if (were_inrange)
		{
			send_move( client, chr );
		}
		else
		{
			send_owncreate( client, chr );
		}
	}
	else if ( were_inrange ) 
	{
		// if we just walked out of range of this character, send its
		// client a remove object, or else a ghost character will remain.
		send_remove_character( client, chr );
	}
}

void Character::getpos_ifmove( UFACING i_facing, unsigned short* px, unsigned short* py )
{
		*px = x + move_delta[ i_facing ].xmove;
		*py = y + move_delta[ i_facing ].ymove;
}

void Character::swing_task_func( Character* chr )
{
	THREAD_CHECKPOINT( tasks, 800 );
	dtrace(20) << "swing_task_func(" << chr << ")" << endl;
	chr->ready_to_swing = true;
	chr->check_attack_after_move();
	THREAD_CHECKPOINT( tasks, 899 );
}

void Character::schedule_attack()
{
	dtrace(18) << "schedule_attack("<<this<<")" << endl;
			// we'll get here with a swing_task already set, if 
			// while in an adjacent cell to your opponent, you turn/move
			// while waiting for your timeout.
	if (swing_task == NULL)
	{
		unsigned long weapon_speed = weapon->speed();
		unsigned long weapon_delay = weapon->delay();
		polclock_t clocks;

		if (!weapon_delay)
		{
			dtrace(19) << "clocks[speed] = (" <<
			POLCLOCKS_PER_SEC << "*15000)/((" << dexterity() << "+100)*" << weapon_speed << ")" << endl;

			clocks = POLCLOCKS_PER_SEC * static_cast<polclock_t>(15000L);
			clocks /= (dexterity() + 100) * weapon_speed;
		}
		else
		{
			long delay_sum = weapon_delay + delay_mod_;
			if (delay_sum < 0)
				delay_sum = 0;

			dtrace(19) << "clocks[delay] = ((" <<
			weapon_delay << "+" << delay_mod_ << "=" << delay_sum << ")*" << POLCLOCKS_PER_SEC << ")/1000" << endl;

			clocks = (delay_sum * POLCLOCKS_PER_SEC) / 1000;
		}

		if (clocks < (POLCLOCKS_PER_SEC/5))
		{
			dtrace(20) << name() << " attack timer: " << clocks << endl;
		}
		dtrace(19) << "clocks=" << clocks << endl;
		//if (isa(CLASS_CHARACTER))
		//{
		//	cout << name() << " attack timer at: " << swing_timer_start_clock_ + clocks + 1 << endl;
		//}
		new OneShotTaskInst<Character*>( &swing_task, 
										 swing_timer_start_clock_ + clocks + 1, 
										 swing_task_func, 
										 this);
	}
}

void Character::reset_swing_timer()
{
	dtrace(15) << "reset_swing_timer(" << this << ")" << endl;
	ready_to_swing = false;

	swing_timer_start_clock_ = polclock();
	if (swing_task)
		swing_task->cancel();

	if (opponent_ || !opponent_of.empty())
	{
		schedule_attack();
	}
}

bool Character::manual_set_swing_timer(polclock_t clocks)
{
	dtrace(15) << "manual_set_swing_timer(" << this << ") delay: " << clocks << endl;
	ready_to_swing = false;

	swing_timer_start_clock_ = polclock();
	if (swing_task)
		swing_task->cancel();

	if (opponent_ || !opponent_of.empty())
	{
		new OneShotTaskInst<Character*>( &swing_task, 
										 swing_timer_start_clock_ + clocks + 1, 
										 swing_task_func, 
										 this);
		return true;
	}
	else
		return false;
}

/* The highlighted character is:
	 Your selected_opponent, if you have one.
	 If not, then the first char that has you as their opponent.
	 Or, noone.
*/
Character* Character::get_opponent() const
{
	if (opponent_ != NULL)
		return opponent_;
	else if (!opponent_of.empty())
		return *opponent_of.begin();
	else return NULL;
}

bool Character::is_attackable( Character* who ) const
{
	passert( who != NULL );
	if ( combat_config.scripted_attack_checks )
	{
		dtrace(21) << "is_attackable(" << this << "," << who << "): will be handled by combat hook." << endl;
		return true;
	}
	else
	{
		dtrace(21) << "is_attackable(" << this << "," << who << "):" << endl
					<< "  who->dead:	" << who->dead_ << endl
					<< "  wpn->inrange: " << weapon->in_range(this,who) << endl
					<< "  hidden:	   " << hidden() << endl
					<< "  who->hidden:  " << who->hidden() << endl
					<< "  concealed:	" << is_concealed_from_me(who) << endl;
		if ( who->dead() )
			return false;
		else if ( !weapon->in_range(this, who) )
			return false;
		else if ( ( hidden() )  && ( !cached_settings.hiddenattack ) )
			return false;
		else if ( ( who->hidden() )  && ( !cached_settings.attackhidden ) )
			return false;
		else if ( is_concealed_from_me(who) )
			return false;
		else if ( !realm->has_los(*this, *who) )
			return false;
		else
			return true;
	}
}

Character* Character::get_attackable_opponent() const
{
	if (opponent_ != NULL)
	{
		dtrace(20) << "get_attackable_opponent(" << this << "): checking opponent, " << opponent_ << endl;
		if (is_attackable( opponent_ ) )
			return opponent_;
	}
	
	if (!opponent_of.empty())
	{
		for( CharacterSet::const_iterator itr = opponent_of.begin(), end = opponent_of.end(); itr != end; ++itr )
		{
			Character* who = (*itr);
			dtrace(20) << "get_attackable_opponent(" << this 
					   << "): checking opponent_of, " << who << endl;
			if (is_attackable( who ))
				return who;
		}
	}

	return NULL;
}

void Character::send_highlight() const
{
	if ( client != NULL && has_active_client() )
	{
		Character* opponent = get_opponent();

		PKTOUT_AA msg;
		msg.msgtype = PKTOUT_AA_ID;
		
		if (opponent != NULL)
			msg.serial = opponent->serial_ext;
		else
			msg.serial = 0;

		transmit( client, &msg, sizeof msg );
	}
}

void Character::on_swing_failure( Character* attacker )
{
	// do nothing
}

void Character::inform_disengaged( Character* disengaged )
{
		// someone has just disengaged. If we don't have an explicit opponent,
		// pick one of those that has us targetted as the highlight character.
	if (opponent_ == NULL) 
		send_highlight();
}

void Character::inform_engaged( Character* engaged )
{
		// someone has targetted us.  If we don't have an explicit opponent,
		// pick one of those that has us targetted as the highlight character.
	if (opponent_ == NULL) 
		send_highlight();
}

void Character::inform_criminal( Character* moved )
{
	// virtual that does nothing at character level, but fires event for NPCs
}

void Character::inform_leftarea( Character* wholeft )
{
	// virtual that does nothing at character level, but fires event for NPCs
}

void Character::inform_moved( Character* moved )
{
	// consider moving PropagateMove here!
}
void Character::inform_imoved( Character* chr )
{
}

void Character::set_opponent( Character* new_opponent, bool inform_old_opponent )
{
	dtrace(12) << "set_opponent(" << this << "," << new_opponent << ")" << endl;
	if (new_opponent != NULL)
	{
		if (new_opponent->dead())
			return;

		if (!warmode && (script_isa(POLCLASS_NPC) || has_active_client()))
			set_warmode( true );
	}

	if (opponent_ != NULL)
	{
		opponent_->opponent_of.erase( this );
		// Turley 05/26/09 no need to send disengaged event on shutdown
		if ( !exit_signalled )
		{
			if ( inform_old_opponent && opponent_ != NULL )
				opponent_->inform_disengaged( this );
		}
	}

	opponent_ = new_opponent;

	
	// Turley 05/26/09 possible shutdown crashfix during cleanup
	// (inside schedule_attack() the rest is also senseless on shutdowncleanup)
	if ( !exit_signalled )
	{
		reset_swing_timer();

		if (opponent_ != NULL)
		{
			repsys_on_attack( opponent_ );
			if (opponent_->get_opponent() == NULL)
				opponent_->reset_swing_timer();

			opponent_->opponent_of.insert( this );

			opponent_->inform_engaged( this );

			opponent_->schedule_attack();
		}

		send_highlight();
	}
}

void Character::select_opponent( u32 serial )
{
		// test for setting to same so swing timer doesn't reset
		// if you double-click the same guy over and over
	if (opponent_ == NULL ||
		opponent_->serial != serial)
	{
		Character* new_opponent = find_character( serial );
		if (new_opponent != NULL)
		{
			if(realm != new_opponent->realm)
				return;
			set_opponent( new_opponent );
		}
	}
}

void Character::disable_regeneration_for( int seconds )
{
	time_t new_disable_time = poltime() + seconds;
	if (new_disable_time > disable_regeneration_until)
		disable_regeneration_until = new_disable_time;
}

void Character::set_warmode( bool i_warmode )
{
	if (warmode != i_warmode)
	{
		disable_regeneration_for( 2 );

	}

	warmode = i_warmode;
	if (warmode == false)
	{
		set_opponent( NULL );
	}
	reset_swing_timer();

	if (dead())
	{
		if (warmode)	// if entered warmode, display me now
		{
			send_create_mobile_to_nearby_cansee( this );
		}
		else			// if leaving warmode, I go invisible.
		{				   
			send_remove_character_to_nearby_cantsee( this );
		}
	}
	else
	{
		ConstForEach( clients, send_move_if_inrange, this );
	}
}

const AttributeValue& Character::weapon_attribute() const
{
	return attribute(weapon->attribute().attrid);
}

unsigned short Character::random_weapon_damage() const
{
	return weapon->get_random_damage();
}

unsigned short Character::min_weapon_damage() const
{
	return weapon->descriptor().damage_dice.min_value();
}

unsigned short Character::max_weapon_damage() const
{
	return weapon->descriptor().damage_dice.max_value();
}

void Character::damage_weapon()
{
	if (!weapon->is_intrinsic() && !weapon->is_projectile())
	{
		weapon->reduce_hp_from_hit();
	}
}

UArmor* Character::choose_armor() const
{
	double f = static_cast<double>(random_float( static_cast<float>(armor_zone_chance_sum) ));
	for( unsigned zone = 0; zone < armorzones.size(); ++zone )
	{
		f -= armorzones[ zone ].chance;
		if (f <= 0.0)
		{
			return armor_[ zone ];
		}
	}
	return NULL;
}

UACTION Character::weapon_anim() const
{
	if (on_mount())
		return weapon->mounted_anim();
	else
		return weapon->anim();
}

void Character::do_attack_effects( Character* target )
{
	if (weapon->is_projectile())
	{
		// 234 is hit, 238 is miss??
		play_sound_effect( this, weapon->projectile_sound() );
		play_moving_effect( this, target,
							weapon->projectile_anim(),
							9,		// Speed (??)
							0,		// Loop
							0 );  // Explode
		if (graphic >= UOBJ_HUMAN_MALE)
		{
			send_action_to_inrange( this, weapon_anim() );
		}
		else
		{
			send_action_to_inrange( this, ACTION_LOOK_AROUND );
		}
	}
	else if (graphic >= UOBJ_HUMAN_MALE)
	{
		send_action_to_inrange( this, weapon_anim() );
	}
	else
	{
		send_action_to_inrange( this, ACTION_LOOK_AROUND );
	}
}

void Character::do_hit_success_effects()
{
	unsigned short sound = weapon->hit_sound();
	if (sound)
		play_sound_effect( this, sound );
}

void Character::do_hit_failure_effects()
{
	unsigned short sound = weapon->miss_sound();
	if (sound)
		play_sound_effect( this, sound );
}

void Character::do_imhit_effects()
{
	if ( combat_config.core_hit_sounds )
	{
		int sfx = 0;
		if ( this->isa(UObject::CLASS_NPC) )
		{
			NPC* npc = static_cast<NPC*>(this);
			sfx = npc->damaged_sound;
		}
		if ( !sfx )
		{
			if ( gender == GENDER_MALE )
				sfx = SOUND_EFFECT_MALE_DEFENSE;
			else
				sfx = SOUND_EFFECT_FEMALE_DEFENSE;
		}
		play_sound_effect(this, static_cast<u16>(sfx));
	}	
	if (objtype_ >= UOBJ_HUMAN_MALE)
		send_action_to_inrange( this, ACTION_GOT_HIT );
}


void Character::attack( Character* opponent )
{
	INC_PROFILEVAR(combat_operations);

	if ( system_hooks.attack_hook )
	{
		bool rc = system_hooks.attack_hook->call( new ECharacterRefObjImp(this), new ECharacterRefObjImp(opponent) );
		if ( rc )
			return;
	}

	if ( watch.combat )
		cout << name() << " attacks " << opponent->name() << endl;

	if ( weapon->is_projectile() )
	{
		UContainer* bp = backpack();
		if ((bp == NULL) ||
					(weapon->consume_projectile( bp ) == false))
		{
				// 04/2007 - MuadDib
				// Range through wornitems to find containers and check
				// here also if backpack fails. Use the mainpack first this way.
				bool projectile_check = false;
				for( unsigned layer = LAYER_EQUIP__LOWEST; layer <= LAYER_EQUIP__HIGHEST; layer++ )
				{
					Item *item = wornitems.GetItemOnLayer( layer );
					if (item)
					{
						if (item != NULL && item->script_isa(POLCLASS_CONTAINER) )
						{
							if (layer != LAYER_HAIR && layer != LAYER_BACKPACK_WTF && layer != LAYER_BEARD
								&& layer != LAYER_BACKPACK && layer != LAYER_MOUNT)
							{
								UContainer* cont = static_cast<UContainer*>(item);
								if (weapon->consume_projectile( cont ) == true)
								{
									projectile_check = true;
									break;
								}
							}
						}
					}
				}
				// I'm out of projectiles.  
				if (projectile_check == false)
					return;
		}
	}

	repsys_on_attack( opponent );
	repsys_on_damage( opponent );

	do_attack_effects( opponent );

	if (system_hooks.combat_advancement_hook)
	{
		system_hooks.combat_advancement_hook->call( new ECharacterRefObjImp(this),
												new EItemRefObjImp( weapon ),
												new ECharacterRefObjImp( opponent ) );
	}

	double hit_chance = ( weapon_attribute().effective() + 50.0) / ( 2.0 * (opponent->weapon_attribute().effective() + 50.0));
	hit_chance += hitchance_mod_ * 0.001f;
	hit_chance -= opponent->evasionchance_mod_ * 0.001f;
	if (watch.combat)
	cout << "Chance to hit: " << hit_chance << ": ";
	if (random_float( 1.0 ) < hit_chance)
	{
		if (watch.combat)
			cout << "Hit!" << endl;
		do_hit_success_effects();
				
		double damage = random_weapon_damage();
		damage_weapon();

		if (watch.combat) cout << "Base damage: " << damage << endl;

		double damage_multiplier = attribute(pAttrTactics->attrid).effective() + 50;
		damage_multiplier += strength() * 0.20f;
		damage_multiplier *= 0.01f;

		damage *= damage_multiplier;

		if (watch.combat) 
			cout << "Damage multiplier due to tactics/STR: " << damage_multiplier << " Result: " << damage << endl;
				
		if (opponent->shield != NULL)
		{
			if (system_hooks.parry_advancement_hook)
			{
				system_hooks.parry_advancement_hook->call( new ECharacterRefObjImp(this),
															new EItemRefObjImp( weapon ),
															new ECharacterRefObjImp( opponent ),
															new EItemRefObjImp( opponent->shield ) );
			}

			double parry_chance = opponent->attribute(pAttrParry->attrid).effective() / 200.0;
			if (watch.combat) cout << "Parry Chance: " << parry_chance << ": ";
			if (random_float( 1.0 ) < parry_chance)
			{
				if ( watch.combat ) 
					cout << opponent->shield->ar() << " hits deflected" << endl;
				if ( combat_config.display_parry_success_messages && opponent->client )
					send_sysmessage( opponent->client, "You successfully parried the attack!" );
							
				damage -= opponent->shield->ar(); 
				if (damage < 0)
					damage = 0;
			}
			else
			{
				if (watch.combat) cout << "failed." << endl;
			}
		}
		if (weapon->hit_script().empty())
		{
			opponent->apply_damage( damage, this );
		}
		else
		{
			run_hit_script( opponent, damage );
		}
	}
	else
	{
		if (watch.combat) 
			cout << "Miss!" << endl;
		opponent->on_swing_failure( this );
		do_hit_failure_effects();
		if (system_hooks.hitmiss_hook)
		{
			system_hooks.hitmiss_hook->call( new ECharacterRefObjImp( this ),
											 new ECharacterRefObjImp( opponent ) );
		}

	}
}

void Character::check_attack_after_move()
{
	FUNCTION_CHECKPOINT( check_attack_after_move, 1 );
	Character* opponent = get_attackable_opponent();
	FUNCTION_CHECKPOINT( check_attack_after_move, 2 );
	dtrace(20) << "check_attack_after_move(" << this << "): opponent is " << opponent << endl;	
	if (opponent != NULL &&				   // and I have an opponent
		!dead_ /*&&							 // If I'm not dead
		!paralyzed() &&
		!frozen()*/)							  
	{
		FUNCTION_CHECKPOINT( check_attack_after_move, 3 );
		if (ready_to_swing)				 // and I can swing now,
		{								   // do so.
			FUNCTION_CHECKPOINT( check_attack_after_move, 4 );
			if (combat_config.send_swing_packet && client != NULL)
				send_fight_occuring( client, opponent );
			attack( opponent );
			FUNCTION_CHECKPOINT( check_attack_after_move, 5 );
			reset_swing_timer();
			FUNCTION_CHECKPOINT( check_attack_after_move, 6 );
		}
		else
		{
			FUNCTION_CHECKPOINT( check_attack_after_move, 7 );
			dtrace(20) << "not ready to swing" << endl;
			schedule_attack();
			FUNCTION_CHECKPOINT( check_attack_after_move, 8 );
		}
	}
	FUNCTION_CHECKPOINT( check_attack_after_move, 0 );
}
#include "../lightlvl.h"
#include "../guardrgn.h"
#include "../miscrgn.h"
#include "../musicrgn.h"
#include "../network/cgdata.h"

void Character::check_light_region_change()
{
	if (client->gd->weather_region && 
		client->gd->weather_region->lightoverride != -1 &&
		lightoverride == -1)
		return;

	int newlightlevel;
	if (lightoverride != -1)
		newlightlevel = lightoverride;
	else
	{
		//dave 12-22 check for no regions
		LightRegion* light_region = lightdef->getregion( x, y, realm );
		if( light_region != NULL)
			newlightlevel = light_region->lightlevel;
		else
			newlightlevel =  ssopt.default_light_level;
	}

	if (newlightlevel != client->gd->lightlevel)
	{
		send_light( client, newlightlevel );
		client->gd->lightlevel = newlightlevel;
	}
}

void Character::check_justice_region_change()
{
	JusticeRegion* cur_justice_region = client->gd->justice_region;
	JusticeRegion* new_justice_region = justicedef->getregion( x, y, client->chr->realm );

	if ( cur_justice_region != new_justice_region )
	{
		if ( cur_justice_region != NULL )
			cur_justice_region->RunLeaveScript(client->chr);
		if ( new_justice_region != NULL )
			new_justice_region->RunEnterScript(client->chr);

		// print 'leaving' message
		bool printmsgs;
		if (cur_justice_region != NULL &&
			new_justice_region != NULL &&
			cur_justice_region->entertext() == new_justice_region->entertext() &&
			cur_justice_region->leavetext() == new_justice_region->leavetext())
		{
			printmsgs = false;
		}
		else
		{
			printmsgs = true;
		}

		if ( printmsgs && cur_justice_region )
		{
			const std::string& leavetext = cur_justice_region->leavetext();
			if (!leavetext.empty())
			{
				send_sysmessage( client, leavetext );
			}

		}

		client->gd->justice_region = new_justice_region;

		if(new_justice_region->RunNoCombatCheck(client) == true )
		{	
			Character* opp2 = get_opponent();
			if( (opp2 != NULL &&
				opp2->client) )
			{
				opp2->opponent_of.erase( client->chr );
				opp2->set_opponent( NULL, true );
				opp2->schedule_attack();
				opp2->opponent_ = NULL;
				opp2->clear_opponent_of();
				set_opponent( NULL, true );
			if (swing_task != NULL)
				swing_task->cancel();
			}
		}


		// print 'entering' message
		// handle nocombat while we have entered.
		if ( printmsgs && new_justice_region )
		{
			const std::string& entertext = new_justice_region->entertext();
			if (!entertext.empty())
			{
				send_sysmessage( client, entertext );
			}
		}
	}
}

void Character::check_music_region_change()
{
	MusicRegion* cur_music_region = client->gd->music_region;
	MusicRegion* new_music_region = musicdef->getregion( x, y, realm );

	// may want to consider changing every n minutes, too, even if region didn't change
	if (cur_music_region != new_music_region)
	{
		client->gd->music_region = new_music_region;
		if (new_music_region != NULL)
		{
			send_midi( client, new_music_region->getmidi() );
		}
		else
		{
			send_midi( client, 0 );
		}
	}
}

bool move_character_to( Character* chr, unsigned short x, unsigned short y,int z,long flags ); //for some reason send_goxyz doesn't work well for this (see below)
void Character::check_weather_region_change(bool force) //dave changed 5/26/03 - use force boolean if current weather region changed type/intensity
{
	WeatherRegion* cur_weather_region = client->gd->weather_region;
	WeatherRegion* new_weather_region = weatherdef->getregion( x, y, realm );

	// eric 5/31/03: I don't think this is right.  it's possible to go from somewhere that has no weather region,
	// and to walk to somewhere that doesn't have a weather region.
	// 
	if ( force || (cur_weather_region != new_weather_region) )
	{
		if (new_weather_region != NULL &&
			new_weather_region->lightoverride != -1 &&
			lightoverride == -1)
		{
			send_light( client, new_weather_region->lightoverride );
			client->gd->lightlevel = new_weather_region->lightoverride;
		}
		
		// eric removed this 5/31/03, it's calling itself recursively:
		// move_character_to -> tellmove -> check_region_changes -> check_weather_region_change (here, doh)
		// if you need to send the client something special, just do it.
		// move_character_to(this,x,y,z,0); //dave added 5/26/03: client doesn't refresh properly without a teleport :| and send_goxyz causes weather effects to stop if character is walking/running too
		
		if (new_weather_region)
		{
			send_weather( client, 
					  new_weather_region->weathertype, 
					  new_weather_region->severity,
					  new_weather_region->aux );
		}
		else
		{
			send_weather( client, 0xff, 0, 0 ); // turn off 
		}
		client->gd->weather_region = new_weather_region;
	}
}

void Character::check_region_changes()
{
	if (client != NULL)
	{
		check_weather_region_change();

		check_light_region_change();
		
		check_justice_region_change();
		
		check_music_region_change();
	}
}

void Character::position_changed()
{ 
	wornitems.x = x; 
	wornitems.y = y; 
	wornitems.z = z; 
	wornitems.realm = realm;
	
}

void Character::unhide()
{
	hidden_ = false;
	if (is_visible())
	{
		if (client != NULL)
			send_owncreate( client, this );
		ForEach( clients, send_char_data, this );

		//dave 12-21 added this hack to get enteredarea events fired when unhiding
		u16 oldlastx = lastx;
		u16 oldlasty = lasty;
		lastx = 0;
		lasty = 0;
		//tellmove();
		ForEachMobileInRange( x, y, realm, 32, NpcPropagateMove, this );
		lastx = oldlastx;
		lasty = oldlasty;
	}
}

void Character::set_stealthsteps( unsigned short newval )
{
	stealthsteps_ = newval;
}

bool Character::doors_block() const
{
	return ! (graphic == UOBJ_GAMEMASTER || 
			  graphic == UOBJ_HUMAN_MALE_GHOST || 
			  graphic == UOBJ_HUMAN_FEMALE_GHOST ||
			  graphic == UOBJ_ELF_MALE_GHOST || 
			  graphic == UOBJ_ELF_FEMALE_GHOST ||
			  cached_settings.ignoredoors);
}

/* 
	This isn't quite right, for hidden characters.
	What normally happens:
	1) Client sends "use skill hiding"
	2) Server sends "77 move" with the hidden flag set.
	3) client sends move-request
	4) server sends move-approve
	5) server sense "78 create" message with hidden flag clear, at new posn.
 
	We're sending the "78 create" _before_ the move-approve.
 */

bool Character::can_face( UFACING i_facing )
{
	if(cached_settings.freemove)
		return true;

	if (frozen() || paralyzed())
	{
		if ((client != NULL) && frozen())
			private_say_above( this, this, "I am frozen and cannot move." );

		return false;
	}

	if (ssopt.movement_uses_stamina && 
		vital(pVitalStamina->vitalid).current_ones() == 0 &&
		!dead())
	{
		private_say_above( this, this, "You are too fatigued to move." );
		return false;
	}

	return true;
}


bool Character::face( UFACING i_facing, long flags )
{
	if((flags & 1) == 0)
	{
		if(!can_face(i_facing))
			return false;
	}

	if(i_facing != facing)
	{
		setfacing(static_cast<u8>(i_facing));

		if (combat_config.reset_swing_onturn && !cached_settings.firewhilemoving && weapon->is_projectile())
			reset_swing_timer();
		if (hidden() && (ssopt.hidden_turns_count))
		{
			if (stealthsteps_==0)
				unhide();
			else
				stealthsteps_--;
		}
	}

	return true;
}


//************************************
// Method:    move
// FullName:  Character::move
// Access:    public 
// Returns:   bool
// Qualifier:
// Parameter: unsigned char i_dir
//************************************
bool Character::move( unsigned char i_dir )
{
	lastx = x;
	lasty = y;
	lastz = z;

	u8 oldFacing = facing;

	UFACING i_facing = static_cast<UFACING>(i_dir & PKTIN_02_FACING_MASK);
	if(!face(i_facing))
		return false;

	// If we're a player, and we used our "move" command to turn,
	//  we want to skip the meat of this function
	if((script_isa(POLCLASS_NPC)) || (facing == oldFacing))
	{
		if (facing & 1) // check if diagonal movement is allowed -- Nando (2009-02-26)
		{
			short new_z;
			u8 tmp_facing = (facing+1) & 0x7;
			unsigned short tmp_newx = x + move_delta[ tmp_facing ].xmove;
			unsigned short tmp_newy = y + move_delta[ tmp_facing ].ymove;

			// needs to save because if only one direction is blocked, it shouldn't block ;)
			bool walk1 = realm->walkheight(this, tmp_newx, tmp_newy, z, &new_z, NULL, NULL, NULL );

			tmp_facing = (facing-1) & 0x7;
			tmp_newx = x + move_delta[ tmp_facing ].xmove;
			tmp_newy = y + move_delta[ tmp_facing ].ymove;

			if (!walk1 && !realm->walkheight(this, tmp_newx, tmp_newy, z, &new_z, NULL, NULL, NULL ))
				return false;
		}

		unsigned short newx = x + move_delta[ facing ].xmove;
		unsigned short newy = y + move_delta[ facing ].ymove;
		
		// FIXME consider consolidating with similar code in UOEMOD.CPP
		short newz;
		UMulti* supporting_multi;
		Item* walkon_item;
		//cout << "checking walkheight" << endl;
		
		int current_boost = gradual_boost;
		if (!realm->walkheight(this, newx, newy, z, &newz, &supporting_multi, &walkon_item, &current_boost ))
			return false;
		//cout << "walkheight is okay" << endl;
		//if (walkon_item)
		//	cout << "chr::move walkon item" << endl;

		remote_containers_.clear();

		if(!CheckPushthrough())
			return false;

		if (!cached_settings.freemove && ssopt.movement_uses_stamina && !dead())
		{
			int carry_perc = weight() * 100 / carrying_capacity();
			unsigned short tmv = movecost( this, carry_perc, (i_dir&PKTIN_02_DIR_RUNNING_BIT)?true:false, on_mount() );
			VitalValue& stamina = vital(pVitalStamina->vitalid);
			//u16 old_stamina = stamina.current_ones();
			if (!consume( pVitalStamina, stamina, tmv ))
			{
				private_say_above( this, this, "You are too fatigued to move." );
				return false;
			}
		}

		// Maybe have a flag for this in servspecopt?
		if (!cached_settings.firewhilemoving && weapon->is_projectile())
			reset_swing_timer();

		x = static_cast<u16>(newx);
		y = static_cast<u16>(newy);
		z = static_cast<s8>(newz);

		if (supporting_multi != NULL)
		{
			supporting_multi->register_object( this );
		}

		gradual_boost = current_boost;
		MoveCharacterWorldPosition( lastx, lasty, x, y, this, NULL );
		
		position_changed();
		if (walkon_item != NULL)
		{
			//cout << "walkon_item found" << endl;
			walkon_item->walk_on( this );
		}

		if (hidden())
		{
			if ((i_dir & 0x80) || (stealthsteps_ == 0))
				unhide();
			else if (stealthsteps_)
				--stealthsteps_;
		}
	}

	set_dirty();
	dir = i_dir;

	return true;


	// this would be a great place for tellmove(), except that
	// we want to send the move acknowledge to the client before
	// sending the move/create messages to the neighboring clients.
	
	// Why? Maybe to give the best response time to the client.
	
	// may want to rethink this, since it's starting to complicate
	// things.

}

void Character::realm_changed()
{
	// Commented out the explicit backpack handling, should be handled 
	// automagically by wormitems realm handling.  There is a slim 
	// possibility that backpacks might be assigned to a character but
	// not be a worn item?  If this is the case, that will be broken.
	//	backpack()->realm = realm;
	//	backpack()->for_each_item(setrealm, (void*)realm);
	wornitems.for_each_item(setrealm, (void*)realm);
	if(gotten_item)
		gotten_item->realm = realm;
	if(trading_cont.get())
		trading_cont->realm = realm;

	if(has_active_client())
	{
		//these are important to keep here in this order
		send_realm_change( client, realm );
		send_map_difs( client );
		send_season_info( client );
		send_short_statmsg( client, this );
		send_feature_enable( client );
	}
}

bool Character::CheckPushthrough()
{	
	if(!cached_settings.freemove && system_hooks.pushthrough_hook != NULL)
	{
		unsigned short newx = x + move_delta[ facing ].xmove;
		unsigned short newy = y + move_delta[ facing ].ymove;

		unsigned short wx, wy;
		zone_convert_clip( newx, newy, realm, wx, wy );

		ZoneCharacters& wchr = realm->zone[wx][wy].characters;
		ZoneCharacters::iterator itr = wchr.begin();
		ZoneCharacters::iterator end = wchr.end();

		ObjArray* mobs = NULL;
		for( ; itr != end; ++itr )
		{
			Character* chr = *itr;
			if (chr->x == newx &&
				chr->y == newy &&
				chr->z >= z-10 && chr->z <= z+10 &&
				!chr->dead() &&
				(is_visible_to_me(chr) || chr->hidden()) ) //add hidden mobs even if they're not visible to me
			{
				if(mobs == NULL)
				{
					mobs = new ObjArray();
				}
				mobs->addElement(make_mobileref(chr));
			}
		}
		if(mobs != NULL)
		{
			bool result = system_hooks.pushthrough_hook->call( make_mobileref(this), mobs );
			return result;
		}
		return true;

	}
	return true;
}

static void _check_attack( Character* chr )
{
	chr->check_attack_after_move();
}

void Character::tellmove()
{
	check_region_changes();
	ForEach( clients, PropagateMove, this );
	// Austin 8-25-05
	// if distance > 32 - Inform NPCs in the old position about the movement.
	// This is specifically for long distance teleportations.
	// TO DO: Place in realm change support so npcs know when you enter/leave one?
	if ( pol_distance(lastx, lasty, x, y) > 32 )
	{	
		ForEachMobileInRange( lastx, lasty, realm, 32, NpcPropagateMove, this );
	}

	// Inform nearby NPCs that a movement has been made.
	ForEachMobileInRange( x, y, realm, 32, NpcPropagateMove, this );
	
	//ForEach( characters, NpcPropagateMove, this );
	
	check_attack_after_move();
	
	if (opponent_ != NULL)
		opponent_->check_attack_after_move();
	
	// attacking can change the opponent_of array drastically.
	set<Character*> tmp(opponent_of);
	ForEach( tmp, _check_attack );

	move_reason = OTHER;
}

void Character::add_remote_container( Item* item )
{
	remote_containers_.push_back( ItemRef(item) );
}

Item* Character::search_remote_containers( u32 serial, bool* isRemoteContainer ) const
{
	if (isRemoteContainer)
		*isRemoteContainer = false;
	for( std::vector<ItemRef>::const_iterator itr = remote_containers_.begin();
		 itr != remote_containers_.end();
		 itr++ )
	{
		Item* item = (*itr).get();
		if (item->orphan())
			continue; // it'll be cleaned up when they move
		if (item->serial == serial)
		{
			if (isRemoteContainer)
				*isRemoteContainer = true;
			return item;
		}
		if (item->isa(UObject::CLASS_CONTAINER))
		{
			item = ((UContainer *) item)->find( serial );
			if (item) 
			{
				if (isRemoteContainer)
					*isRemoteContainer = false;
				return item;
			}
		}
	}
	return NULL;
}

bool Character::mightsee( const Item *item ) const
{
	while (item->container != NULL)
		item = item->container;
	
	for( std::vector<ItemRef>::const_iterator itr = remote_containers_.begin();
		 itr != remote_containers_.end();
		 itr++ )
	{
		Item* additional_item = (*itr).get();
		if (additional_item == item)
			return true;
	}


	return ( (item->realm == realm) &&
			 (abs( x - item->x ) <= RANGE_VISUAL) &&
			 (abs( y - item->y ) <= RANGE_VISUAL) );
}

bool Character::squelched() const
{
	if (squelched_until == 0)
		return false;
	else if (squelched_until == ~0Lu)
		return true;
	
	if (read_gameclock() < squelched_until)
	{
		return true;
	}
	else
	{
		squelched_until = 0;
		return false;
	}
}

bool Character::invul() const
{
	return cached_settings.invul;
}

u16 Character::strength() const
{
	return static_cast<u16>(attribute(pAttrStrength->attrid).effective());
}
u16 Character::dexterity() const
{
	return static_cast<u16>(attribute(pAttrDexterity->attrid).effective());
}
u16 Character::intelligence() const
{
	return static_cast<u16>(attribute(pAttrIntelligence->attrid).effective());
}

// a bad thing about having this function be a member of Character
// is that Character has to access global state variables (namely,
// the items collection).
Item* Character::get_from_ground( u32 serial, UContainer** found_in )
{
	unsigned short wxL, wyL, wxH, wyH;
	zone_convert_clip( x - 3, y - 3, realm, wxL, wyL );
	zone_convert_clip( x + 3, y + 3, realm, wxH, wyH );
	for( unsigned short wx = wxL; wx <= wxH; ++wx )
	{
		for( unsigned short wy = wyL; wy <= wyH; ++wy )
		{
			ZoneItems& witem = realm->zone[wx][wy].items;
			for( ZoneItems::iterator itr = witem.begin(), end = witem.end(); itr != end; ++itr )
			{

				Item* item = *itr;
				if (inrange( this, item )) // FIXME needs to check smaller range.
				{
					if (item->serial == serial)
					{
						if (can_move( item ))
						{
							item->set_dirty();
							remove_item_from_world( item );
														
							*found_in = NULL;
							return item;
						}
						else
						{
							return NULL;
						}
					}
					if (item->isa(UObject::CLASS_CONTAINER))
					{
						item = ((UContainer *) item)->remove( serial, found_in );
						if (item)
							return item;
					}
				}
			}
		}
	}
	*found_in = NULL;
	return NULL;
}

bool Character::target_cursor_busy() const
{
	if (tcursor2 != NULL)
		return true;
	if (client && client->gd && client->gd->target_cursor_uoemod != NULL)
		return true;
	return false;
}

// get_legal_item removed, wasn't being used. - MuadDib

void Character::cancel_menu()
{
	menu = NULL;
	if (on_menu_selection != NULL)
		on_menu_selection( client, NULL, NULL );
	on_menu_selection = NULL;
}

bool Character::is_trading() const
{
	return (trading_with.get() != NULL);
}

void Character::create_trade_container()
{
	if (trading_cont.get() == NULL)	 // FIXME hardcoded
	{
		Item* cont = Item::create( EXTOBJ_SECURE_TRADE_CONTAINER );
		cont->realm = realm;
		trading_cont.set( static_cast<UContainer*>(cont) );
	}
}

UContainer* Character::trade_container()
{
	return trading_cont.get();
}

// SkillValue removed for no use - MuadDib

const char* Character::target_tag() const
{
	return "mobile";
}

void Character::set_caps_to_default()
{
	
	for( unsigned ai = 0; ai < numAttributes; ++ai ) {
		
		Attribute* pAttr = ::attributes[ai];
		AttributeValue& av = attribute(ai);
		
		av = attribute(ai);
		av.cap(pAttr->default_cap);
	}
}