/** @file
 *
 * @par History
 * - 2006/10/28 Shinigami: GCC 4.1.1 fix - invalid use of constructor as a template
 */


#ifndef POL_POLMODL_H
#define POL_POLMODL_H

#include "../bscript/execmodl.h"

namespace Pol
{
namespace Bscript
{
class Executor;
}  // namespace Bscript
}  // namespace Pol

namespace Pol
{
namespace Core
{

class PolModule : public Bscript::ExecutorModule
{
public:
  PolModule( const char* moduleName, Bscript::Executor& iExec );
};


template <class T>
class TmplPolModule : public PolModule
{
  //protected:
//  TmplPolModule( Executor& exec );
};

}  // namespace Core
}  // namespace Pol

#endif
