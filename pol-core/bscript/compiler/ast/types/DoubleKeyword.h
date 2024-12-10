#pragma once

#include "bscript/compiler/ast/types/TypeNode.h"

namespace Pol::Bscript::Compiler
{
class DoubleKeyword : public TypeNode
{
public:
  DoubleKeyword( const SourceLocation& );

  void describe_to( std::string& ) const override;
};
}  // namespace Pol::Bscript::Compiler
