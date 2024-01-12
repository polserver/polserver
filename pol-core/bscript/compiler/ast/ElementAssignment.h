#ifndef POLSERVER_ELEMENTASSIGNMENT_H
#define POLSERVER_ELEMENTASSIGNMENT_H

#include "bscript/compiler/ast/Expression.h"

namespace Pol::Bscript::Compiler
{
class ElementIndexes;

class ElementAssignment : public Expression
{
public:
  ElementAssignment( const SourceLocation&, bool consume, std::unique_ptr<Expression> entity,
                     std::unique_ptr<ElementIndexes>, std::unique_ptr<Expression> rhs );

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;

  const bool consume;
  ElementIndexes& indexes();

  std::unique_ptr<Expression> take_entity();
  std::unique_ptr<ElementIndexes> take_indexes();
  std::unique_ptr<Expression> take_rhs();
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_ELEMENTASSIGNMENT_H
