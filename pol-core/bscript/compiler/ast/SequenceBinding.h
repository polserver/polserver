#pragma once

#include "bscript/compiler/ast/Node.h"
#include "clib/rawtypes.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;
class Expression;
class Variable;

class SequenceBinding : public Node
{
public:
  SequenceBinding( const SourceLocation&, std::vector<std::unique_ptr<Node>> bindings );

  u8 binding_count() const;

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;

  // Filled by SemanticAnalyzer. If <128, then a valid value.
  u8 rest_index = 0xFF;
};

}  // namespace Pol::Bscript::Compiler
