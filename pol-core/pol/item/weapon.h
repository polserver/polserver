/** @file
 *
 * @par History
 * - 2005/03/09 Shinigami: Added Prop Delay [ms]
 */


#ifndef WEAPON_H
#define WEAPON_H


#include "equipmnt.h"

#include "../action.h"
#include "../scrdef.h"

#include <iosfwd>
#include <memory>
#include <string>

#define WEAPON_TMPL (static_cast<const WeaponDesc*>(tmpl))

namespace Pol
{
namespace Mobile
{
class Character;
class Attribute;
}
namespace Items
{
class WeaponDesc;
class UWeapon;


class UWeapon : public Equipment
{
  typedef Equipment base;
public:
  virtual ~UWeapon() {};

  unsigned short speed() const;
  unsigned short delay() const;

  const Mobile::Attribute& attribute() const;
  unsigned short get_random_damage() const;
  bool is_projectile() const;
  bool consume_projectile( Core::UContainer* cont ) const;
  bool in_range( const Mobile::Character* wielder,
                 const Mobile::Character* target ) const;
  unsigned short projectile_sound() const;
  unsigned short projectile_anim() const;
  // UACTION projectile_action() const;
  Core::UACTION anim() const;
  Core::UACTION mounted_anim( ) const;
  unsigned short hit_sound() const;
  unsigned short miss_sound() const;

  const WeaponDesc& descriptor() const;
  const Core::ScriptDef& hit_script( ) const;
  void set_hit_script( const std::string& scriptname );

  virtual Item* clone() const POL_OVERRIDE;
  virtual size_t estimatedSize() const POL_OVERRIDE;

  DYN_PROPERTY(damage_mod, s16, Core::PROP_DMG_MOD, 0);
  DYN_PROPERTY(speed_mod,  s16, Core::PROP_SPEED_MOD, 0);

protected:
  virtual void printProperties( Clib::StreamWriter& sw ) const POL_OVERRIDE;
  virtual void readProperties( Clib::ConfigElem& elem ) POL_OVERRIDE;
  //BObjectImp* script_member( const char *membername );
  virtual Bscript::BObjectImp* get_script_member( const char* membername ) const POL_OVERRIDE;
  virtual Bscript::BObjectImp* get_script_member_id( const int id ) const POL_OVERRIDE; ///id test

  virtual Bscript::BObjectImp* set_script_member(const char* membername, const std::string& value) POL_OVERRIDE;
  virtual Bscript::BObjectImp* set_script_member( const char* membername, int value ) POL_OVERRIDE;
  virtual Bscript::BObjectImp* set_script_member_double( const char* membername, double value ) POL_OVERRIDE;
  virtual Bscript::BObjectImp* set_script_member_id( const int id, const std::string& value ) POL_OVERRIDE; //id test
  virtual Bscript::BObjectImp* set_script_member_id( const int id, int value ) POL_OVERRIDE; //id test
  virtual Bscript::BObjectImp* set_script_member_id_double( const int id, double value ) POL_OVERRIDE; //id test
  virtual bool script_isa( unsigned isatype ) const POL_OVERRIDE;
  UWeapon( const WeaponDesc& descriptor, const WeaponDesc* permanent_descriptor );
  friend class Item;
  friend void load_data();
  friend UWeapon* create_intrinsic_weapon( const char* name, Clib::ConfigElem& elem, const Plib::Package* pkg );
  friend void load_intrinsic_weapons();

private:

  /*
    max repair % in template
    quality(0-200%), repair( 0-125%)
    effective repair is min(repair,100%)
    quality drops after each repair attempt
    effects on damage/speed: damage * f( quality, effective repair)
    f ranges from .5 to 1.5
    effects on speed: speed * g(quality, effective_repair)
    g ranges from .75 to 1.25
    */

  Core::ScriptDef hit_script_;
};

inline const Core::ScriptDef& UWeapon::hit_script() const
{
  return hit_script_;
}

bool isa_weapon( u32 objtype );

UWeapon* create_intrinsic_weapon( Clib::ConfigElem& elem );
UWeapon* create_intrinsic_weapon_from_npctemplate( Clib::ConfigElem& elem, const Plib::Package* pkg );
}
}
#endif
