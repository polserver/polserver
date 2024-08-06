#pragma once

#include "bscript/compiler/ast/Statement.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;

class ClassBody : public Statement
{
public:
  ClassBody( const SourceLocation&, NodeVector statements );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( std::string& ) const override;
};

}  // namespace Pol::Bscript::Compiler
