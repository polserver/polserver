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

      virtual double getQuality() const POL_OVERRIDE;
      virtual void setQuality(double value) POL_OVERRIDE;

	protected:
	  Equipment( const ItemDesc& itemdesc, UOBJ_CLASS uobj_class );
	  virtual void printProperties( Clib::StreamWriter& sw ) const POL_OVERRIDE;
	  virtual void readProperties( Clib::ConfigElem& elem ) POL_OVERRIDE;
	  virtual Bscript::BObjectImp* get_script_member( const char *membername ) const POL_OVERRIDE;
	  virtual Bscript::BObjectImp* get_script_member_id( const int id ) const POL_OVERRIDE; ///id test
	  //virtual Bscript::BObjectImp* set_script_member( const char *membername, const std::string& value );
	  virtual Bscript::BObjectImp* set_script_member( const char *membername, int value ) POL_OVERRIDE;
	  virtual Bscript::BObjectImp* set_script_member_double( const char *membername, double value ) POL_OVERRIDE;
	  virtual Bscript::BObjectImp* set_script_member_id( const int id, int value ) POL_OVERRIDE; //id test
	  virtual Bscript::BObjectImp* set_script_member_id_double( const int id, double value ) POL_OVERRIDE; //id test

	  virtual bool script_isa( unsigned isatype ) const POL_OVERRIDE;
	  virtual Item* clone() const POL_OVERRIDE;

      virtual size_t estimatedSize() const POL_OVERRIDE;

	protected:
	  const Core::EquipDesc& eq_tmpl_;
    private:
      double _quality;
	};
  }
}
#endif
