#pragma once

#include "bscript/compiler/ast/Value.h"

namespace Pol::Bscript::Compiler
{
class FunctionLink;
class NodeVisitor;

class FunctionExpression : public Value
{
public:
  FunctionExpression( const SourceLocation&, std::shared_ptr<FunctionLink> );

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;

  const std::shared_ptr<FunctionLink> function_link;
};

}  // namespace Pol::Bscript::Compiler
