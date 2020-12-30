#ifndef POLSERVER_EXITSTATEMENT_H
#define POLSERVER_EXITSTATEMENT_H

#include "bscript/compiler/ast/Statement.h"

namespace Pol::Bscript::Compiler
{
class ExitStatement : public Statement
{
public:
  explicit ExitStatement( const SourceLocation& );

  void accept( NodeVisitor& ) override;
  void describe_to( fmt::Writer& ) const override;
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_EXITSTATEMENT_H
