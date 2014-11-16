/*
History
=======


Notes
=======

*/

#ifndef DOOR_H
#define DOOR_H

#ifndef LOCKABLE_H
#include "lockable.h"
#endif

#ifndef ITEM_H
#include "item/item.h"
#endif

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
    public:
      virtual ~UDoor() {};
      virtual size_t estimatedSize( ) const;
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
