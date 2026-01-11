/** @file
 *
 * @par History
 */


#ifndef LOCKABLE_H
#define LOCKABLE_H

#include "baseobject.h"

namespace Pol
{
namespace Bscript
{
class BObjectImp;
class Executor;
}  // namespace Bscript
namespace Clib
{
class ConfigElem;
class StreamWriter;
}  // namespace Clib
namespace Items
{
class ItemDesc;
}  // namespace Items
}  // namespace Pol
#ifndef ITEM_H
#include "item/item.h"
#endif

namespace Pol
{
namespace Core
{
class ExportScript;
class ULockable : public Items::Item
{
  typedef Item base;

public:
  bool locked() const;

protected:
  void locked( bool newvalue );

  void printProperties( Clib::StreamWriter& sw ) const override;
  void readProperties( Clib::ConfigElem& elem ) override;
  Bscript::BObjectImp* get_script_member( const char* membername ) const override;
  Bscript::BObjectImp* get_script_member_id( const int id ) const override;  /// id test
  Bscript::BObjectImp* set_script_member( const char* membername, int value ) override;
  Bscript::BObjectImp* set_script_member_id( const int id,
                                             int value ) override;  // id test
  bool get_method_hook( const char* methodname, Bscript::Executor* ex, Core::ExportScript** hook,
                        unsigned int* PC ) const override;
  bool script_isa( unsigned isatype ) const override;

  Items::Item* clone() const override;  // dave 12-20
  ~ULockable() override{};
  size_t estimatedSize() const override;

protected:
  explicit ULockable( const Items::ItemDesc& itemdesc, UOBJ_CLASS uobj_class );
  friend class Items::Item;
};

inline bool ULockable::locked() const
{
  return flags_.get( OBJ_FLAGS::LOCKED );
}
inline void ULockable::locked( bool newvalue )
{
  flags_.change( OBJ_FLAGS::LOCKED, newvalue );
}
}  // namespace Core
}  // namespace Pol
#endif
