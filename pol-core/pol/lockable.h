/*
History
=======


Notes
=======

*/

#ifndef LOCKABLE_H
#define LOCKABLE_H

#include "item/item.h"

namespace Pol {
  namespace Items {
    class Itemdesc;
  }
  namespace Core {
	class ULockable : public Items::Item
	{
	  typedef Item base;
	public:
	  bool locked() const;
	protected:
	  bool locked_;

	  virtual void printProperties( Clib::StreamWriter& sw ) const;
	  virtual void readProperties( Clib::ConfigElem& elem );
	  virtual Bscript::BObjectImp* get_script_member( const char* membername ) const;
	  virtual Bscript::BObjectImp* get_script_member_id( const int id ) const; ///id test
	  virtual Bscript::BObjectImp* set_script_member( const char* membername, int value );
	  virtual Bscript::BObjectImp* set_script_member_id( const int id, int value ); //id test
	  virtual bool script_isa( unsigned isatype ) const;

	  virtual Items::Item* clone() const; //dave 12-20
	protected:
	  explicit ULockable( const Items::ItemDesc& itemdesc, UObject::UOBJ_CLASS uobj_class );
	  friend class Items::Item;
	};

	inline bool ULockable::locked() const
	{
	  return locked_;
	}
  }
}
#endif
