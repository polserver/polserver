/** @file
 *
 * @par History
 */


#ifndef BSCRIPT_BASICIOMOD_H
#define BSCRIPT_BASICIOMOD_H

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
namespace Node
{
template <typename PolModule>
class NodeModuleWrap;
}
namespace Module
{
class BasicIoExecutorModule : public Bscript::TmplExecutorModule<BasicIoExecutorModule>
{
public:
  Bscript::BObjectImp* print();

  BasicIoExecutorModule( Bscript::Executor& exec );

  friend class Node::NodeModuleWrap<BasicIoExecutorModule>;
};
}

//namespace Bscript 
//{
//  template <>
//  extern const char* TmplExecutorModule<Pol::Module::BasicIoExecutorModule>::modname;
//
//  template <>
//  extern TmplExecutorModule<Pol::Module::BasicIoExecutorModule>::FunctionTable TmplExecutorModule<Pol::Module::BasicIoExecutorModule>::function_table;
//}
}
#endif
