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
};
}  // namespace Module
}  // namespace Pol
#endif  // UTILMOD_H
