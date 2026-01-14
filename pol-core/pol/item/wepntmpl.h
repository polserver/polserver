/** @file
 *
 * @par History
 * - 2005/03/09 Shinigami: Added Prop Delay [ms]
 */


#ifndef WEPNTMPL_H
#define WEPNTMPL_H

#include <string>

#include "../action.h"
#include "../dice.h"
#include "../equipdsc.h"

namespace Pol
{
namespace Mobile
{
class Attribute;
}
namespace Items
{
class WeaponDesc : public Core::EquipDesc
{
public:
  typedef EquipDesc base;
  WeaponDesc( u32 objtype, Clib::ConfigElem& elem, const Plib::Package* pkg );
  WeaponDesc();  // for dummy template
  ~WeaponDesc() override{};
  void PopulateStruct( Bscript::BStruct* descriptor ) const override;
  size_t estimatedSize() const override;

  unsigned short get_random_damage() const;

  const Mobile::Attribute* pAttr;

  unsigned short speed;
  unsigned short delay;

  Core::ScriptDef hit_script;

  Core::Dice damage_dice;

  bool projectile;
  unsigned short projectile_type;
  unsigned short projectile_anim;
  unsigned short projectile_sound;
  Core::UACTION anim;
  Core::UACTION mounted_anim;
  unsigned short hit_sound;
  unsigned short miss_sound;

  bool two_handed;

  unsigned short minrange;
  unsigned short maxrange;
};
}  // namespace Items
}  // namespace Pol
#endif
