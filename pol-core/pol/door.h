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
namespace Pol
{
namespace Bscript
{
class BObjectImp;
class Executor;
}  // namespace Bscript
}  // namespace Pol
}
namespace Items
{
class DoorDesc;
}
namespace Core
{
class ExportScript;
class UOExecutor;
class UDoor final : public ULockable
{
  typedef ULockable base;

public:
  virtual ~UDoor() = default;
  virtual size_t estimatedSize() const override;
  virtual bool get_method_hook( const char* methodname, Bscript::Executor* ex, ExportScript** hook,
                                unsigned int* PC ) const override;

private:
  virtual void builtin_on_use( Network::Client* client ) override;
  void toggle();
  void open();
  void close();
  bool is_open() const;
  virtual Bscript::BObjectImp* get_script_member( const char* membername ) const override;
  virtual Bscript::BObjectImp* get_script_member_id( const int id ) const override;  /// id test
  virtual Bscript::BObjectImp* script_method( const char* methodname,
                                              UOExecutor& ex ) override;
  virtual Bscript::BObjectImp* script_method_id( const int id, UOExecutor& ex ) override;
  virtual bool script_isa( unsigned isatype ) const override;

protected:
  UDoor( const Items::DoorDesc& descriptor );
  friend class Items::Item;
};
}
}
#endif
