/** @file
 *
 * @par History
 */


#include "basiciomod.h"
#include "modules/basicio-tbl.h"
#include "../../clib/logfacility.h"


namespace Pol
{
// namespace Bscript
// {
// using namespace Module;
// template <>
// TmplExecutorModule<BasicIoExecutorModule>::FunctionTable
//     TmplExecutorModule<BasicIoExecutorModule>::function_table = {
//         {"print", &BasicIoExecutorModule::print, 1}};

// template <>
// const char* TmplExecutorModule<BasicIoExecutorModule>::modname = "BasicIo";
// }  // namespace Bscript

namespace Module
{
BasicIoExecutorModule::BasicIoExecutorModule( Bscript::Executor& exec )
    : Bscript::TmplExecutorModule<BasicIoExecutorModule>( exec )
{
}

Bscript::BObjectImp* BasicIoExecutorModule::print()
{
  INFO_PRINT << exec.getParamImp( 0 )->getStringRep() << "\n";
  return new Bscript::UninitObject;
}
}
}
