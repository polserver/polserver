#ifndef POLSERVER_ENUMDECLARATION_H
#define POLSERVER_ENUMDECLARATION_H

#include "Statement.h"

namespace Pol::Bscript::Compiler
{
class Expression;

class EnumDeclaration : public Statement
{
public:
  EnumDeclaration( const SourceLocation&, std::string identifier, std::vector<std::string> names,
                   std::vector<std::unique_ptr<Expression>> expressions );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( fmt::Writer& ) const override;

  const std::string identifier;
  const std::vector<std::string> names;
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_ENUMDECLARATION_H
