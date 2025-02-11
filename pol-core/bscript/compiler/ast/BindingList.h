#pragma once

#include "bscript/compiler/ast/Node.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;
class Expression;
class Variable;

class BindingList : public Node
{
public:
  BindingList( const SourceLocation&, std::vector<std::unique_ptr<Node>> bindings,
               bool index_binding );

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;

  const bool index_binding;
};

}  // namespace Pol::Bscript::Compiler
