#ifndef POLSERVER_VALUECONSUMER_H
#define POLSERVER_VALUECONSUMER_H

#include "compiler/ast/Expression.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;

class ValueConsumer : public Expression
{
public:
  ValueConsumer( const SourceLocation&, std::unique_ptr<Expression> child );

  void accept( NodeVisitor& ) override;
  void describe_to( fmt::Writer& ) const override;
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_VALUECONSUMER_H
