/** @file
 *
 * @par History
 * - 2009/09/03 MuadDib:     Changes for account related source file relocation
 *                           Changes for multi related source file relocation
 */


#ifndef BOATCOMP_H
#define BOATCOMP_H

#include "boat.h"
#include "../lockable.h"

namespace Pol
{
namespace Bscript
{
class BObjectImp;
}
namespace Multi
{
class UPlank : public Core::ULockable
{
  typedef Core::ULockable base;

public:
  void setboat( UBoat* boat );
  virtual ~UPlank(){};
  virtual size_t estimatedSize() const POL_OVERRIDE;

protected:
  explicit UPlank( const Items::ItemDesc& descriptor );
  friend Items::Item* Items::Item::create( const Items::ItemDesc& descriptor, u32 serial );

  virtual void destroy() POL_OVERRIDE;
  virtual Bscript::BObjectImp* get_script_member( const char* membername ) const POL_OVERRIDE;
  virtual Bscript::BObjectImp* get_script_member_id( const int id ) const POL_OVERRIDE;  /// id test

private:
  ref_ptr<UBoat> boat_;
};
}
}
#endif
