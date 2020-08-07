#ifndef POLSERVER_OPTIMIZER_H
#define POLSERVER_OPTIMIZER_H

#include "compiler/ast/NodeVisitor.h"

#include <memory>

namespace Pol::Bscript::Compiler
{
class Report;
class CompilerWorkspace;

class Optimizer : public NodeVisitor
{
public:
  explicit Optimizer( Report& );

  void optimize( CompilerWorkspace& );

private:
  Report& report;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_OPTIMIZER_H
