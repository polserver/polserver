/** @file
 *
 * @par History
 * - 2006/10/28 Shinigami: GCC 4.1.1 fix - extra qualification in class removed
 * - 2009/11/20 Turley:    RecalcVitals can update single Attributes/Vitals - based on Tomi
 */


#ifndef VITALEMOD_H
#define VITALEMOD_H

#include <stddef.h>

#include "../polmodl.h"
#include "uomod.h"

namespace Pol
{
namespace Bscript
{
class BObjectImp;
class Executor;
}  // namespace Bscript
namespace Module
{
class UOExecutorModule;
}  // namespace Module
}  // namespace Pol

namespace Pol
{
namespace Mobile
{
class Character;
}
namespace Module
{
class VitalExecutorModule : public Bscript::TmplExecutorModule<VitalExecutorModule, Core::PolModule>
{
public:
  VitalExecutorModule( Bscript::Executor& exec );

  [[nodiscard]] Bscript::BObjectImp* mf_ApplyRawDamage();
  [[nodiscard]] Bscript::BObjectImp* mf_ApplyDamage();

  [[nodiscard]] Bscript::BObjectImp* mf_HealDamage();

  [[nodiscard]] Bscript::BObjectImp* mf_ConsumeMana();

  [[nodiscard]] Bscript::BObjectImp* mf_ConsumeVital( /* mob, vital, hundredths */ );
  [[nodiscard]] Bscript::BObjectImp* mf_RecalcVitals( /* mob, attributes, vitals */ );

  [[nodiscard]] Bscript::BObjectImp* mf_GetVitalName( /*alias_name*/ );

  [[nodiscard]] Bscript::BObjectImp* mf_GetVital( /* mob, vitalid */ );
  [[nodiscard]] Bscript::BObjectImp* mf_GetVitalMaximumValue( /* mob, vitalid */ );
  [[nodiscard]] Bscript::BObjectImp* mf_GetVitalRegenRate( /* mob, vitalid */ );

  [[nodiscard]] Bscript::BObjectImp* mf_SetVital( /* mob, vitalid, value */ );

private:
  Mobile::Character* GetUOController();
};

inline Mobile::Character* VitalExecutorModule::GetUOController()
{
  UOExecutorModule* uo_module = static_cast<UOExecutorModule*>( exec.findModule( "UO" ) );

  if ( uo_module != nullptr && uo_module->controller_.get() )
    return uo_module->controller_.get();
  return nullptr;
}
}  // namespace Module
}  // namespace Pol
#endif
