#ifndef POLSERVER_STRUCTMEMBERINITIALIZER_H
#define POLSERVER_STRUCTMEMBERINITIALIZER_H

#include "bscript/compiler/ast/Node.h"

namespace Pol::Bscript::Compiler
{
class Expression;

class StructMemberInitializer : public Node
{
public:
  StructMemberInitializer( const SourceLocation&, std::string name );
  StructMemberInitializer( const SourceLocation&, std::string name, std::unique_ptr<Expression> );

  void accept( NodeVisitor& ) override;
  void describe_to( fmt::Writer& ) const override;

  const std::string name;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_STRUCTMEMBERINITIALIZER_H
