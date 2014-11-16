/*
History
=======


Notes
=======

*/

#ifndef EQUIPMNT_H
#define EQUIPMNT_H

#include <iosfwd>

#ifndef ITEM_H
#include "item.h"
#endif

namespace Pol {
  namespace Bscript {
	class BObjectImp;
  }
  namespace Clib {
	class ConfigElem;
  }
  namespace Core {
    class EquipDesc;
  }
  namespace Items {

	class Equipment : public Item
	{
	  typedef Item base;

	public:
      virtual ~Equipment() {};
	  void reduce_hp_from_hit();

	protected:
	  Equipment( const ItemDesc& itemdesc, UOBJ_CLASS uobj_class );
	  virtual void printProperties( Clib::StreamWriter& sw ) const;
	  virtual void readProperties( Clib::ConfigElem& elem );
	  virtual Bscript::BObjectImp* get_script_member( const char *membername ) const;
	  virtual Bscript::BObjectImp* get_script_member_id( const int id ) const; ///id test
	  //virtual Bscript::BObjectImp* set_script_member( const char *membername, const std::string& value );
	  virtual Bscript::BObjectImp* set_script_member( const char *membername, int value );
	  virtual Bscript::BObjectImp* set_script_member_double( const char *membername, double value );
	  virtual Bscript::BObjectImp* set_script_member_id( const int id, int value ); //id test
	  virtual Bscript::BObjectImp* set_script_member_id_double( const int id, double value ); //id test

	  virtual bool script_isa( unsigned isatype ) const;
	  virtual Item* clone() const;

      virtual size_t estimatedSize() const;

	protected:
	  const Core::EquipDesc& eq_tmpl_;
	};
  }
}
#endif
