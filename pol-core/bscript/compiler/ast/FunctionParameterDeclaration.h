#ifndef POLSERVER_FUNCTIONPARAMETERDECLARATION_H
#define POLSERVER_FUNCTIONPARAMETERDECLARATION_H

#include "Node.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;
class Expression;

class FunctionParameterDeclaration : public Node
{
public:
  FunctionParameterDeclaration( const SourceLocation& source_location, std::string name, bool byref,
                        bool unused );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( fmt::Writer& ) const override;

  const std::string name;
  const bool byref;
  const bool unused;
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_FUNCTIONPARAMETERDECLARATION_H
