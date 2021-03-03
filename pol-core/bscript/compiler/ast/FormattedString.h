#ifndef POLSERVER_FORMATTEDSTRING_H
#define POLSERVER_FORMATTEDSTRING_H

#include "compiler/ast/Expression.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;
class StringValue;

class FormattedString : public Expression
{
public:
  FormattedString( const SourceLocation& source_location,
                      std::unique_ptr<Expression> expression, std::unique_ptr<StringValue> format );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( fmt::Writer& ) const override;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_INTERPOLATEDSTRING_H
