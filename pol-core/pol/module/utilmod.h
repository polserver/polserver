/** @file
 *
 * @par History
 * - 2008/07/08 Turley: Added mf_RandomIntMinMax - Return Random Value between...
 */


#ifndef __UTILMOD_H
#define __UTILMOD_H

#include "../../bscript/execmodl.h"

namespace Pol
{
namespace Bscript
{
class BObjectImp;
class Executor;
}  // namespace Bscript
}  // namespace Pol

namespace Pol
{
namespace Module
{
class UtilExecutorModule : public Bscript::TmplExecutorModule<UtilExecutorModule, Bscript::ExecutorModule>
{
public:
  UtilExecutorModule( Bscript::Executor& exec );

  Bscript::BObjectImp* mf_RandomInt();
  Bscript::BObjectImp* mf_RandomFloat();
  Bscript::BObjectImp* mf_RandomDiceRoll();
  Bscript::BObjectImp* mf_StrFormatTime();
  Bscript::BObjectImp* mf_RandomIntMinMax();
};
}
}
#endif  // UTILMOD_H
