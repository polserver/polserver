#ifndef POLSERVER_NODEVISITOR_H
#define POLSERVER_NODEVISITOR_H

namespace Pol
{
namespace Bscript
{
namespace Compiler
{
class NodeVisitor
{
public:
  virtual ~NodeVisitor() = default;
};

}  // namespace Compiler
}  // namespace Bscript
}  // namespace Pol


#endif  // POLSERVER_NODEVISITOR_H
