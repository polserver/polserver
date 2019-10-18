/** @file
 *
 * @par History
 */


#include "basiciomod.h"
#include "../../clib/logfacility.h"


namespace Pol
{
namespace Module
{
BasicIoExecutorModule::BasicIoExecutorModule( Bscript::Executor& exec )
    : Bscript::TmplExecutorModule<BasicIoExecutorModule>( exec )
{
}

Bscript::BObjectImp* BasicIoExecutorModule::mf_Print()
{
  INFO_PRINT << exec.getParamImp( 0 )->getStringRep() << "\n";
  return new Bscript::UninitObject;
}
}
}
