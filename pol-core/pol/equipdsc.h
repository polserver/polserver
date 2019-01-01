/** @file
 *
 * @par History
 */


#ifndef EQUIPDSC_H
#define EQUIPDSC_H

#include "../clib/rawtypes.h"
#include "item/itemdesc.h"

namespace Pol
{
namespace Bscript
{
class BStruct;
}  // namespace Bscript
}  // namespace Pol

namespace Pol
{
namespace Clib
{
class ConfigElem;
}
namespace Plib
{
class Package;
}
namespace Core
{
class EquipDesc : public Items::ItemDesc
{
public:
  typedef Items::ItemDesc base;
  EquipDesc( u32 objtype, Clib::ConfigElem& elem, Type type, const Plib::Package* pkg );
  EquipDesc();  // for dummy template
  virtual ~EquipDesc() = default;
  virtual void PopulateStruct( Bscript::BStruct* descriptor ) const override;
  virtual size_t estimatedSize() const override;

  bool is_intrinsic;
  bool is_pc_intrinsic;  // used to differentiate npc and pc intrinsics
};
}
}
#endif
