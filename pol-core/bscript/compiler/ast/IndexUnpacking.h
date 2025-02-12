#pragma once

#include "bscript/compiler/ast/Node.h"
#include "bscript/compiler/model/ScopableName.h"

namespace Pol::Bscript::Compiler
{
class Expression;
class NodeVisitor;
class Variable;

class IndexUnpacking : public Node
{
public:
  IndexUnpacking( const SourceLocation&, std::string scope, std::string name, bool rest );

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;

  const ScopableName scoped_name;
  const bool rest;
  std::shared_ptr<Variable> variable;
};

}  // namespace Pol::Bscript::Compiler
