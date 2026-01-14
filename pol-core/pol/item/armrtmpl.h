/** @file
 *
 * @par History
 */


#ifndef ARMRTMPL_H
#define ARMRTMPL_H

#include <set>
#include <string>

#include "../equipdsc.h"
namespace Pol
{
namespace Clib
{
class ConfigElem;
}
namespace Items
{
class ArmorDesc : public Core::EquipDesc
{
public:
  // ArmorTemplate();
  typedef EquipDesc base;
  ArmorDesc( u32 objtype, Clib::ConfigElem& elem, const Plib::Package* pkg,
             bool forceShield = false );
  ~ArmorDesc() override{};
  void PopulateStruct( Bscript::BStruct* descriptor ) const override;
  size_t estimatedSize() const override;

  unsigned short ar;
  std::set<unsigned short> zones;
  Core::ScriptDef on_hit_script;
};
}  // namespace Items
}  // namespace Pol
#endif
