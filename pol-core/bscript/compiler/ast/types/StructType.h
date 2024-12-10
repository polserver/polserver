#pragma once

#include "bscript/compiler/ast/types/TypeNode.h"

namespace Pol::Bscript::Compiler
{
class StructType : public TypeNode
{
public:
  StructType( const SourceLocation& );
  StructType( const SourceLocation&, NodeVector members );

  void describe_to( std::string& ) const override;
};

}  // namespace Pol::Bscript::Compiler
