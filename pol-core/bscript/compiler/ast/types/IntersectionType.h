#pragma once

#include "bscript/compiler/ast/types/TypeNode.h"

namespace Pol::Bscript::Compiler
{
class IntersectionType : public TypeNode
{
public:
  IntersectionType( const SourceLocation&, std::unique_ptr<TypeNode> lhs,
                    std::unique_ptr<TypeNode> rhs );

  void describe_to( std::string& ) const override;
};
}  // namespace Pol::Bscript::Compiler
