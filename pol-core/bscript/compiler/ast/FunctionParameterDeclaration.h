#ifndef POLSERVER_FUNCTIONPARAMETERDECLARATION_H
#define POLSERVER_FUNCTIONPARAMETERDECLARATION_H

#include "bscript/compiler/ast/Node.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;
class Expression;

class FunctionParameterDeclaration : public Node
{
public:
  FunctionParameterDeclaration( const SourceLocation& source_location, std::string name, bool byref,
                                bool unused, std::unique_ptr<Expression> default_value );
  FunctionParameterDeclaration( const SourceLocation& source_location, std::string name, bool byref,
                                bool unused );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( std::string& ) const override;

  Expression* default_value();

  const std::string name;
  const bool byref;
  const bool unused;
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_FUNCTIONPARAMETERDECLARATION_H
