#ifndef POLSERVER_INTEGERVALUE_H
#define POLSERVER_INTEGERVALUE_H

#include "bscript/compiler/ast/Value.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;

class IntegerValue : public Value
{
public:
  IntegerValue( const SourceLocation&, int value );

  void accept( NodeVisitor& ) override;
  void describe_to( fmt::Writer& ) const override;

  const int value;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_INTEGERVALUE_H
