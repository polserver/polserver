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
class ULockable : public Items::Item
{
  typedef Item base;

public:
  bool locked() const;

protected:
  void locked( bool newvalue );

  virtual void printProperties( Clib::StreamWriter& sw ) const override;
  virtual void readProperties( Clib::ConfigElem& elem ) override;
  virtual Bscript::BObjectImp* get_script_member( const char* membername ) const override;
  virtual Bscript::BObjectImp* get_script_member_id( const int id ) const override;  /// id test
  virtual Bscript::BObjectImp* set_script_member( const char* membername, int value ) override;
  virtual Bscript::BObjectImp* set_script_member_id( const int id,
                                                     int value ) override;  // id test
  virtual bool script_isa( unsigned isatype ) const override;

  virtual Items::Item* clone() const override;  // dave 12-20
  virtual ~ULockable(){};
  virtual size_t estimatedSize() const override;

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
}
}
#endif
