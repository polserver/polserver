/*
History
=======
2009/09/20 MuadDib:   docast no longer unhides. Let scripts handle this.
2010/01/15 Turley:    (Tomi) SpeakPowerWords font and color params

Notes
=======

*/

#include "../clib/stl_inc.h"
#ifdef _MSC_VER
 #pragma warning( disable: 4786 )
#endif


#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/endian.h"
#include "../clib/fileutil.h"

#include "action.h"
#include "mobile/charactr.h"
#include "network/client.h"
#include "extcmd.h"
#include "item/itemdesc.h"
#include "miscrgn.h"
#include "mkscrobj.h"
#include "objtype.h"
#include "../plib/pkg.h"
#include "pktin.h"
#include "polcfg.h"
#include "polclass.h"
#include "polsig.h"
#include "schedule.h"
#include "scrstore.h"
#include "sockio.h"
#include "spelbook.h"
#include "spells.h"
#include "syshook.h"
#include "target.h"
#include "ufunc.h"
#include "umanip.h"
#include "vital.h"

// Magery is repeated at array entry 3, because as of right now, NO spellbook
// on OSI uses the 301+ spellrange that we can find. 5/30/06 - MuadDib
// We use Mysticism at array entry 3 because Mysticism spellids are 678 -> 693 and this slot is free.
u32 spell_scroll_objtype_limits[8][2] = { {0x1F2D,0x1F6C}, {0x2260,0x226F},
										  {0x2270,0x227C}, {0x2D9E,0x2DAD},
{0x238D,0x2392}, {0x23A1,0x23A8}, {0x2D51,0x2D60}, {0x574B,0x5750}};

static bool nocast_here( Character* chr )
{
	NoCastRegion* rgn = nocastdef->getregion( chr->x, chr->y, chr->realm );
	if (rgn == NULL)
	{
		return false;
	}
	else
	{
		return rgn->nocast();
	}
}

bool knows_spell( Character* chr, u16 spellid )
{
	//copied code from Character::spellbook to support multiple spellbooks in the pack
	Item* item = chr->wornitem( LAYER_HAND1 );
	if (item != NULL && item->script_isa(POLCLASS_SPELLBOOK) )
	{
		Spellbook* book = static_cast<Spellbook*>(item);
		if(book->has_spellid( spellid ))
			return true;
	}

	UContainer* cont = chr->backpack();
	if (cont != NULL)
	{
		for( UContainer::const_iterator itr = cont->begin(), end = cont->end(); itr != end; ++itr )
		{
			const Item* _item = GET_ITEM_PTR( itr );
			
			if(_item != NULL && _item->script_isa(POLCLASS_SPELLBOOK))
			{
				const Spellbook* book = static_cast<const Spellbook*>(_item);
				if(book->has_spellid( spellid ))
					return true;
			}
		}
	}

	return false;
}

bool hands_are_free( Character* chr )
{
	Item* item;
	
	item = chr->wornitem( LAYER_HAND1 );
	if (item != NULL)
	{
		const ItemDesc& id = item->itemdesc();
		if (id.blocks_casting_if_in_hand)
			return false;
	}

	item = chr->wornitem( LAYER_HAND2 );
	if (item != NULL)
	{
		const ItemDesc& id = item->itemdesc();
		if (id.blocks_casting_if_in_hand)
			return false;
	}

	return true;
}

USpellParams::USpellParams() :
	manacost(0),
	difficulty(0),
	pointvalue(0),
	delay(0)
{
}

USpellParams::USpellParams( ConfigElem& elem ) :
	manacost( elem.remove_ushort( "MANA" ) ),
	difficulty( elem.remove_ushort( "DIFFICULTY" ) ),
	pointvalue( elem.remove_ushort( "POINTVALUE" ) ),
	delay( elem.remove_ushort( "DELAY" ) )
{
}

class SpellCircle
{
public:
	SpellCircle( ConfigElem& elem );

public:
	USpellParams params;

private:
	// not implemented:
	SpellCircle( const SpellCircle& );
	SpellCircle& operator=( const SpellCircle& );
};

SpellCircle::SpellCircle( ConfigElem& elem ) :
	params( elem )
{
}



vector<SpellCircle*> spellcircles;

USpell::USpell( ConfigElem& elem, Package* pkg ) :
	pkg_(pkg),
	spellid_( elem.remove_ushort( "SPELLID" ) ),
	name_( elem.remove_string( "NAME" )),
	power_words_( elem.remove_string( "POWERWORDS" ) ),
	scriptdef_( elem.remove_string( "SCRIPT", "" ), pkg, "scripts/" )
{
	unsigned short action;
	if (elem.remove_prop( "ANIMATION", &action ))
	{
		if (UACTION_IS_VALID( action ))
		{
			action_ = static_cast<UACTION>(action);
		}
		else
		{
			elem.throw_error( "Animation is out of range" );
		}
	}
	else
	{
		action_ = ACTION_CAST_SPELL1;
	}

	unsigned short circle;
	if (elem.remove_prop( "CIRCLE", &circle ))
	{
		if (circle < 1 || circle > spellcircles.size() ||
			spellcircles[ circle-1 ] == NULL)
		{
			cerr << "Error reading spell " << name_
				 << ": Circle " << circle << " is not defined."
				 << endl;
			throw runtime_error( "Config file error" );
		}

		params_ = spellcircles[circle-1]->params;
	}
	else
	{
		params_ = USpellParams( elem );
	}
	
	std::string reagent_name;
	while (elem.remove_prop( "Reagent", &reagent_name ))
	{
		unsigned short reagent = get_objtype_from_string( reagent_name );

		reglist_.push_back(reagent);
	}
}	

void USpell::cast( Character* chr )
{
	if (nocast_here( chr ))
	{
		if (chr->client != NULL)
			send_sysmessage( chr->client, "Spells cannot be cast here." );
		return;
	}

	if (!scriptdef_.empty())
	{
		ref_ptr<EScriptProgram> prog = find_script2( scriptdef_,
													true,
													config.cache_interactive_scripts );

		if (prog.get() != NULL)
		{
			if (chr->start_spell_script( prog.get(), this ))
				return;
		}

	}

	if (chr->client != NULL)
		send_sysmessage( chr->client, "That spell doesn't seem to work." );
}

bool USpell::consume_reagents( Character *chr )
{
	UContainer* bp = chr->backpack();
	if (bp == NULL)
		return false;

	for( RegList::iterator itr = reglist_.begin(), end =reglist_.end();
		 itr != end;
		 ++itr )
	{
		Item* item = bp->find_objtype_noninuse( *itr );
		if (item == NULL)
			return false;
		subtract_amount_from_item(item, 1);
	}
	
	return true;
}

bool USpell::check_mana( Character *chr )
{
	return (chr->vital(pVitalMana->vitalid).current_ones() >= manacost());
}

bool USpell::check_skill( Character *chr )
{
	return chr->check_skill( SKILLID_MAGERY, params_.difficulty, params_.pointvalue );
}

void USpell::consume_mana( Character *chr )
{
	chr->consume( pVitalMana, chr->vital(pVitalMana->vitalid), manacost() * 100 );
}

void USpell::speak_power_words( Character* chr, unsigned short font, unsigned short color )
{
	if ( chr->client != NULL && chr->hidden() )
	{
		private_say_above(chr, chr, power_words_.c_str(), font, color);
	}
	else if ( !chr->hidden() )
	{
		say_above( chr, power_words_.c_str(), font, color );
	}
}

SpellTask::SpellTask( OneShotTask** handle, polclock_t run_when_clock, Character* caster, USpell* spell, bool dummy ) :
	OneShotTask( handle, run_when_clock ),
	caster_(caster),
	spell_(spell)
{
}

void play_spell_fizzle( Character* caster )
{
	play_object_centered_effect( caster, 
								 0x3735, // HARDCODE
								 0,
								 30 );
	play_sound_effect( caster, 0x5D ); // HARDCODE
}

void SpellTask::on_run()
{
	THREAD_CHECKPOINT( tasks, 900 );
	Character* caster = caster_.get();
	if (!caster->orphan())
	{
		THREAD_CHECKPOINT( tasks, 911 );
		spell_->cast( caster );
		THREAD_CHECKPOINT( tasks, 912 );
	}
	THREAD_CHECKPOINT( tasks, 999 );
}

vector<USpell*> spells2;


void do_cast( Client *client, u16 spellid )
{
	if (system_hooks.on_cast_hook != NULL)
	{
		if (system_hooks.on_cast_hook->call(make_mobileref(client->chr),new BLong(spellid)))
			return;
	}
	// CHECKME should this look at spellnum, instead? static_cast behavior undefined if out of range.
	if (spellid > spells2.size())
		return;

	USpell *spell = spells2[ spellid ];
	if (spell == NULL)
	{
		cerr << "Spell " << spellid << " is not implemented." << endl;
		send_sysmessage( client, "That spell does not function." );
		return;
	}

// Let scripts handle this.
//	if (client->chr->hidden())
//		client->chr->unhide();

	if (client->chr->frozen())
	{
		private_say_above( client->chr, client->chr, "I am frozen and cannot cast spells" );
		return;
	}

	if (client->chr->paralyzed())
	{
		private_say_above( client->chr, client->chr, "I am paralyzed and cannot cast spells" );
		return;
	}

	if (client->chr->skill_ex_active())
	{
		send_sysmessage( client, "You are already doing something else." );
		return;
	}

	if (client->chr->casting_spell())
	{
		send_sysmessage( client, "You are already casting a spell." );
		return;
	}

	if (nocast_here( client->chr ))
	{
		send_sysmessage( client, "Spells cannot be cast here." );
		return;
	}

	if (config.require_spellbooks)
	{
		if (!knows_spell( client->chr, spellid ))
		{
			send_sysmessage( client, "You don't know that spell." );
			return;
		}
	}

#if 1
	client->chr->schedule_spell( spell );
#else
	spell->cast( client );
	client->restart();
#endif
}

void handle_cast_spell( Client *client, PKTIN_12 *msg )
{
	u16 spellnum = static_cast<u16>(strtoul( (char*) msg->data, NULL, 10 ));

	do_cast( client, spellnum );
}
ExtendedMessageHandler spell_msg_handler1( EXTMSGID_CASTSPELL1, handle_cast_spell );
ExtendedMessageHandler spell_msg_handler2( EXTMSGID_CASTSPELL2, handle_cast_spell );

void handle_open_spellbook( Client *client, PKTIN_12 *msg )
{
	if (system_hooks.open_spellbook_hook != NULL)
	{
		if (system_hooks.open_spellbook_hook->call( make_mobileref(client->chr) ))
			return;
	}

	if (client->chr->dead())
	{
		send_sysmessage( client, "I am dead and cannot do that." );
		return;
	}

	
	Item* spellbook = client->chr->wornitem( LAYER_HAND1 );
	if (spellbook == NULL)
	{
		UContainer* backpack = client->chr->backpack();
		if (backpack != NULL)
		{
			spellbook = backpack->find_toplevel_polclass( POLCLASS_SPELLBOOK );
			
				//
				// Client crashes if the pack isn't open and you don't tell him
				// about the spellbook
				//
			if (spellbook != NULL)
				send_put_in_container( client, spellbook );
		}
	}

	if (spellbook != NULL)
	{
		spellbook->double_click( client );
	}
}
ExtendedMessageHandler open_spellbook_handler( EXTMSGID_SPELLBOOK, handle_open_spellbook );

void register_spell( USpell *spell, unsigned short spellid )
{
	if (spellid >= spells2.size())
	{
		spells2.resize( spellid + 1, 0 );
	}
	
	if (spells2[ spellid ])
	{
		USpell* origspell = spells2[spellid];
		cerr << "Spell ID " << spellid << " (" << origspell->name() << ") multiply defined" << endl;
		if (origspell->pkg_ != NULL)
		{
			cerr << "	Spell originally defined in package '" 
					<< origspell->pkg_->name() << "' (" << origspell->pkg_->dir() << ")" << endl;
		}
		else
		{
			cerr << "	Spell originally defined in main" << endl;
		}
		if (spell->pkg_ != NULL)
		{
			cerr << "	Spell redefined in package '" 
					<< spell->pkg_->name() << "' (" << spell->pkg_->dir() << ")" << endl;
		}
		else
		{
			cerr << "	Spell redefined in main" << endl;
		}
		throw runtime_error( "Spell ID multiply defined" );
	}
	
	spells2[ spellid ] = spell;
}




void load_circle_data()
{
	if ( !FileExists("config/circles.cfg") )
	{
		if ( config.loglevel > 1 )
			cout << "File config/circles not found, skipping.\n";
		return;
	}

	ConfigFile cf("config/circles.cfg", "Circle");
	ConfigElem elem;

	while ( cf.read(elem) )
	{
		int index = strtoul( elem.rest(), NULL, 0 ) - 1;
		if ( index < 0 || index >= 100 )
		{
			cerr << "Error in CIRCLES.CFG: Circle must fall between 1 and 100" << endl;
			throw runtime_error("Config file error");
		}

		spellcircles.resize(index+1, NULL);

		if ( spellcircles[ index ] != NULL )
		{
			cerr << "Error in CIRCLES.CFG: Circle " << index+1 << " is multiply defined." << endl;
			throw runtime_error("Config file error");
		}

		spellcircles[ index ] = new SpellCircle(elem);
	}
}

void load_spells_cfg( const char* path, Package* pkg )
{
	ConfigFile cf( path, "Spell" );
	ConfigElem elem;

	while (cf.read( elem ))
	{
		std::unique_ptr<USpell> spell( new USpell(elem, pkg) );

		unsigned short spellid = spell->spell_id();

		register_spell( spell.release(), spellid );
	}
}

void load_spell_data()
{
	load_circle_data();

	if ( FileExists("config/spells.cfg") )
		load_spells_cfg("config/spells.cfg", NULL);
	else if ( config.loglevel > 1 )
		cout << "File config/spells.cfg not found, skipping\n";

	for( Packages::iterator itr = packages.begin(); itr != packages.end(); ++itr )
	{
		Package* pkg = (*itr);
		string filename = GetPackageCfgPath(pkg, "spells.cfg");
		if ( FileExists(filename.c_str()) )
		{
			load_spells_cfg(filename.c_str(), pkg);
		}
	}
}

void clean_spells()
{
	std::vector<SpellCircle*>::iterator c_iter = spellcircles.begin();
	for ( ; c_iter != spellcircles.end(); ++c_iter) {
		delete *c_iter;
		*c_iter=NULL;
	}
	spellcircles.clear();
	std::vector<USpell*>::iterator s_iter = spells2.begin();
	for ( ; s_iter != spells2.end(); ++s_iter) {
		delete *s_iter;
		*s_iter=NULL;
	}
	spells2.clear();
}

