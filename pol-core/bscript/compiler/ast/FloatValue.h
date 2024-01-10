#ifndef POLSERVER_FLOATVALUE_H
#define POLSERVER_FLOATVALUE_H

#include "bscript/compiler/ast/Value.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;

class FloatValue : public Value
{
public:
  FloatValue( const SourceLocation&, double value );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( std::string& ) const override;

  const double value;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_FLOATVALUE_H
