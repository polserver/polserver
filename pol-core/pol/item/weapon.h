/** @file
 *
 * @par History
 * - 2005/03/09 Shinigami: Added Prop Delay [ms]
 */


#ifndef WEAPON_H
#define WEAPON_H


#include <iosfwd>
#include <memory>
#include <string>

#include "../../bscript/bobject.h"
#include "../../clib/rawtypes.h"
#include "../action.h"
#include "../dynproperties.h"
#include "../scrdef.h"
#include "equipmnt.h"

namespace Pol
{
namespace Bscript
{
class Executor;
}  // namespace Bscript
namespace Clib
{
class ConfigElem;
class StreamWriter;
}  // namespace Clib
namespace Core
{
class UContainer;
class ExportScript;
}  // namespace Core
namespace Items
{
class Item;
}  // namespace Items
namespace Plib
{
class Package;
}  // namespace Plib
}  // namespace Pol

#define WEAPON_TMPL ( static_cast<const WeaponDesc*>( tmpl ) )

namespace Pol
{
namespace Mobile
{
class Attribute;
class Character;
}  // namespace Mobile
namespace Items
{
class UWeapon;
class WeaponDesc;


class UWeapon final : public Equipment
{
  using base = Equipment;

public:
  ~UWeapon() override = default;

  unsigned short speed() const;
  unsigned short delay() const;

  const Mobile::Attribute& attribute() const;
  unsigned short get_random_damage() const;
  unsigned short min_weapon_damage() const;
  unsigned short max_weapon_damage() const;
  bool is_projectile() const;
  bool consume_projectile( Core::UContainer* cont ) const;
  bool in_range( const Mobile::Character* wielder, const Mobile::Character* target ) const;
  unsigned short projectile_sound() const;
  unsigned short projectile_anim() const;
  Core::UACTION anim() const;
  Core::UACTION mounted_anim() const;
  unsigned short hit_sound() const;
  unsigned short miss_sound() const;

  const WeaponDesc& descriptor() const;
  const Core::ScriptDef& hit_script() const;
  void set_hit_script( const std::string& scriptname );

  Item* clone() const override;
  size_t estimatedSize() const override;

  DYN_PROPERTY( damage_mod, s16, Core::PROP_DMG_MOD, 0 );
  DYN_PROPERTY( speed_mod, s16, Core::PROP_SPEED_MOD, 0 );

  bool get_method_hook( const char* methodname, Bscript::Executor* ex, Core::ExportScript** hook,
                        unsigned int* PC ) const override;

protected:
  void printProperties( Clib::StreamWriter& sw ) const override;
  void readProperties( Clib::ConfigElem& elem ) override;
  // BObjectImp* script_member( const char *membername );
  Bscript::BObjectImp* get_script_member( const char* membername ) const override;
  Bscript::BObjectImp* get_script_member_id( const int id ) const override;  /// id test

  Bscript::BObjectImp* set_script_member( const char* membername,
                                          const std::string& value ) override;
  Bscript::BObjectImp* set_script_member( const char* membername, int value ) override;
  Bscript::BObjectImp* set_script_member_double( const char* membername, double value ) override;
  Bscript::BObjectImp* set_script_member_id( const int id,
                                             const std::string& value ) override;  // id test
  Bscript::BObjectImp* set_script_member_id( const int id,
                                             int value ) override;  // id test
  Bscript::BObjectImp* set_script_member_id_double( const int id,
                                                    double value ) override;  // id test
  bool script_isa( unsigned isatype ) const override;
  UWeapon( const WeaponDesc& descriptor, const WeaponDesc* permanent_descriptor );
  friend class Item;
  friend void load_data();
  friend UWeapon* create_intrinsic_weapon( const char* name, Clib::ConfigElem& elem,
                                           const Plib::Package* pkg );
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
UWeapon* create_intrinsic_weapon_from_npctemplate( Clib::ConfigElem& elem,
                                                   const Plib::Package* pkg );
}  // namespace Items
}  // namespace Pol
#endif
