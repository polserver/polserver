#pragma once

#include "bscript/compiler/ast/types/TypeNode.h"

namespace Pol::Bscript::Compiler
{
class TupleType : public TypeNode
{
public:
  TupleType( const SourceLocation&, NodeVector elements );

  void describe_to( std::string& ) const override;
};
}  // namespace Pol::Bscript::Compiler
