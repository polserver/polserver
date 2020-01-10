/** @file
 *
 * @par History
 * - 2006/10/28 Shinigami: GCC 4.1.1 fix - extra qualification in class removed
 * - 2009/11/20 Turley:    RecalcVitals can update single Attributes/Vitals - based on Tomi
 */


#ifndef VITALEMOD_H
#define VITALEMOD_H

#include <stddef.h>

#include "../../bscript/execmodl.h"
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
class VitalExecutorModule : public Bscript::TmplExecutorModule<VitalExecutorModule, Bscript::ExecutorModule>
{
public:
  VitalExecutorModule( Bscript::Executor& exec );

  Bscript::BObjectImp* mf_ApplyRawDamage();
  Bscript::BObjectImp* mf_ApplyDamage();

  Bscript::BObjectImp* mf_HealDamage();

  Bscript::BObjectImp* mf_ConsumeMana();

  Bscript::BObjectImp* mf_ConsumeVital( /* mob, vital, hundredths */ );
  Bscript::BObjectImp* mf_RecalcVitals( /* mob, attributes, vitals */ );

  Bscript::BObjectImp* mf_GetVitalName( /*alias_name*/ );

  Bscript::BObjectImp* mf_GetVital( /* mob, vitalid */ );
  Bscript::BObjectImp* mf_GetVitalMaximumValue( /* mob, vitalid */ );
  Bscript::BObjectImp* mf_GetVitalRegenRate( /* mob, vitalid */ );

  Bscript::BObjectImp* mf_SetVitalMaximumValue( /* mob, vitalid, value */ );
  Bscript::BObjectImp* mf_SetVital( /* mob, vitalid, value */ );
  Bscript::BObjectImp* mf_SetVitalRegenRate( /* mob, vitalid, rate */ );

private:
  Mobile::Character* GetUOController();
};

inline Mobile::Character* VitalExecutorModule::GetUOController()
{
  UOExecutorModule* uo_module = static_cast<UOExecutorModule*>( exec.findModule( "UO" ) );

  if ( uo_module != nullptr && uo_module->controller_.get() )
    return uo_module->controller_.get();
  else
    return nullptr;
}
}
}
#endif
