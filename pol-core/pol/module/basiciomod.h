/** @file
 *
 * @par History
 */


#ifndef BSCRIPT_BASICIOMOD_H
#define BSCRIPT_BASICIOMOD_H

#include "../../bscript/execmodl.h"


namespace Pol::Bscript
{
class BObjectImp;
class Executor;
}  // namespace Pol::Bscript


namespace Pol::Module
{
class BasicIoExecutorModule
    : public Bscript::TmplExecutorModule<BasicIoExecutorModule, Bscript::ExecutorModule>
{
public:
  [[nodiscard]] Bscript::BObjectImp* mf_Print();

  BasicIoExecutorModule( Bscript::Executor& exec );
};
}  // namespace Pol::Module

#endif
