#ifndef POLSERVER_RETURNSTATEMENT_H
#define POLSERVER_RETURNSTATEMENT_H

#include "Statement.h"

namespace Pol::Bscript::Compiler
{
class Expression;

class ReturnStatement : public Statement
{
public:
  ReturnStatement( const SourceLocation& source_location, std::unique_ptr<Expression> expression );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( fmt::Writer& ) const override;
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_RETURNSTATEMENT_H
