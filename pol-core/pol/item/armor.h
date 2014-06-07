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
	  virtual Item* clone() const;
      virtual size_t estimatedSize( ) const;

	  void set_onhitscript( const std::string& scriptname );
	  std::set<unsigned short> tmplzones();

	protected:

	  virtual void printProperties( Clib::StreamWriter& sw ) const;
	  virtual void readProperties( Clib::ConfigElem& elem );
	  virtual Bscript::BObjectImp* get_script_member( const char *membername ) const;
	  virtual Bscript::BObjectImp* get_script_member_id( const int id ) const; ///id test
	  virtual Bscript::BObjectImp* set_script_member( const char *membername, const std::string& value );
	  virtual Bscript::BObjectImp* set_script_member( const char *membername, int value );
	  virtual Bscript::BObjectImp* set_script_member_id( const int id, const std::string& value ); //id test
	  virtual Bscript::BObjectImp* set_script_member_id( const int id, int value );//id test
	  virtual bool script_isa( unsigned isatype ) const;

	  UArmor( const ArmorDesc& descriptor, const ArmorDesc* permanent_descriptor );
	  friend class Item;
	  //friend void load_data();

	  const ArmorDesc& descriptor() const;
	  const Core::ScriptDef& onhitscript() const;


	private:
	  const ArmorDesc* tmpl;

	  Core::ScriptDef onhitscript_;
	};

	void load_armor_templates();
	void unload_armor_templates();
  }
}
#endif
