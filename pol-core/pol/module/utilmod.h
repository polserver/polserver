/** @file
 *
 * @par History
 * - 2008/07/08 Turley: Added mf_RandomIntMinMax - Return Random Value between...
 */


#pragma once

#include "../../bscript/execmodl.h"


namespace Pol::Bscript
{
class BObjectImp;
class Executor;
}  // namespace Pol::Bscript


namespace Pol::Module
{
class UtilExecutorModule
    : public Bscript::TmplExecutorModule<UtilExecutorModule, Bscript::ExecutorModule>
{
public:
  UtilExecutorModule( Bscript::Executor& exec );

  [[nodiscard]] Bscript::BObjectImp* mf_RandomInt();
  [[nodiscard]] Bscript::BObjectImp* mf_RandomFloat();
  [[nodiscard]] Bscript::BObjectImp* mf_RandomDiceRoll();
  [[nodiscard]] Bscript::BObjectImp* mf_StrFormatTime();
  [[nodiscard]] Bscript::BObjectImp* mf_RandomIntMinMax();
  [[nodiscard]] Bscript::BObjectImp* mf_RandomUUID();
};
}  // namespace Pol::Module
