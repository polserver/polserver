#ifndef POLSERVER_PROGRAMPARAMETERLIST_H
#define POLSERVER_PROGRAMPARAMETERLIST_H

#include "bscript/compiler/ast/Node.h"

namespace Pol::Bscript::Compiler
{
class ProgramParameterDeclaration;

class ProgramParameterList : public Node
{
public:
  ProgramParameterList( const SourceLocation&,
                        std::vector<std::unique_ptr<ProgramParameterDeclaration>> parameters );

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;
  std::string type() const override;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_PROGRAMPARAMETERLIST_H
