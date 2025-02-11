#pragma once

#include "bscript/compiler/ast/Statement.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;
class Expression;
class Variable;

class UnpackingStatement : public Statement
{
public:
  UnpackingStatement( const SourceLocation&, std::unique_ptr<Node> unpackings,
                      std::unique_ptr<Expression> initializer );

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;
};

}  // namespace Pol::Bscript::Compiler
