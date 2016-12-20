/** @file
 *
 * @par History
 */


#ifndef BSCRIPT_BASICIOMOD_H
#define BSCRIPT_BASICIOMOD_H

#include "../../bscript/execmodl.h"

namespace Pol
{
namespace Module
{
class BasicIoExecutorModule : public Bscript::TmplExecutorModule<BasicIoExecutorModule>
{
public:
  Bscript::BObjectImp* print();

  BasicIoExecutorModule( Bscript::Executor& exec );
};
}
}
#endif
