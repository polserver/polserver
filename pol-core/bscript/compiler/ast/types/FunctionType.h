#pragma once

#include "bscript/compiler/ast/types/TypeNode.h"

namespace Pol::Bscript::Compiler
{
class ParameterList;

class FunctionType : public TypeNode
{
public:
  FunctionType( const SourceLocation&, std::unique_ptr<ParameterList> parameters,
                std::unique_ptr<TypeNode> type );

  void describe_to( std::string& ) const override;
};
}  // namespace Pol::Bscript::Compiler
