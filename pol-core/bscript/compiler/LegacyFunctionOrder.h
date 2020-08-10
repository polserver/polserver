#ifndef POLSERVER_LEGACYFUNCTIONORDER_H
#define POLSERVER_LEGACYFUNCTIONORDER_H

#include <string>
#include <vector>

namespace Pol::Bscript::Compiler
{
struct LegacyFunctionOrder
{
  std::vector<std::string> modulefunc_emit_order;
  std::vector<std::string> userfunc_emit_order;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_LEGACYFUNCTIONORDER_H
