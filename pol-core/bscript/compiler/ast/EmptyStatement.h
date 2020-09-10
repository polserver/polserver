#ifndef POLSERVER_EMPTYSTATEMENT_H
#define POLSERVER_EMPTYSTATEMENT_H

#include "compiler/ast/Statement.h"

namespace Pol::Bscript::Compiler
{
class EmptyStatement : public Statement
{
public:
  EmptyStatement( const SourceLocation& );

  void accept( NodeVisitor& ) override;
  void describe_to( fmt::Writer& ) const override;
};

}  // namespace Pol::Bscript::Compiler
#endif  // POLSERVER_EMPTYSTATEMENT_H
