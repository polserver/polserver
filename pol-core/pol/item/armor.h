/*
History
=======


Notes
=======

*/

#ifndef ARMOR_H
#define ARMOR_H

#include <iosfwd>

#include "../scrdef.h"

#include "equipmnt.h"

#include <set>
#include <string>

#define ARMOR_TMPL (static_cast<const ArmorDesc*>(tmpl))

namespace Pol {
  namespace Items {
	class ArmorDesc;
    class Item;

	class UArmor : public Equipment
	{
	  typedef Equipment base;

	public:
      virtual ~UArmor() {};
	  virtual unsigned short ar() const;
	  virtual unsigned short ar_base() const;
	  virtual bool covers( unsigned short layer ) const;
	  virtual Item* clone() const POL_OVERRIDE;
      virtual size_t estimatedSize( ) const POL_OVERRIDE;

	  void set_onhitscript( const std::string& scriptname );
	  std::set<unsigned short> tmplzones();

      DYN_PROPERTY(ar_mod, s16, Core::PROP_AR_MOD, 0);

	protected:
	  virtual void printProperties( Clib::StreamWriter& sw ) const POL_OVERRIDE;
	  virtual void readProperties( Clib::ConfigElem& elem ) POL_OVERRIDE;
	  virtual Bscript::BObjectImp* get_script_member( const char *membername ) const POL_OVERRIDE;
	  virtual Bscript::BObjectImp* get_script_member_id( const int id ) const POL_OVERRIDE; ///id test

      virtual Bscript::BObjectImp* set_script_member(const char *membername, const std::string& value) POL_OVERRIDE;
	  virtual Bscript::BObjectImp* set_script_member( const char *membername, int value ) POL_OVERRIDE;
	  virtual Bscript::BObjectImp* set_script_member_id( const int id, const std::string& value ) POL_OVERRIDE; //id test
	  virtual Bscript::BObjectImp* set_script_member_id( const int id, int value ) POL_OVERRIDE;//id test
	  virtual bool script_isa( unsigned isatype ) const POL_OVERRIDE;

	  UArmor( const ArmorDesc& descriptor, const ArmorDesc* permanent_descriptor );
	  friend class Item;
	  //friend void load_data();
	  friend void load_weapon_templates();
	  friend UArmor* create_intrinsic_shield( const char* name, Clib::ConfigElem& elem, const Plib::Package* pkg );

	  const ArmorDesc& descriptor() const;
	  const Core::ScriptDef& onhitscript() const;

	private:
	  Core::ScriptDef onhitscript_;

	};

	void load_armor_templates();
	void unload_armor_templates();

	void validate_intrinsic_shield_template();
	UArmor* create_intrinsic_shield_from_npctemplate( Clib::ConfigElem& elem, const Plib::Package* pkg );
  }
}
#endif
