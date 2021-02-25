#ifndef POLSERVER_ELEMENTACCESS_H
#define POLSERVER_ELEMENTACCESS_H

#include "bscript/compiler/ast/Expression.h"

namespace Pol::Bscript::Compiler
{
class ElementIndexes;

class ElementAccess : public Expression
{
public:
  ElementAccess( const SourceLocation&, std::unique_ptr<Expression> lhs,
                      std::unique_ptr<ElementIndexes> );

  void accept( NodeVisitor& ) override;
  void describe_to( fmt::Writer& ) const override;

  ElementIndexes& indexes();

  std::unique_ptr<Expression> take_entity();
  std::unique_ptr<ElementIndexes> take_indexes();
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_ELEMENTACCESS_H
