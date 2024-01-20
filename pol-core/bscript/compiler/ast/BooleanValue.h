#ifndef POLSERVER_BOOLEANVALUE_H
#define POLSERVER_BOOLEANVALUE_H

#include "bscript/compiler/ast/Value.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;

class BooleanValue : public Value
{
public:
  BooleanValue( const SourceLocation&, bool value );

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;

  const bool value;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_BOOLEANVALUE_H
