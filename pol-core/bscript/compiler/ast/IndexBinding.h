#pragma once

#include "bscript/compiler/ast/Node.h"
#include "clib/rawtypes.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;
class ElementIndexes;

class IndexBinding : public Node
{
public:
  IndexBinding( const SourceLocation&, std::unique_ptr<ElementIndexes> indices,
                std::vector<std::unique_ptr<Node>> bindings );

  u8 binding_count() const;

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;

  ElementIndexes& indexes();
  std::vector<std::reference_wrapper<Node>> bindings();


  // Filled by SemanticAnalyzer. If <128, then a valid value.
  u8 rest_index = 0xFF;
};

}  // namespace Pol::Bscript::Compiler
