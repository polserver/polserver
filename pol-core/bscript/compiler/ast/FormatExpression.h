#ifndef POLSERVER_FORMATEXPRESSION_H
#define POLSERVER_FORMATEXPRESSION_H

#include "compiler/ast/Expression.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;
class StringValue;

class FormatExpression : public Expression
{
public:
  FormatExpression( const SourceLocation& source_location, std::unique_ptr<Expression> expression,
                    std::unique_ptr<StringValue> format );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( std::string& ) const override;
  std::string type() const override;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_FORMATEXPRESSION_H
