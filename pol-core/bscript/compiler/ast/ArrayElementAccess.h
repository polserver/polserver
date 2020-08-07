#ifndef POLSERVER_ARRAYELEMENTACCESS_H
#define POLSERVER_ARRAYELEMENTACCESS_H

#include "Expression.h"

namespace Pol::Bscript::Compiler
{
class ArrayElementIndexes;

class ArrayElementAccess : public Expression
{
public:
  ArrayElementAccess( const SourceLocation&, std::unique_ptr<Expression> lhs,
                      std::unique_ptr<ArrayElementIndexes> );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( fmt::Writer& ) const override;

  ArrayElementIndexes& indexes();

  std::unique_ptr<Expression> take_entity();
  std::unique_ptr<ArrayElementIndexes> take_indexes();
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_ARRAYELEMENTACCESS_H
