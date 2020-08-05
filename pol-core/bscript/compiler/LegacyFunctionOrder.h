#ifndef POLSERVER_LEGACYFUNCTIONORDER_H
#define POLSERVER_LEGACYFUNCTIONORDER_H

#include <string>
#include <vector>

namespace Pol
{
namespace Bscript
{
namespace Compiler
{
struct LegacyFunctionOrder
{
  std::vector<std::string> modulefunc_emit_order;
  std::vector<std::string> userfunc_emit_order;
};

}  // namespace Compiler
}  // namespace Bscript
}  // namespace Pol

#endif  // POLSERVER_LEGACYFUNCTIONORDER_H
