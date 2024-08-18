#ifndef POLSERVER_RETURNSTATEMENT_H
#define POLSERVER_RETURNSTATEMENT_H

#include "bscript/compiler/ast/Statement.h"

namespace Pol::Bscript::Compiler
{
class Expression;

class ReturnStatement : public Statement
{
public:
  // `expression` is nullable (inside constructor functions)
  ReturnStatement( const SourceLocation& source_location, std::unique_ptr<Expression> expression );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( std::string& ) const override;
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_RETURNSTATEMENT_H
