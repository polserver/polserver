/*
History
=======


Notes
=======

*/

#ifndef __DOOR_H
#define __DOOR_H

#ifndef __ITEM_H
#	include "lockable.h"
#endif
#include "item/item.h"

namespace Pol {
  namespace Network {
	class Client;
  }
  namespace Items {
	class DoorDesc;
  }
  namespace Core {
	class UDoor : public ULockable
	{
	  typedef ULockable base;
	private:
	  virtual void builtin_on_use( Network::Client *client );
	  void toggle();
	  void open();
	  void close();
	  bool is_open() const;
	  virtual Bscript::BObjectImp* get_script_member( const char *membername ) const;
	  virtual Bscript::BObjectImp* get_script_member_id( const int id ) const; ///id test
	  virtual Bscript::BObjectImp* script_method( const char* methodname, Bscript::Executor& ex );
	  virtual Bscript::BObjectImp* script_method_id( const int id, Bscript::Executor& ex );
	  virtual bool script_isa( unsigned isatype ) const;

	protected:
	  UDoor( const Items::DoorDesc& descriptor );
	  friend class Items::Item;
	};
  }
}
#endif
