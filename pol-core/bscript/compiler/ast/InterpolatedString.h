#ifndef POLSERVER_INTERPOLATEDSTRING_H
#define POLSERVER_INTERPOLATEDSTRING_H

#include "compiler/ast/Expression.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;

class InterpolatedString : public Expression
{
public:
  InterpolatedString( const SourceLocation& source_location,
                      std::vector<std::unique_ptr<Expression>> );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( fmt::Writer& ) const override;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_INTERPOLATEDSTRING_H
