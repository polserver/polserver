#ifndef POLSERVER_PROGRAMPARAMETERDECLARATION_H
#define POLSERVER_PROGRAMPARAMETERDECLARATION_H

#include "bscript/compiler/ast/Node.h"

namespace Pol::Bscript::Compiler
{
class ProgramParameterDeclaration : public Node
{
public:
  ProgramParameterDeclaration( const SourceLocation&, std::string name, bool unused );

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;

  const std::string name;
  const bool unused;
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_PROGRAMPARAMETERDECLARATION_H
