#ifndef POLSERVER_ASSIGNSUBSCRIPT_H
#define POLSERVER_ASSIGNSUBSCRIPT_H

#include "Expression.h"

namespace Pol::Bscript::Compiler
{
class ArrayElementIndexes;
class AssignSubscript : public Expression
{
public:
  AssignSubscript( const SourceLocation&, bool consume, std::unique_ptr<Expression> entity,
                   std::unique_ptr<ArrayElementIndexes> indexes, std::unique_ptr<Expression> rhs );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( fmt::Writer& ) const override;

  const bool consume;
  ArrayElementIndexes& indexes();

  std::unique_ptr<Expression> take_entity();
  std::unique_ptr<ArrayElementIndexes> take_indexes();
  std::unique_ptr<Expression> take_rhs();
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_ASSIGNSUBSCRIPT_H
