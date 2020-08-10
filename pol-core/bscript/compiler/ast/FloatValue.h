#ifndef POLSERVER_FLOATVALUE_H
#define POLSERVER_FLOATVALUE_H

#include "Value.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;

class FloatValue : public Value
{
public:
  FloatValue( const SourceLocation&, double value );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( fmt::Writer& ) const override;

  const double value;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_FLOATVALUE_H
