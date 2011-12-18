/*
History
=======
2005/03/09 Shinigami: Added Prop Delay [ms]
2005/07/25 Shinigami: added MinDamage and MaxDamage in Weapon-Descriptor
2011/11/12 Tomi:	  added extobj.wrestling

Notes
=======

*/

#include "../../clib/stl_inc.h"

#ifdef _MSC_VER
#	pragma warning( disable:4786 )
#endif

#include <algorithm>
#include <iostream>
#include <map>
#include <string>

#include <limits.h>

#include "../../clib/cfgelem.h"
#include "../../clib/cfgfile.h"
#include "../../clib/endian.h"
#include "../../clib/fileutil.h"
#include "../../clib/random.h"

#include "../../bscript/bstruct.h"
#include "../../bscript/impstr.h"

#include "../../plib/pkg.h"
#include "../../plib/realm.h"

#include "../action.h"
#include "../extobj.h"
#include "../mobile/attribute.h"
#include "../mobile/charactr.h"
#include "../dtrace.h"
#include "../gflag.h"
#include "../los.h"
#include "../objecthash.h"
#include "../polcfg.h"
#include "../realms.h"
#include "../skillid.h"
#include "../ufunc.h"
#include "../umanip.h"
#include "../uvars.h"
#include "wepntmpl.h"

#include "weapon.h"

/*
WeaponDesc::WeaponDesc() :
	skillid( SKILLID_WRESTLING ),
	speed( 100 ),
	delay( 0 ),
	damage_dice(),
	projectile(false),
	projectile_type(0),
	maxhp(1)
{
}
*/

UACTION remove_action( ConfigElem& elem, const char* name, UACTION deflt )
{
	unsigned short tmp = elem.remove_ushort( name, static_cast<unsigned short>(deflt) ); 
	if (UACTION_IS_VALID( tmp ))
	{
		return static_cast<UACTION>(tmp);
	}
	else
	{
		elem.throw_error( string(name) + " is out of range" );
		return ACTION__LOWEST;
	}
}
UACTION remove_action( ConfigElem& elem, const char* name )
{
	unsigned short tmp = elem.remove_ushort( name ); 
	if (UACTION_IS_VALID( tmp ))
	{
		return static_cast<UACTION>(tmp);
	}
	else
	{
		elem.throw_error( string(name) + " is out of range" );
		return ACTION__LOWEST;
	}
}
					  
UACTION default_anim( bool two_handed )
{
	if (two_handed)
		return ACTION_RIDINGHORSE7;
	else
		return ACTION_RIDINGHORSE4;
}

WeaponDesc::WeaponDesc( u32 objtype, ConfigElem& elem, const Package* pkg ) :
	EquipDesc( objtype, elem, WEAPONDESC, pkg ),
	pAttr( NULL ),
	delay( elem.remove_ushort( "DELAY", 0 ) ),
	hit_script( elem.remove_string( "HITSCRIPT", "" ), pkg, "scripts/items/" ),
	damage_dice(),
	projectile( elem.remove_bool( "PROJECTILE", false ) ),
	projectile_type(0),
	projectile_anim(0),
	projectile_sound(0),
	projectile_action(static_cast<UACTION>(0)),
	anim( remove_action( elem, "Anim", ACTION_HUMAN_ATTACK ) ),
	mounted_anim( remove_action( elem, "MountedAnim", anim ) ),
	hit_sound( elem.remove_ushort( "HITSOUND", 0 ) ),
	miss_sound( elem.remove_ushort( "MISSSOUND", 0 ) ),
	is_intrinsic( false ),
	two_handed( elem.remove_bool( "TWOHANDED", false ) ),
	minrange( elem.remove_ushort( "MINRANGE", projectile?2:0 ) ),
	maxrange( elem.remove_ushort( "MAXRANGE", projectile?20:1 ) )
{   
	if (delay == 0)
		speed = elem.remove_ushort( "SPEED");
	else
		speed = 35;

	string attrname = elem.remove_string( "Attribute", "" );
	if (attrname.empty())
	{
		attrname = elem.remove_string( "SkillID", "" );
		
		// throw the error this time
		if (attrname.empty())
			attrname = elem.remove_string( "Attribute" );

		elem.warn( "Element specifies a SkillID instead of an Attribute" );
	}

	pAttr = FindAttribute( attrname );
	if (!pAttr)
	{
		elem.throw_error( "Attribute " + attrname + " not found" );
	}

	if ( elem.has_prop("INTRINSIC") ) {
		elem.throw_error("Weapon has INTRINSIC property, which is no longer needed");
	}

	if (speed <= 0)
	{
		cerr << "Weapon Template " << objtype << " has an illegal Speed value." << endl;
		throw runtime_error( "config file error in weapon.cfg" );
	}
	if (maxhp <= 0)
	{
		cerr << "Weapon Template " << objtype << " has an illegal MaxHp value." << endl;
		throw runtime_error( "config file error in weapon.cfg" );
	}

	string errmsg;
	if (!damage_dice.load( elem.remove_string( "DAMAGE" ).c_str(), &errmsg ))
	{
		cerr << "Error parsing DAMAGE string for WeaponTemplate " << objtype << endl;
		cerr << "	" << errmsg << endl;
		throw runtime_error( "Configuration error" );
	}


	if (projectile)
	{
		projectile_type = elem.remove_ushort( "ProjectileType" );
		projectile_anim = elem.remove_ushort( "ProjectileAnim" );
		projectile_sound = elem.remove_ushort( "ProjectileSound" );
	}
}

unsigned short WeaponDesc::get_random_damage() const
{
	return damage_dice.roll();
}

void WeaponDesc::PopulateStruct( BStruct* descriptor ) const
{
	base::PopulateStruct( descriptor );
	descriptor->addMember( "Speed", new BLong(speed) );
	descriptor->addMember( "Delay", new BLong(delay) );

	descriptor->addMember( "Projectile", new BLong(projectile) );
	descriptor->addMember( "ProjectileType", new BLong(projectile_type) );
	descriptor->addMember( "ProjectileAnim", new BLong(projectile_anim) );
	descriptor->addMember( "ProjectileSound", new BLong(projectile_sound) );

	descriptor->addMember( "HitSound", new BLong(hit_sound) );
	descriptor->addMember( "MissSound", new BLong(miss_sound) );

	descriptor->addMember( "MinRange", new BLong(minrange) );
	descriptor->addMember( "MaxRange", new BLong(maxrange) );

	descriptor->addMember( "Intrinsic", new BLong(is_intrinsic) );
	descriptor->addMember( "TwoHanded", new BLong(two_handed) );

	descriptor->addMember( "Anim", new BLong(anim) );
	descriptor->addMember( "MountedAnim", new BLong(mounted_anim) );

	std::string tempstr;
	damage_dice.die_string(tempstr);
	descriptor->addMember( "Damage", new String(tempstr) );
	descriptor->addMember( "MinDamage", new BLong(damage_dice.min_value()) );
	descriptor->addMember( "MaxDamage", new BLong(damage_dice.max_value()) );

	descriptor->addMember( "HitScript", new String(hit_script.relativename(pkg)) );
	descriptor->addMember( "Attribute", new String(pAttr->name) );

}

typedef std::map<string, UWeapon*> IntrinsicWeapons;
IntrinsicWeapons intrinsic_weapons;
void load_npc_weapon_templates();

UWeapon* find_intrinsic_weapon( const string& name )
{
	IntrinsicWeapons::iterator itr = intrinsic_weapons.find( name );
	if (itr != intrinsic_weapons.end())
	{
		return (*itr).second;
	}
	else
	{
		return NULL;
	}
}

void allocate_intrinsic_weapon_serials()
{
	for( IntrinsicWeapons::iterator itr = intrinsic_weapons.begin(); itr != intrinsic_weapons.end(); ++itr )
	{
		UWeapon* wpn = (*itr).second;

		wpn->serial = GetNewItemSerialNumber();
		wpn->serial_ext = ctBEu32( wpn->serial );
		objecthash.Insert( wpn );
	}
}

UWeapon* create_intrinsic_weapon( const char* name, ConfigElem& elem, const Package* pkg )
{
	WeaponDesc* tmpl = new WeaponDesc( extobj.wrestling, elem, pkg );
	tmpl->is_intrinsic = true;
	UWeapon* wpn = new UWeapon(*tmpl, tmpl);
	wpn->tmpl = tmpl;

	wpn->inuse( true );

	// during system startup, defer serial allocation in order to avoid clashes with 
	// saved items.
	if ( !gflag_in_system_startup )
	{
		wpn->serial = GetNewItemSerialNumber();
		wpn->serial_ext = ctBEu32( wpn->serial );
		objecthash.Insert( wpn );
	}

	intrinsic_weapons.insert(IntrinsicWeapons::value_type(name, wpn) );
	return wpn;
}

void load_weapon_templates()
{
	load_npc_weapon_templates();
}

void load_intrinsic_weapons()
{
	if (!dont_save_itemtype( extobj.wrestling ))
		throw runtime_error( "Wrestling weapon " + hexint(extobj.wrestling) + " must specify SaveOnExit 0" );

	const ItemDesc& id = find_itemdesc( extobj.wrestling );

	if (id.type == ItemDesc::WEAPONDESC)
	{
		const WeaponDesc* weapon_descriptor = static_cast<const WeaponDesc*>(&id);
		wrestling_weapon = new UWeapon( *weapon_descriptor, weapon_descriptor );
		
		{
			// sets wrestling weapondesc as intrinsic
			WeaponDesc* wdesc = const_cast<WeaponDesc*>(weapon_descriptor);
			wdesc->is_intrinsic = true; 
		}
		wrestling_weapon->inuse(true);

		intrinsic_weapons.insert( IntrinsicWeapons::value_type("PC_weapon", wrestling_weapon) );

	}

	// wrestling_weapon = find_intrinsic_weapon( "Wrestling" );
	if (wrestling_weapon == NULL)
		throw runtime_error( "A WeaponTemplate for Wrestling is required in itemdesc.cfg" );
}

void unload_intrinsic_weapons()
{
	if (wrestling_weapon != NULL)
	{
		wrestling_weapon->destroy();
		wrestling_weapon = NULL;
	}
}

UWeapon* create_intrinsic_weapon_from_npctemplate( ConfigElem& elem, const Package* pkg )
{
	string tmp;
	if (elem.remove_prop( "AttackSpeed", &tmp ))
	{
		// Construct a WeaponTemplate for this NPC template.
		ConfigElem wpnelem;
		wpnelem.set_rest(elem.rest());
		wpnelem.set_source(elem);
		wpnelem.add_prop("Objtype", "0xFFFF");
		wpnelem.add_prop("Graphic", "1");
		wpnelem.add_prop("Speed", tmp.c_str());
		
		if ( elem.remove_prop("AttackDelay", &tmp) )
		  wpnelem.add_prop("Delay", tmp.c_str());
		
		wpnelem.add_prop("Damage", elem.remove_string("AttackDamage").c_str());
		
		if ( elem.has_prop("AttackSkillId") )
			wpnelem.add_prop("SkillId", elem.remove_string("AttackSkillId").c_str());
		else
			wpnelem.add_prop("Attribute", elem.remove_string("AttackAttribute").c_str());

		wpnelem.add_prop("HitSound", elem.remove_string("AttackHitSound", "0").c_str());
		wpnelem.add_prop("MissSound", elem.remove_string("AttackMissSound", "0").c_str());
		wpnelem.add_prop("Anim", elem.remove_string("AttackAnimation", "0").c_str());
		wpnelem.add_prop("MaxHp", "1");
		if ( elem.remove_prop("AttackHitScript", &tmp) )   
			wpnelem.add_prop("HitScript", tmp.c_str());
		if ( elem.remove_prop("AttackMinRange", &tmp) )	
			wpnelem.add_prop("MinRange", tmp.c_str());
		if ( elem.remove_prop("AttackMaxRange", &tmp ) )	
			wpnelem.add_prop("MaxRange", tmp.c_str());

		if ( elem.has_prop("AttackProjectile") )
		{
			wpnelem.add_prop("Projectile", "1");
			wpnelem.add_prop("ProjectileType", elem.remove_string("AttackProjectileType").c_str());
			wpnelem.add_prop("ProjectileAnim", elem.remove_string("AttackProjectileAnim").c_str());
			wpnelem.add_prop("ProjectileSound", elem.remove_string("AttackProjectileSound").c_str());
		}
		
		return create_intrinsic_weapon( elem.rest(), wpnelem, pkg );
	}
	else
	{
		return NULL;
	}
}

void load_npc_weapon_templates()
{
	if ( FileExists("config/npcdesc.cfg") )
	{
		ConfigFile cf("config/npcdesc.cfg");
		ConfigElem elem;
		while ( cf.read(elem) )
		{
			create_intrinsic_weapon_from_npctemplate(elem, NULL);
		}
	}
	for( Packages::iterator itr = packages.begin(); itr != packages.end(); ++itr )
	{
		Package* pkg = (*itr);
		string filename = GetPackageCfgPath(pkg, "npcdesc.cfg");
		
		if ( FileExists(filename.c_str()) )
		{
			ConfigFile cf(filename.c_str());
			ConfigElem elem;
			while ( cf.read(elem) )
			{
				string newrest = ":" + pkg->name() + ":" + string(elem.rest());
				elem.set_rest(newrest.c_str());
				create_intrinsic_weapon_from_npctemplate(elem, pkg);
			}
		}
	}
}

static void kill_weapon( IntrinsicWeapons::value_type t )
{
//	t.second->serial = 1; // just to force the delete to work.
	if (t.second != NULL) {
		t.second->destroy();
		t.second = NULL;
	}
}

void unload_weapon_templates()
{
	for_each( intrinsic_weapons.begin(), intrinsic_weapons.end(), kill_weapon );
	intrinsic_weapons.clear();
}

UWeapon::UWeapon( const WeaponDesc& descriptor, const WeaponDesc* permanent_descriptor ) :
	Equipment( descriptor, CLASS_WEAPON ),
	tmpl( permanent_descriptor ),
	dmg_mod_(0),
	hit_script_( descriptor.hit_script )
{
}
extern map<u32,ItemDesc*> desctable;

UWeapon::~UWeapon() 
{
	// Every intrinsic weapon has its own local itemdesc element that should be deleted here.
	// Only exception is the wrestling weapon, which should be deferred 
	// to the global desctable cleaning.

	if (is_intrinsic() && tmpl != NULL && tmpl->objtype != extobj.wrestling) {
		WeaponDesc* wd = const_cast<WeaponDesc*>(tmpl); 

		wd->unload_scripts();

		delete tmpl;
		tmpl = NULL;
	}
}

unsigned short UWeapon::speed() const
{
	return tmpl->speed;
}

unsigned short UWeapon::delay() const
{
	return tmpl->delay;
}

const Attribute& UWeapon::attribute() const
{
	return *(tmpl->pAttr);
}


unsigned short UWeapon::get_random_damage() const
{
	int dmg = int(tmpl->get_random_damage()) * hp_ / maxhp();
	dmg += dmg_mod_;
	if (dmg < 0)
		return 0;
	else if (dmg <= USHRT_MAX)
		return static_cast<unsigned short>(dmg);
	else
		return USHRT_MAX;
}

bool UWeapon::is_projectile() const
{
	return tmpl->projectile;
}

bool UWeapon::is_intrinsic() const
{
	return tmpl->is_intrinsic;
}

unsigned short UWeapon::projectile_sound() const
{
	return tmpl->projectile_sound;
}

unsigned short UWeapon::projectile_anim() const
{
	return tmpl->projectile_anim;
}

/*
UACTION UWeapon::projectile_action() const
{
	return tmpl->projectile_action;
}
*/

UACTION UWeapon::anim() const
{
	return tmpl->anim;
}

UACTION UWeapon::mounted_anim() const
{
	return tmpl->mounted_anim;
}

unsigned short UWeapon::hit_sound() const
{
	return tmpl->hit_sound;
}

unsigned short UWeapon::miss_sound() const
{
	return tmpl->miss_sound;
}

const WeaponDesc& UWeapon::descriptor() const
{
	return *tmpl;
}

bool UWeapon::consume_projectile( UContainer* cont ) const
{
	Item* item = cont->find_objtype_noninuse( tmpl->projectile_type );
	if (item != NULL)
	{
		subtract_amount_from_item(item, 1);
		return true;
	}
	else
	{
		return false;
	}
}

bool UWeapon::in_range( const Character* wielder,
						const Character* target ) const
{
	unsigned short dist = pol_distance( wielder, target );
	dtrace(22) << "in_range(" << wielder << "," << target << "):" << endl
			   << "dist:	 " << dist << endl
			   << "minrange: " << tmpl->minrange << endl
			   << "maxrange: " << tmpl->maxrange << endl
			   << "has_los:  " << wielder->realm->has_los(*wielder,*target) << endl;
	return ( dist >= tmpl->minrange && 
			 dist <= tmpl->maxrange &&
			 wielder->realm->has_los(*wielder, *target) );
}

// FIXME weak, weak..
Item* UWeapon::clone() const
{
	UWeapon* wpn = static_cast<UWeapon*>(base::clone());
	wpn->dmg_mod_ = dmg_mod_;
	wpn->hit_script_ = hit_script_;
	return wpn;
}

void UWeapon::printProperties( ostream& os ) const
{
	base::printProperties( os );
	if (dmg_mod_)
		os << "\tdmg_mod\t" << dmg_mod_ << pf_endl;
	if (!(hit_script_ == tmpl->hit_script))
		os << "\tHitScript\t" << hit_script_.relativename( tmpl->pkg ) << pf_endl;
}

void UWeapon::readProperties( ConfigElem& elem )
{
	base::readProperties( elem );
	
	dmg_mod_ = static_cast<short>(elem.remove_int( "DMG_MOD", 0 ));

	// if the HITSCRIPT is not specified in the data file, keep the value from the template.
	if (elem.has_prop( "HITSCRIPT" ))
		set_hit_script( elem.remove_string( "HITSCRIPT" ) );
}

void UWeapon::set_hit_script( const string& scriptname )
{
	// cout << "set_hit_script" << endl;
	if (scriptname.empty())
	{
		// cout << "clearing hit script" << endl;
		hit_script_.clear();
	}
	else
	{
		// cout << "setting hit script" << endl;
		hit_script_.config( scriptname, 
							tmpl->pkg,
							"scripts/items/",
							true );
	}
}
