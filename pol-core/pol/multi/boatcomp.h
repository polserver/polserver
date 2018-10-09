/** @file
 *
 * @par History
 * - 2009/09/03 MuadDib:     Changes for account related source file relocation
 *                           Changes for multi related source file relocation
 */


#ifndef BOATCOMP_H
#define BOATCOMP_H

#include "../../bscript/bobject.h"
#include "../../clib/rawtypes.h"
#include "../../clib/refptr.h"
#include "../item/item.h"
#include "../lockable.h"
#include "boat.h"

namespace Pol
{
namespace Items
{
class ItemDesc;
}  // namespace Items
namespace Multi
{
class UBoat;
}  // namespace Multi
}  // namespace Pol

namespace Pol
{
namespace Bscript
{
class BObjectImp;
}
namespace Multi
{
class UPlank final : public Core::ULockable
{
  typedef Core::ULockable base;

public:
  void setboat( UBoat* boat );
  virtual ~UPlank(){};
  virtual size_t estimatedSize() const override;

protected:
  explicit UPlank( const Items::ItemDesc& descriptor );
  friend Items::Item* Items::Item::create( const Items::ItemDesc& descriptor, u32 serial );

  virtual void destroy() override;
  virtual Bscript::BObjectImp* get_script_member( const char* membername ) const override;
  virtual Bscript::BObjectImp* get_script_member_id( const int id ) const override;  /// id test

private:
  ref_ptr<UBoat> boat_;
};
}
}
#endif
