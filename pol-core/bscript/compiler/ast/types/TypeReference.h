#pragma once

#include "bscript/compiler/ast/types/TypeNode.h"

namespace Pol::Bscript::Compiler
{
class TypeArgumentList;
class TypeReference : public TypeNode
{
public:
  TypeReference( const SourceLocation&, std::string type_name );
  TypeReference( const SourceLocation&, std::string type_name,
                 std::unique_ptr<TypeArgumentList> type_arguments );

  void describe_to( std::string& ) const override;
  const std::string type_name;
};
}  // namespace Pol::Bscript::Compiler
