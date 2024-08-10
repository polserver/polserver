#pragma once

#include "bscript/compiler/ast/Statement.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;
// Only contains user functions as children, as variable statements are added in
// `top_level_statements` by SourceFileProcessor.
class ClassBody : public Statement
{
public:
  ClassBody( const SourceLocation&, NodeVector statements );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( std::string& ) const override;
};

}  // namespace Pol::Bscript::Compiler
