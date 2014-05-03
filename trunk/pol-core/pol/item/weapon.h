/*
History
=======
2005/03/09 Shinigami: Added Prop Delay [ms]

Notes
=======

*/

#ifndef WEAPON_H
#define WEAPON_H

#include <iosfwd>
#include <memory>
#include <string>

#include "../action.h"

#include "equipmnt.h"

#include "../scrdef.h"

namespace Pol {
  namespace Mobile {
	class Character;
	class Attribute;
  }
  namespace Items {
	class WeaponDesc;
	class UWeapon;


	class UWeapon : public Equipment
	{
	  typedef Equipment base;
	public:

	  unsigned short speed() const;
	  unsigned short delay() const;
	  unsigned short damage_mod() const;
	  const Mobile::Attribute& attribute() const;
	  unsigned short get_random_damage() const;
	  bool is_projectile() const;
	  bool is_intrinsic() const;
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

	  virtual Item* clone() const;

	protected:
	  virtual void printProperties( Clib::StreamWriter& sw ) const;
	  virtual void readProperties( Clib::ConfigElem& elem );
	  //BObjectImp* script_member( const char *membername );
	  virtual Bscript::BObjectImp* get_script_member( const char *membername ) const;
	  virtual Bscript::BObjectImp* get_script_member_id( const int id ) const; ///id test
	  virtual Bscript::BObjectImp* set_script_member( const char *membername, const std::string& value );
	  virtual Bscript::BObjectImp* set_script_member( const char *membername, int value );
	  virtual Bscript::BObjectImp* set_script_member_double( const char *membername, double value );
	  virtual Bscript::BObjectImp* set_script_member_id( const int id, const std::string& value ); //id test
	  virtual Bscript::BObjectImp* set_script_member_id( const int id, int value ); //id test
	  virtual Bscript::BObjectImp* set_script_member_id_double( const int id, double value ); //id test
	  virtual bool script_isa( unsigned isatype ) const;
	  UWeapon( const WeaponDesc& descriptor, const WeaponDesc* permanent_descriptor );
	  ~UWeapon();
	  friend class Item;
	  friend void load_data();
	  friend UWeapon* create_intrinsic_weapon( const char* name, Clib::ConfigElem& elem, const Plib::Package* pkg );
	  friend void load_weapon_templates();
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


	  const WeaponDesc* tmpl;

	  Core::ScriptDef hit_script_;
	};

	inline const Core::ScriptDef& UWeapon::hit_script() const
	{
	  return hit_script_;
	}


	void load_weapon_templates();
	void unload_weapon_templates();
	bool isa_weapon( u32 objtype );

	UWeapon* find_intrinsic_weapon( const std::string& name );
	UWeapon* create_intrinsic_weapon( Clib::ConfigElem& elem );
	UWeapon* create_intrinsic_weapon_from_npctemplate( Clib::ConfigElem& elem, const Plib::Package* pkg );
  }
}
#endif
