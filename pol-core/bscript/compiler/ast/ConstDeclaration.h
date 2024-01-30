#ifndef POLSERVER_CONSTDECLARATION_H
#define POLSERVER_CONSTDECLARATION_H

#include "bscript/compiler/ast/Node.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;
class Expression;
class SourceLocation;

class ConstDeclaration : public Node
{
public:
  const std::string identifier;

  ConstDeclaration( const SourceLocation&, std::string identifier, std::unique_ptr<Expression>,
                    bool ignore_overwrite_attempt = false );


  Expression& expression();

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;
  std::string type() const override;

  const bool ignore_overwrite_attempt;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_CONSTDECLARATION_H
