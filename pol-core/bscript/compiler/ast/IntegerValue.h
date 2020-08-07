#ifndef POLSERVER_INTEGERVALUE_H
#define POLSERVER_INTEGERVALUE_H

#include "Value.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;

class IntegerValue : public Value
{
public:
  IntegerValue( const SourceLocation&, int value );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( fmt::Writer& ) const override;

  const int value;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_INTEGERVALUE_H
