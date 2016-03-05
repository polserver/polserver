/** @file
 *
 * @par History
 */


#ifndef DOOR_H
#define DOOR_H

#ifndef LOCKABLE_H
#include "lockable.h"
#endif

#ifndef ITEM_H
#include "item/item.h"
#endif

namespace Pol
{
namespace Network
{
class Client;
}
namespace Items
{
class DoorDesc;
}
namespace Core
{
class UDoor : public ULockable
{
  typedef ULockable base;
public:
  virtual ~UDoor() {};
  virtual size_t estimatedSize( ) const POL_OVERRIDE;
private:
  virtual void builtin_on_use( Network::Client* client ) POL_OVERRIDE;
  void toggle();
  void open();
  void close();
  bool is_open() const;
  virtual Bscript::BObjectImp* get_script_member( const char* membername ) const POL_OVERRIDE;
  virtual Bscript::BObjectImp* get_script_member_id( const int id ) const POL_OVERRIDE; ///id test
  virtual Bscript::BObjectImp* script_method( const char* methodname, Bscript::Executor& ex ) POL_OVERRIDE;
  virtual Bscript::BObjectImp* script_method_id( const int id, Bscript::Executor& ex ) POL_OVERRIDE;
  virtual bool script_isa( unsigned isatype ) const POL_OVERRIDE;

protected:
  UDoor( const Items::DoorDesc& descriptor );
  friend class Items::Item;
};
}
}
#endif
