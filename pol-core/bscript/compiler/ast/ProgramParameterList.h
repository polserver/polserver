#ifndef POLSERVER_PROGRAMPARAMETERLIST_H
#define POLSERVER_PROGRAMPARAMETERLIST_H

#include "Node.h"

namespace Pol::Bscript::Compiler
{
class ProgramParameterDeclaration;

class ProgramParameterList : public Node
{
public:
  ProgramParameterList( const SourceLocation&,
                 std::vector<std::unique_ptr<ProgramParameterDeclaration>> parameters );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( fmt::Writer& ) const override;
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_PROGRAMPARAMETERLIST_H
