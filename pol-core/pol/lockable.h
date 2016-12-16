/** @file
 *
 * @par History
 */


#ifndef LOCKABLE_H
#define LOCKABLE_H

#ifndef ITEM_H
#include "item/item.h"
#endif

namespace Pol
{
namespace Items
{
class Itemdesc;
}
namespace Core
{
class ULockable : public Items::Item
{
  typedef Item base;

public:
  bool locked() const;

protected:
  void locked( bool newvalue );

  virtual void printProperties( Clib::StreamWriter& sw ) const POL_OVERRIDE;
  virtual void readProperties( Clib::ConfigElem& elem ) POL_OVERRIDE;
  virtual Bscript::BObjectImp* get_script_member( const char* membername ) const POL_OVERRIDE;
  virtual Bscript::BObjectImp* get_script_member_id( const int id ) const POL_OVERRIDE;  /// id test
  virtual Bscript::BObjectImp* set_script_member( const char* membername, int value ) POL_OVERRIDE;
  virtual Bscript::BObjectImp* set_script_member_id( const int id,
                                                     int value ) POL_OVERRIDE;  // id test
  virtual bool script_isa( unsigned isatype ) const POL_OVERRIDE;

  virtual Items::Item* clone() const POL_OVERRIDE;  // dave 12-20
  virtual ~ULockable(){};
  virtual size_t estimatedSize() const POL_OVERRIDE;

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
