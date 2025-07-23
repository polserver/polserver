#pragma once

#include "bscript/compiler/ast/NodeVisitor.h"

#include <string>

namespace Pol::Bscript::Compiler
{
class AbstractSyntaxTreeStringGenerator : public NodeVisitor
{
public:
  AbstractSyntaxTreeStringGenerator();

  void visit_children( Node& parent ) override;

  const std::string& tree() const { return _tree; }

private:
  std::string _tree;
};

}  // namespace Pol::Bscript::Compiler
