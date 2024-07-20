#pragma once

#include "bscript/compiler/ast/Value.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;

class FunctionExpression : public Value
{
public:
  FunctionExpression( const SourceLocation&, bool value );

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;

  const bool value;
};

}  // namespace Pol::Bscript::Compiler
