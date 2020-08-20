#ifndef POLSERVER_STRINGVALUE_H
#define POLSERVER_STRINGVALUE_H

#include "compiler/ast/Value.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;

class StringValue : public Value
{
public:
  StringValue( const SourceLocation& source_location, std::string value );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( fmt::Writer& ) const override;

  const std::string value;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_STRINGVALUE_H
