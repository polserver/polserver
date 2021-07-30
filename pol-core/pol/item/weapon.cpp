/** @file
 *
 * @par History
 * - 2005/03/09 Shinigami: Added Prop Delay [ms]
 * - 2005/07/25 Shinigami: added MinDamage and MaxDamage in Weapon-Descriptor
 * - 2011/11/12 Tomi:    added extobj.wrestling
 */


#include "weapon.h"

#include <stddef.h>
#include <string>

#include "../../bscript/bstruct.h"
#include "../../bscript/executor.h"
#include "../../bscript/impstr.h"
#include "../../clib/cfgelem.h"
#include "../../clib/logfacility.h"
#include "../../clib/passert.h"
#include "../../clib/streamsaver.h"
#include "../../clib/strutil.h"
#include "../../plib/systemstate.h"
#include "../containr.h"
#include "../dice.h"
#include "../equipdsc.h"
#include "../extobj.h"
#include "../globals/settings.h"
#include "../globals/uvars.h"
#include "../layers.h"
#include "../mobile/charactr.h"
#include "../realms/realm.h"
#include "../syshookscript.h"
#include "../ufunc.h"
#include "../umanip.h"
#include "../uobject.h"
#include "itemdesc.h"
#include "wepntmpl.h"

namespace Pol
{
namespace Items
{
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
    elem.throw_error( std::string( name ) + " is out of range" );
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
    elem.throw_error( std::string( name ) + " is out of range" );
  }
}

Core::UACTION default_anim( bool two_handed )
{
  if ( two_handed )
    return Core::ACTION_RIDINGHORSE7;
  else
    return Core::ACTION_RIDINGHORSE4;
}

WeaponDesc::WeaponDesc( u32 objtype, Clib::ConfigElem& elem, const Plib::Package* pkg )
    : EquipDesc( objtype, elem, WEAPONDESC, pkg ),
      pAttr( nullptr ),
      delay( elem.remove_ushort( "DELAY", 0 ) ),
      hit_script( elem.remove_string( "HITSCRIPT", "" ), pkg, "scripts/items/" ),
      damage_dice(),
      projectile( elem.remove_bool( "PROJECTILE", false ) ),
      projectile_type( 0 ),
      projectile_anim( 0 ),
      projectile_sound( 0 ),
      anim( remove_action( elem, "Anim", Core::ACTION_HUMAN_ATTACK ) ),
      mounted_anim( remove_action( elem, "MountedAnim", anim ) ),
      hit_sound( elem.remove_ushort( "HITSOUND", 0 ) ),
      miss_sound( elem.remove_ushort( "MISSSOUND", 0 ) ),
      two_handed( elem.remove_bool( "TWOHANDED", false ) ),
      minrange( elem.remove_ushort( "MINRANGE", projectile ? 2 : 0 ) ),
      maxrange( elem.remove_ushort( "MAXRANGE", projectile ? 20 : 1 ) )
{
  if ( delay == 0 )
    speed = elem.remove_ushort( "SPEED" );
  else
    speed = 35;

  std::string attrname = elem.remove_string( "Attribute", "" );
  if ( attrname.empty() )
  {
    attrname = elem.remove_string( "SkillID", "" );

    // throw the error this time
    if ( attrname.empty() )
      attrname = elem.remove_string( "Attribute" );

    elem.warn( "Element specifies a SkillID instead of an Attribute" );
  }

  pAttr = Mobile::Attribute::FindAttribute( attrname );
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

  std::string errmsg;
  if ( !damage_dice.load( elem.remove_string( "DAMAGE" ).c_str(), &errmsg ) )
  {
    elem.throw_error( "Error parsing DAMAGE string for WeaponTemplate\n" + errmsg );

    /*cerr << "Error parsing DAMAGE string for WeaponTemplate " << objtype << endl;
    cerr << "  " << errmsg << endl;
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

  // descriptor->addMember( "Intrinsic", new BLong(is_intrinsic) );
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

size_t WeaponDesc::estimatedSize() const
{
  return base::estimatedSize() + sizeof( const Mobile::Attribute* ) /*pAttr*/
         + sizeof( unsigned short )                                 /*speed*/
         + sizeof( unsigned short )                                 /*delay*/
         + sizeof( Core::ScriptDef )                                /*hit_script*/
         + sizeof( Core::Dice )                                     /*damage_dice*/
         + sizeof( bool )                                           /*projectile*/
         + sizeof( unsigned short )                                 /*projectile_type*/
         + sizeof( unsigned short )                                 /*projectile_anim*/
         + sizeof( unsigned short )                                 /*projectile_sound*/
         + sizeof( Core::UACTION )                                  /*anim*/
         + sizeof( Core::UACTION )                                  /*mounted_anim*/
         + sizeof( unsigned short )                                 /*hit_sound*/
         + sizeof( unsigned short )                                 /*miss_sound*/
         + sizeof( bool )                                           /*two_handed*/
         + sizeof( unsigned short )                                 /*minrange*/
         + sizeof( unsigned short )                                 /*maxrange*/
         + hit_script.estimatedSize();
}

/// Creates a new intrinsic weapon and returns it
/// @param name: the unique weapon's name
/// @param elem: the config element to create from
/// @param pkg: the package
UWeapon* create_intrinsic_weapon( const char* name, Clib::ConfigElem& elem,
                                  const Plib::Package* pkg )
{
  auto tmpl = new WeaponDesc( Core::settingsManager.extobj.wrestling, elem, pkg );
  tmpl->is_intrinsic = true;
  auto wpn = new UWeapon( *tmpl, tmpl );
  wpn->layer = Core::LAYER_HAND1;
  wpn->tmpl = tmpl;
  wpn->copyprops( tmpl->props );

  Items::register_intrinsic_equipment( name, wpn );

  return wpn;
}

/// Creates the intrinsic wrestling weapon for PCs
/// must be called at startup
void load_intrinsic_weapons()
{
  const ItemDesc& id = find_itemdesc( Core::settingsManager.extobj.wrestling );
  if ( id.save_on_exit )
    throw std::runtime_error( "Wrestling weapon " +
                              Clib::hexint( Core::settingsManager.extobj.wrestling ) +
                              " must specify SaveOnExit 0" );

  if ( id.type == ItemDesc::WEAPONDESC )
  {
    const WeaponDesc* weapon_descriptor = static_cast<const WeaponDesc*>( &id );
    Core::gamestate.wrestling_weapon = new UWeapon( *weapon_descriptor, weapon_descriptor );

    {
      // sets wrestling weapondesc as intrinsic
      WeaponDesc* wdesc = const_cast<WeaponDesc*>( weapon_descriptor );
      wdesc->is_intrinsic = true;
      wdesc->is_pc_intrinsic = true;
    }
    Core::gamestate.wrestling_weapon->inuse( true );
    Core::gamestate.wrestling_weapon->layer = Core::LAYER_HAND1;

    Items::insert_intrinsic_equipment( "PC_weapon", Core::gamestate.wrestling_weapon );
  }

  // wrestling_weapon = find_intrinsic_weapon( "Wrestling" );
  if ( Core::gamestate.wrestling_weapon == nullptr )
    throw std::runtime_error( "A WeaponTemplate for Wrestling is required in itemdesc.cfg" );
}

/// Creates a new intrinic weapon for an NPC template and returns it
/// @param elem: The conig element defining the NPC
/// @param pkg: The package
/// @returns The created weapon or nullptr if none is defined in the template
UWeapon* create_intrinsic_weapon_from_npctemplate( Clib::ConfigElem& elem,
                                                   const Plib::Package* pkg )
{
  std::string tmp;
  if ( elem.remove_prop( "AttackSpeed", &tmp ) )
  {
    // Construct a WeaponTemplate for this NPC template.
    Clib::ConfigElem wpnelem;
    wpnelem.set_rest( elem.rest() );
    wpnelem.set_source( elem );
    wpnelem.add_prop( "Objtype", "0xFFFF" );
    wpnelem.add_prop( "Graphic", "1" );
    wpnelem.add_prop( "Speed", tmp );

    if ( elem.remove_prop( "AttackDelay", &tmp ) )
      wpnelem.add_prop( "Delay", tmp );

    wpnelem.add_prop( "Damage", elem.remove_string( "AttackDamage" ) );

    if ( elem.has_prop( "AttackSkillId" ) )
      wpnelem.add_prop( "SkillId", elem.remove_string( "AttackSkillId" ) );
    else
      wpnelem.add_prop( "Attribute", elem.remove_string( "AttackAttribute" ) );

    wpnelem.add_prop( "HitSound", elem.remove_string( "AttackHitSound", "0" ) );
    wpnelem.add_prop( "MissSound", elem.remove_string( "AttackMissSound", "0" ) );
    wpnelem.add_prop( "Anim", elem.remove_string( "AttackAnimation", "0" ) );
    wpnelem.add_prop( "MaxHp", "1" );
    if ( elem.remove_prop( "AttackHitScript", &tmp ) )
      wpnelem.add_prop( "HitScript", tmp );
    if ( elem.remove_prop( "AttackMinRange", &tmp ) )
      wpnelem.add_prop( "MinRange", tmp );
    if ( elem.remove_prop( "AttackMaxRange", &tmp ) )
      wpnelem.add_prop( "MaxRange", tmp );

    if ( elem.has_prop( "AttackProjectile" ) )
    {
      wpnelem.add_prop( "Projectile", "1" );
      wpnelem.add_prop( "ProjectileType", elem.remove_string( "AttackProjectileType" ) );
      wpnelem.add_prop( "ProjectileAnim", elem.remove_string( "AttackProjectileAnim" ) );
      wpnelem.add_prop( "ProjectileSound", elem.remove_string( "AttackProjectileSound" ) );
    }

    while ( elem.remove_prop( "AttackCProp", &tmp ) )
      wpnelem.add_prop( "CProp", tmp );

    return create_intrinsic_weapon( elem.rest(), wpnelem, pkg );
  }
  else
  {
    return nullptr;
  }
}

UWeapon::UWeapon( const WeaponDesc& descriptor, const WeaponDesc* permanent_descriptor )
    : Equipment( descriptor, Core::UOBJ_CLASS::CLASS_WEAPON, permanent_descriptor ),
      hit_script_( descriptor.hit_script )
{
}

size_t UWeapon::estimatedSize() const
{
  return base::estimatedSize() + hit_script_.estimatedSize();
}

unsigned short UWeapon::speed() const
{
  int speed_ = WEAPON_TMPL->speed + speed_mod();

  if ( speed_ < 0 )
    return 0;
  else if ( speed_ <= USHRT_MAX )
    return static_cast<u16>( speed_ );
  else
    return USHRT_MAX;
}

unsigned short UWeapon::delay() const
{
  return WEAPON_TMPL->delay;
}

const Mobile::Attribute& UWeapon::attribute() const
{
  return *( WEAPON_TMPL->pAttr );
}

unsigned short UWeapon::get_random_damage() const
{
  int dmg = int( WEAPON_TMPL->get_random_damage() ) * hp_ / maxhp();
  dmg += damage_mod();
  if ( dmg < 0 )
    return 0;
  else if ( dmg <= USHRT_MAX )
    return static_cast<unsigned short>( dmg );
  else
    return USHRT_MAX;
}

unsigned short UWeapon::min_weapon_damage() const
{
  int dmg = static_cast<int>( WEAPON_TMPL->damage_dice.min_value() ) + damage_mod();
  if ( dmg < 0 )
    return 0;
  else if ( dmg <= USHRT_MAX )
    return static_cast<unsigned short>( dmg );
  return USHRT_MAX;
}

unsigned short UWeapon::max_weapon_damage() const
{
  int dmg = static_cast<int>( WEAPON_TMPL->damage_dice.max_value() ) + damage_mod();
  if ( dmg < 0 )
    return 0;
  else if ( dmg <= USHRT_MAX )
    return static_cast<unsigned short>( dmg );
  return USHRT_MAX;
}

bool UWeapon::is_projectile() const
{
  passert( tmpl != nullptr );
  return WEAPON_TMPL->projectile;
}

unsigned short UWeapon::projectile_sound() const
{
  passert( tmpl != nullptr );
  return WEAPON_TMPL->projectile_sound;
}

unsigned short UWeapon::projectile_anim() const
{
  passert( tmpl != nullptr );
  return WEAPON_TMPL->projectile_anim;
}

Core::UACTION UWeapon::anim() const
{
  passert( tmpl != nullptr );
  return WEAPON_TMPL->anim;
}

Core::UACTION UWeapon::mounted_anim() const
{
  passert( tmpl != nullptr );
  return WEAPON_TMPL->mounted_anim;
}

unsigned short UWeapon::hit_sound() const
{
  passert( tmpl != nullptr );
  return WEAPON_TMPL->hit_sound;
}

unsigned short UWeapon::miss_sound() const
{
  passert( tmpl != nullptr );
  return WEAPON_TMPL->miss_sound;
}

const WeaponDesc& UWeapon::descriptor() const
{
  passert( tmpl != nullptr );
  return *WEAPON_TMPL;
}

bool UWeapon::consume_projectile( Core::UContainer* cont ) const
{
  passert( tmpl != nullptr );

  Item* item = cont->find_objtype_noninuse( WEAPON_TMPL->projectile_type );
  if ( item != nullptr )
  {
    subtract_amount_from_item( item, 1 );
    return true;
  }
  else
  {
    return false;
  }
}

bool UWeapon::in_range( const Mobile::Character* wielder, const Mobile::Character* target ) const
{
  unsigned short dist = pol_distance( wielder, target );
  INFO_PRINT_TRACE( 22 ) << "in_range(0x" << fmt::hexu( wielder->serial ) << ",0x"
                         << fmt::hexu( target->serial ) << "):\n"
                         << "dist:   " << dist << "\n"
                         << "minrange: " << WEAPON_TMPL->minrange << "\n"
                         << "maxrange: " << WEAPON_TMPL->maxrange << "\n"
                         << "has_los:  " << wielder->realm->has_los( *wielder, *target ) << "\n";
  return ( dist >= WEAPON_TMPL->minrange && dist <= WEAPON_TMPL->maxrange &&
           wielder->realm->has_los( *wielder, *target ) );
}

// FIXME weak, weak..
Item* UWeapon::clone() const
{
  UWeapon* wpn = static_cast<UWeapon*>( base::clone() );
  wpn->hit_script_ = hit_script_;
  wpn->damage_mod( this->damage_mod() );
  wpn->speed_mod( this->speed_mod() );

  return wpn;
}

void UWeapon::printProperties( Clib::StreamWriter& sw ) const
{
  base::printProperties( sw );

  short speed_mod_ = speed_mod();
  short dmg_mod = damage_mod();

  if ( dmg_mod )
    sw() << "\tdmg_mod\t" << dmg_mod << pf_endl;
  if ( speed_mod_ )
    sw() << "tspeed_mod\t" << speed_mod_ << pf_endl;
  if ( !( hit_script_ == WEAPON_TMPL->hit_script ) )
    sw() << "\tHitScript\t" << hit_script_.relativename( tmpl->pkg ) << pf_endl;
}

void UWeapon::readProperties( Clib::ConfigElem& elem )
{
  base::readProperties( elem );

  damage_mod( static_cast<s16>( elem.remove_int( "DMG_MOD", 0 ) ) );
  speed_mod( static_cast<s16>( elem.remove_int( "SPEED_MOD", 0 ) ) );

  // if the HITSCRIPT is not specified in the data file, keep the value from the template.
  if ( elem.has_prop( "HITSCRIPT" ) )
    set_hit_script( elem.remove_string( "HITSCRIPT" ) );
}

void UWeapon::set_hit_script( const std::string& scriptname )
{
  if ( scriptname.empty() )
  {
    hit_script_.clear();
  }
  else
  {
    passert( tmpl != nullptr );
    hit_script_.config( scriptname, tmpl->pkg, "scripts/items/", true );
  }
}

bool UWeapon::get_method_hook( const char* methodname, Bscript::Executor* ex,
                               Core::ExportScript** hook, unsigned int* PC ) const
{
  if ( Core::gamestate.system_hooks.get_method_hook(
           Core::gamestate.system_hooks.weapon_method_script.get(), methodname, ex, hook, PC ) )
    return true;
  return base::get_method_hook( methodname, ex, hook, PC );
}
}  // namespace Items
}  // namespace Pol
