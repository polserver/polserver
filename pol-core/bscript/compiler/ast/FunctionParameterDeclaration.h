#ifndef POLSERVER_FUNCTIONPARAMETERDECLARATION_H
#define POLSERVER_FUNCTIONPARAMETERDECLARATION_H

#include "bscript/compiler/ast/Node.h"
#include "bscript/compiler/model/ScopableName.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;
class Expression;

class FunctionParameterDeclaration : public Node
{
public:
  FunctionParameterDeclaration( const SourceLocation& source_location, ScopableName name,
                                bool byref, bool unused, bool uninit_default, bool rest,
                                std::unique_ptr<Expression> default_value );
  FunctionParameterDeclaration( const SourceLocation& source_location, ScopableName name,
                                bool byref, bool unused, bool uninit_default, bool rest );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( std::string& ) const override;

  Expression* default_value();

  const ScopableName name;
  const bool byref;
  const bool unused;
  const bool uninit_default;
  const bool rest;
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_FUNCTIONPARAMETERDECLARATION_H
