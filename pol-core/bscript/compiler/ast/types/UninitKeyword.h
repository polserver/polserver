#pragma once

#include "bscript/compiler/ast/types/TypeNode.h"

namespace Pol::Bscript::Compiler
{
class UninitKeyword : public TypeNode
{
public:
  UninitKeyword( const SourceLocation& );

  void describe_to( std::string& ) const override;
};
}  // namespace Pol::Bscript::Compiler
