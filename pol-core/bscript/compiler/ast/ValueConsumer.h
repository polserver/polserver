#ifndef POLSERVER_VALUECONSUMER_H
#define POLSERVER_VALUECONSUMER_H

#include "compiler/ast/Statement.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;

class ValueConsumer : public Statement
{
public:
  ValueConsumer( const SourceLocation&, std::unique_ptr<Statement> child );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( fmt::Writer& ) const override;
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_VALUECONSUMER_H
