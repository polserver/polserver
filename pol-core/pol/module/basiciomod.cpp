/** @file
 *
 * @par History
 */


#include "basiciomod.h"
#include "modules/basicio-tbl.h"
#include "../../clib/logfacility.h"


namespace Pol
{
namespace Bscript
{
using namespace Module;
template <>
TmplExecutorModule<BasicIoExecutorModule>::FunctionTable
    TmplExecutorModule<BasicIoExecutorModule>::function_table = {
        {"print", &BasicIoExecutorModule::print}};

}  // namespace Bscript

namespace Module
{
BasicIoExecutorModule::BasicIoExecutorModule( Bscript::Executor& exec )
    : Bscript::TmplExecutorModule<BasicIoExecutorModule>( "BasicIo", exec )
{
}

Bscript::BObjectImp* BasicIoExecutorModule::print()
{
  INFO_PRINT << exec.getParamImp( 0 )->getStringRep() << "\n";
  return new Bscript::UninitObject;
}
}
}
