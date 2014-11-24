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
	  virtual bool covers( unsigned short layer ) const;
	  virtual Item* clone() const POL_OVERRIDE;
      virtual size_t estimatedSize( ) const POL_OVERRIDE;

	  void set_onhitscript( const std::string& scriptname );
	  std::set<unsigned short> tmplzones();
      
      s16 ar_mod() const;
      void ar_mod(s16 newvalue);

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

	  const ArmorDesc& descriptor() const;
	  const Core::ScriptDef& onhitscript() const;


	private:
	  const ArmorDesc* tmpl;

	  Core::ScriptDef onhitscript_;
	};

    inline s16 UArmor::ar_mod() const
    {
        return getmember<s16>(Bscript::MBR_AR_MOD);
    }
    inline void UArmor::ar_mod(s16 newvalue)
    {
        setmember<s16>(Bscript::MBR_AR_MOD, newvalue);
    }


	void load_armor_templates();
	void unload_armor_templates();
  }
}
#endif
