#pragma once

#include "bscript/compiler/ast/types/TypeNode.h"

namespace Pol::Bscript::Compiler
{
class TypeReference : public TypeNode
{
public:
  TypeReference( const SourceLocation&, std::string type_name );

  void describe_to( std::string& ) const override;
  const std::string type_name;
};
}  // namespace Pol::Bscript::Compiler
