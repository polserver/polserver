#pragma once

#include "bscript/compiler/ast/Expression.h"

namespace Pol::Bscript::Compiler
{
class SpreadElement : public Expression
{
public:
  SpreadElement( const SourceLocation&, std::unique_ptr<Node> child, bool spread_into );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( std::string& ) const override;

  // If false, the instruction generated from this node will tell the executor
  // to create a BSpread element for `child`. If true, the instruction will tell
  // the executor to spread the elements of `child` into the value on the
  // ValueStack.
  const bool spread_into;
};

}  // namespace Pol::Bscript::Compiler
