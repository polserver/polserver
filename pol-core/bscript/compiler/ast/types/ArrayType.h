#pragma once

#include "bscript/compiler/ast/types/TypeNode.h"

namespace Pol::Bscript::Compiler
{
class ArrayType : public TypeNode
{
public:
  ArrayType( const SourceLocation& );
  ArrayType( const SourceLocation&, std::unique_ptr<TypeNode> element_type );

  void describe_to( std::string& ) const override;
};
}  // namespace Pol::Bscript::Compiler
