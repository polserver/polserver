#ifndef POLSERVER_PROGRAMPARAMETERDECLARATION_H
#define POLSERVER_PROGRAMPARAMETERDECLARATION_H

#include "Node.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;
class Expression;

class ProgramParameterDeclaration : public Node
{
public:
  ProgramParameterDeclaration( const SourceLocation& source_location, std::string name,
                               bool unused );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( fmt::Writer& ) const override;

  const std::string name;
  const bool unused;
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_PROGRAMPARAMETERDECLARATION_H
