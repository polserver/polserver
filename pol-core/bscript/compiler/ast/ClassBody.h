#pragma once

#include "bscript/compiler/ast/Statement.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;
// Contains no children when initially constructed in SourceFileProcessor.
// UserFunctionVisitor will add children once referenced.
class ClassBody : public Statement
{
public:
  ClassBody( const SourceLocation& );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( std::string& ) const override;
};

}  // namespace Pol::Bscript::Compiler
