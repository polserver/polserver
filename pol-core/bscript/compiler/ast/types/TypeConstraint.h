#pragma once

#include "bscript/compiler/ast/Node.h"

namespace Pol::Bscript::Compiler
{
class TypeNode;

class TypeConstraint : public Node
{
public:
  TypeConstraint( const SourceLocation&, std::unique_ptr<TypeNode> base_type );

  void describe_to( std::string& ) const override;
  void accept( NodeVisitor& ) override;
};
}  // namespace Pol::Bscript::Compiler
