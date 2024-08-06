#pragma once

#include "bscript/compiler/ast/Expression.h"

namespace Pol::Bscript::Compiler
{
class SpreadElement : public Expression
{
public:
  SpreadElement( const SourceLocation&, std::unique_ptr<Node> child );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( std::string& ) const override;
};

}  // namespace Pol::Bscript::Compiler
