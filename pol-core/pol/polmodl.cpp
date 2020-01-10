/** @file
 *
 * @par History
 */


#include "polmodl.h"

namespace Pol
{
namespace Core
{
PolModule::PolModule( const char* moduleName, Bscript::Executor& iExec ) : Bscript::ExecutorModule(moduleName, iExec)
{
}
}  // namespace Core
}  // namespace Pol
