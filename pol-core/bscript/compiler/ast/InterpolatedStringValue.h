#ifndef POLSERVER_INTERPOLATEDSTRINGVALUE_H
#define POLSERVER_INTERPOLATEDSTRINGVALUE_H

#include "compiler/ast/Value.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;

class InterpolatedStringValue : public Value
{
public:
  InterpolatedStringValue( const SourceLocation& source_location, std::string value );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( fmt::Writer& ) const override;

  const std::string value;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_INTERPOLATEDSTRINGVALUE_H
