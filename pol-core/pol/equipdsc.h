/** @file
 *
 * @par History
 */


#ifndef EQUIPDSC_H
#define EQUIPDSC_H

#include "../clib/rawtypes.h"
#include "item/itemdesc.h"


namespace Pol::Bscript
{
class BStruct;
}  // namespace Pol::Bscript


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
  using base = Items::ItemDesc;
  EquipDesc( u32 objtype, Clib::ConfigElem& elem, Type type, const Plib::Package* pkg );
  EquipDesc();  // for dummy template
  ~EquipDesc() override = default;
  void PopulateStruct( Bscript::BStruct* descriptor ) const override;
  size_t estimatedSize() const override;

  bool is_intrinsic;
  bool is_pc_intrinsic;  // used to differentiate npc and pc intrinsics
};
}  // namespace Core
}  // namespace Pol
#endif
