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
#include "../../clib/logfacility.h"
#include "../../clib/random.h"

#include "../../bscript/bstruct.h"
#include "../../bscript/impstr.h"
#include "../../bscript/objmembers.h"

#include "../../plib/pkg.h"
#include "../../plib/realm.h"

#include "../action.h"
#include "../extobj.h"
#include "../mobile/attribute.h"
#include "../mobile/charactr.h"
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
namespace Pol {
  namespace Items {
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

    Core::UACTION remove_action( Clib::ConfigElem& elem, const char* name, Core::UACTION deflt )
	{
	  unsigned short tmp = elem.remove_ushort( name, static_cast<unsigned short>( deflt ) );
      if ( Core::UACTION_IS_VALID( tmp ) )
	  {
        return static_cast<Core::UACTION>( tmp );
	  }
	  else
	  {
		elem.throw_error( string( name ) + " is out of range" );
        return Core::ACTION__LOWEST;
	  }
	}
    Core::UACTION remove_action( Clib::ConfigElem& elem, const char* name )
	{
	  unsigned short tmp = elem.remove_ushort( name );
      if ( Core::UACTION_IS_VALID( tmp ) )
	  {
        return static_cast<Core::UACTION>( tmp );
	  }
	  else
	  {
		elem.throw_error( string( name ) + " is out of range" );
        return Core::ACTION__LOWEST;
	  }
	}

    Core::UACTION default_anim( bool two_handed )
	{
	  if ( two_handed )
        return Core::ACTION_RIDINGHORSE7;
	  else
        return Core::ACTION_RIDINGHORSE4;
	}

    WeaponDesc::WeaponDesc( u32 objtype, Clib::ConfigElem& elem, const Plib::Package* pkg ) :
	  EquipDesc( objtype, elem, WEAPONDESC, pkg ),
	  pAttr( NULL ),
	  delay( elem.remove_ushort( "DELAY", 0 ) ),
	  hit_script( elem.remove_string( "HITSCRIPT", "" ), pkg, "scripts/items/" ),
	  damage_dice(),
	  projectile( elem.remove_bool( "PROJECTILE", false ) ),
	  projectile_type( 0 ),
	  projectile_anim( 0 ),
	  projectile_sound( 0 ),
      projectile_action( static_cast<Core::UACTION>( 0 ) ),
      anim( remove_action( elem, "Anim", Core::ACTION_HUMAN_ATTACK ) ),
	  mounted_anim( remove_action( elem, "MountedAnim", anim ) ),
	  hit_sound( elem.remove_ushort( "HITSOUND", 0 ) ),
	  miss_sound( elem.remove_ushort( "MISSSOUND", 0 ) ),
	  is_intrinsic( false ),
	  is_pc_weapon( false ),
	  two_handed( elem.remove_bool( "TWOHANDED", false ) ),
	  minrange( elem.remove_ushort( "MINRANGE", projectile ? 2 : 0 ) ),
	  maxrange( elem.remove_ushort( "MAXRANGE", projectile ? 20 : 1 ) )
	{
	  if ( delay == 0 )
		speed = elem.remove_ushort( "SPEED" );
	  else
		speed = 35;

	  string attrname = elem.remove_string( "Attribute", "" );
	  if ( attrname.empty() )
	  {
		attrname = elem.remove_string( "SkillID", "" );

		// throw the error this time
		if ( attrname.empty() )
		  attrname = elem.remove_string( "Attribute" );

		elem.warn( "Element specifies a SkillID instead of an Attribute" );
	  }

	  pAttr = Mobile::FindAttribute( attrname );
	  if ( !pAttr )
	  {
		elem.throw_error( "Attribute " + attrname + " not found" );
	  }

	  if ( elem.has_prop( "INTRINSIC" ) )
	  {
		elem.throw_error( "Weapon has INTRINSIC property, which is no longer needed" );
	  }

	  if ( speed <= 0 )
	  {
		elem.throw_error( "Weapon has illegal Speed value (Speed must be positive)" );
	  }
	  if ( maxhp <= 0 )
	  {
		elem.throw_error( "Weapon has illegal MaxHP value (MaxHP must be positive)" );
	  }

	  string errmsg;
	  if ( !damage_dice.load( elem.remove_string( "DAMAGE" ).c_str(), &errmsg ) )
	  {
		elem.throw_error( "Error parsing DAMAGE string for WeaponTemplate\n" + errmsg );

		/*cerr << "Error parsing DAMAGE string for WeaponTemplate " << objtype << endl;
		cerr << "	" << errmsg << endl;
		throw runtime_error( "Configuration error" );*/
	  }


	  if ( projectile )
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

	void WeaponDesc::PopulateStruct( Bscript::BStruct* descriptor ) const
	{
      using namespace Bscript;
	  base::PopulateStruct( descriptor );
	  descriptor->addMember( "Speed", new BLong( speed ) );
	  descriptor->addMember( "Delay", new BLong( delay ) );

	  descriptor->addMember( "Projectile", new BLong( projectile ) );
	  descriptor->addMember( "ProjectileType", new BLong( projectile_type ) );
	  descriptor->addMember( "ProjectileAnim", new BLong( projectile_anim ) );
	  descriptor->addMember( "ProjectileSound", new BLong( projectile_sound ) );

	  descriptor->addMember( "HitSound", new BLong( hit_sound ) );
	  descriptor->addMember( "MissSound", new BLong( miss_sound ) );

	  descriptor->addMember( "MinRange", new BLong( minrange ) );
	  descriptor->addMember( "MaxRange", new BLong( maxrange ) );

	  //descriptor->addMember( "Intrinsic", new BLong(is_intrinsic) );
	  descriptor->addMember( "TwoHanded", new BLong( two_handed ) );

	  descriptor->addMember( "Anim", new BLong( anim ) );
	  descriptor->addMember( "MountedAnim", new BLong( mounted_anim ) );

	  std::string tempstr;
	  damage_dice.die_string( tempstr );
	  descriptor->addMember( "Damage", new String( tempstr ) );
	  descriptor->addMember( "MinDamage", new BLong( damage_dice.min_value() ) );
	  descriptor->addMember( "MaxDamage", new BLong( damage_dice.max_value() ) );

	  descriptor->addMember( "HitScript", new String( hit_script.relativename( pkg ) ) );
	  descriptor->addMember( "Attribute", new String( pAttr->name ) );

	}

	typedef std::map<string, UWeapon*> IntrinsicWeapons;
	IntrinsicWeapons intrinsic_weapons;
	void load_npc_weapon_templates();

	UWeapon* find_intrinsic_weapon( const string& name )
	{
	  auto itr = intrinsic_weapons.find( name );
	  if ( itr != intrinsic_weapons.end() )
	  {
		return ( *itr ).second;
	  }
	  else
	  {
		return NULL;
	  }
	}

	void allocate_intrinsic_weapon_serials()
	{
	  for ( const auto &intrinsic_weapon : intrinsic_weapons )
	  {
		UWeapon* wpn = intrinsic_weapon.second;

		wpn->serial = Core::GetNewItemSerialNumber();
		wpn->serial_ext = ctBEu32( wpn->serial );
		Core::objecthash.Insert( wpn );
	  }
	}

	UWeapon* create_intrinsic_weapon( const char* name, Clib::ConfigElem& elem, const Plib::Package* pkg )
	{
	  auto tmpl = new WeaponDesc( Core::extobj.wrestling, elem, pkg );
	  tmpl->is_intrinsic = true;
	  auto wpn = new UWeapon( *tmpl, tmpl );
	  wpn->tmpl = tmpl;

	  wpn->inuse( true );

	  // during system startup, defer serial allocation in order to avoid clashes with 
	  // saved items.
      if ( !Core::gflag_in_system_startup )
	  {
        wpn->serial = Core::GetNewItemSerialNumber( );
		wpn->serial_ext = ctBEu32( wpn->serial );
        Core::objecthash.Insert( wpn );
	  }

	  intrinsic_weapons.insert( IntrinsicWeapons::value_type( name, wpn ) );
	  return wpn;
	}

	void load_weapon_templates()
	{
	  load_npc_weapon_templates();
	}

	void load_intrinsic_weapons()
	{
      const ItemDesc& id = find_itemdesc( Core::extobj.wrestling );
	  if ( id.save_on_exit )
        throw runtime_error( "Wrestling weapon " + Clib::hexint( Core::extobj.wrestling ) + " must specify SaveOnExit 0" );

	  if ( id.type == ItemDesc::WEAPONDESC )
	  {
		const WeaponDesc* weapon_descriptor = static_cast<const WeaponDesc*>( &id );
        Core::wrestling_weapon = new UWeapon( *weapon_descriptor, weapon_descriptor );

		{
		  // sets wrestling weapondesc as intrinsic
		  WeaponDesc* wdesc = const_cast<WeaponDesc*>( weapon_descriptor );
		  wdesc->is_intrinsic = true;
		  wdesc->is_pc_weapon = true;
		}
        Core::wrestling_weapon->inuse( true );

        intrinsic_weapons.insert( IntrinsicWeapons::value_type( "PC_weapon", Core::wrestling_weapon ) );

	  }

	  // wrestling_weapon = find_intrinsic_weapon( "Wrestling" );
      if ( Core::wrestling_weapon == NULL )
		throw runtime_error( "A WeaponTemplate for Wrestling is required in itemdesc.cfg" );
	}

	void unload_intrinsic_weapons()
	{
      if ( Core::wrestling_weapon != NULL )
	  {
        Core::wrestling_weapon->destroy( );
        Core::wrestling_weapon = NULL;
	  }
	}

	UWeapon* create_intrinsic_weapon_from_npctemplate( Clib::ConfigElem& elem, const Plib::Package* pkg )
	{
	  string tmp;
	  if ( elem.remove_prop( "AttackSpeed", &tmp ) )
	  {
		// Construct a WeaponTemplate for this NPC template.
		Clib::ConfigElem wpnelem;
		wpnelem.set_rest( elem.rest() );
		wpnelem.set_source( elem );
		wpnelem.add_prop( "Objtype", "0xFFFF" );
		wpnelem.add_prop( "Graphic", "1" );
		wpnelem.add_prop( "Speed", tmp.c_str() );

		if ( elem.remove_prop( "AttackDelay", &tmp ) )
		  wpnelem.add_prop( "Delay", tmp.c_str() );

		wpnelem.add_prop( "Damage", elem.remove_string( "AttackDamage" ).c_str() );

		if ( elem.has_prop( "AttackSkillId" ) )
		  wpnelem.add_prop( "SkillId", elem.remove_string( "AttackSkillId" ).c_str() );
		else
		  wpnelem.add_prop( "Attribute", elem.remove_string( "AttackAttribute" ).c_str() );

		wpnelem.add_prop( "HitSound", elem.remove_string( "AttackHitSound", "0" ).c_str() );
		wpnelem.add_prop( "MissSound", elem.remove_string( "AttackMissSound", "0" ).c_str() );
		wpnelem.add_prop( "Anim", elem.remove_string( "AttackAnimation", "0" ).c_str() );
		wpnelem.add_prop( "MaxHp", "1" );
		if ( elem.remove_prop( "AttackHitScript", &tmp ) )
		  wpnelem.add_prop( "HitScript", tmp.c_str() );
		if ( elem.remove_prop( "AttackMinRange", &tmp ) )
		  wpnelem.add_prop( "MinRange", tmp.c_str() );
		if ( elem.remove_prop( "AttackMaxRange", &tmp ) )
		  wpnelem.add_prop( "MaxRange", tmp.c_str() );

		if ( elem.has_prop( "AttackProjectile" ) )
		{
		  wpnelem.add_prop( "Projectile", "1" );
		  wpnelem.add_prop( "ProjectileType", elem.remove_string( "AttackProjectileType" ).c_str() );
		  wpnelem.add_prop( "ProjectileAnim", elem.remove_string( "AttackProjectileAnim" ).c_str() );
		  wpnelem.add_prop( "ProjectileSound", elem.remove_string( "AttackProjectileSound" ).c_str() );
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
	  if ( Clib::FileExists( "config/npcdesc.cfg" ) )
	  {
        Clib::ConfigFile cf( "config/npcdesc.cfg" );
        Clib::ConfigElem elem;
		while ( cf.read( elem ) )
		{
		  create_intrinsic_weapon_from_npctemplate( elem, NULL );
		}
	  }
	  for ( const auto &pkg : Plib::packages )
	  {

		string filename = Plib::GetPackageCfgPath( pkg, "npcdesc.cfg" );

        if ( Clib::FileExists( filename.c_str( ) ) )
		{
          Clib::ConfigFile cf( filename.c_str( ) );
          Clib::ConfigElem elem;
		  while ( cf.read( elem ) )
		  {
			string newrest = ":" + pkg->name() + ":" + string( elem.rest() );
			elem.set_rest( newrest.c_str() );
			create_intrinsic_weapon_from_npctemplate( elem, pkg );
		  }
		}
	  }
	}

	static void kill_weapon( IntrinsicWeapons::value_type t )
	{
	  //	t.second->serial = 1; // just to force the delete to work.
	  if ( t.second != NULL )
	  {
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
	  hit_script_( descriptor.hit_script )
	{}
	extern map<u32, ItemDesc*> desctable;

	UWeapon::~UWeapon()
	{
	  // Every intrinsic weapon has its own local itemdesc element that should be deleted here.
	  // Only exception is the wrestling weapon, which should be deferred 
	  // to the global desctable cleaning.

	  if ( is_intrinsic() && tmpl != NULL )
	  {
		WeaponDesc* wd = const_cast<WeaponDesc*>( tmpl );
		if ( !wd->is_pc_weapon )
		{
		  wd->unload_scripts();

		  delete tmpl;
		  tmpl = NULL;
		}
	  }
	}

	unsigned short UWeapon::speed() const
	{
	  int speed_ = tmpl->speed + getmember<s16>( Bscript::MBR_SPEED_MOD );

	  if ( speed_ < 0 )
		return 0;
	  else if ( speed_ <= USHRT_MAX )
		return static_cast<u16>( speed_ );
	  else
		return USHRT_MAX;
	}

	unsigned short UWeapon::delay() const
	{
	  return tmpl->delay;
	}

	unsigned short UWeapon::damage_mod() const
	{
      return getmember<s16>( Bscript::MBR_DMG_MOD );
	}

	const Mobile::Attribute& UWeapon::attribute() const
	{
	  return *( tmpl->pAttr );
	}


	unsigned short UWeapon::get_random_damage() const
	{
	  int dmg = int( tmpl->get_random_damage() ) * hp_ / maxhp();
      dmg += getmember<s16>( Bscript::MBR_DMG_MOD );
	  if ( dmg < 0 )
		return 0;
	  else if ( dmg <= USHRT_MAX )
		return static_cast<unsigned short>( dmg );
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

	Core::UACTION UWeapon::anim() const
	{
	  return tmpl->anim;
	}

	Core::UACTION UWeapon::mounted_anim() const
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

	bool UWeapon::consume_projectile( Core::UContainer* cont ) const
	{
	  Item* item = cont->find_objtype_noninuse( tmpl->projectile_type );
	  if ( item != NULL )
	  {
		subtract_amount_from_item( item, 1 );
		return true;
	  }
	  else
	  {
		return false;
	  }
	}

	bool UWeapon::in_range( const Mobile::Character* wielder,
							const Mobile::Character* target ) const
	{
	  unsigned short dist = pol_distance( wielder, target );
      INFO_PRINT_TRACE( 22 ) << "in_range(0x" << fmt::hexu(wielder->serial) << ",0x" << fmt::hexu(target->serial) << "):\n"
        << "dist:	 " << dist << "\n"
        << "minrange: " << tmpl->minrange << "\n"
        << "maxrange: " << tmpl->maxrange << "\n"
        << "has_los:  " << wielder->realm->has_los( *wielder, *target ) << "\n";
	  return ( dist >= tmpl->minrange &&
			   dist <= tmpl->maxrange &&
			   wielder->realm->has_los( *wielder, *target ) );
	}

	// FIXME weak, weak..
	Item* UWeapon::clone() const
	{
	  UWeapon* wpn = static_cast<UWeapon*>( base::clone() );
	  wpn->hit_script_ = hit_script_;
      wpn->setmember<s16>( Bscript::MBR_DMG_MOD, this->getmember<s16>( Bscript::MBR_DMG_MOD ) );
      wpn->setmember<s16>( Bscript::MBR_SPEED_MOD, this->getmember<s16>( Bscript::MBR_SPEED_MOD ) );
	  return wpn;
	}

	void UWeapon::printProperties( Clib::StreamWriter& sw ) const
	{
	  base::printProperties( sw );

      short speed_mod = getmember<s16>( Bscript::MBR_SPEED_MOD );
      short dmg_mod = getmember<s16>( Bscript::MBR_DMG_MOD );

	  if ( dmg_mod )
		sw() << "\tdmg_mod\t" << dmg_mod << pf_endl;
	  if ( speed_mod )
		sw() << "tspeed_mod\t" << speed_mod << pf_endl;
	  if ( !( hit_script_ == tmpl->hit_script ) )
		sw() << "\tHitScript\t" << hit_script_.relativename( tmpl->pkg ) << pf_endl;
	}

	void UWeapon::readProperties( Clib::ConfigElem& elem )
	{
	  base::readProperties( elem );

      setmember<s16>( Bscript::MBR_DMG_MOD, static_cast<short>( elem.remove_int( "DMG_MOD", 0 ) ) );
      setmember<s16>( Bscript::MBR_SPEED_MOD, static_cast<s16>( elem.remove_int( "SPEED_MOD", 0 ) ) );

	  // if the HITSCRIPT is not specified in the data file, keep the value from the template.
	  if ( elem.has_prop( "HITSCRIPT" ) )
		set_hit_script( elem.remove_string( "HITSCRIPT" ) );
	}

	void UWeapon::set_hit_script( const string& scriptname )
	{
	  if ( scriptname.empty() )
	  {
		hit_script_.clear();
	  }
	  else
	  {
		hit_script_.config( scriptname,
							tmpl->pkg,
							"scripts/items/",
							true );
	  }
	}
  }
}