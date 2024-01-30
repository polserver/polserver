#ifndef POLSERVER_STRINGVALUE_H
#define POLSERVER_STRINGVALUE_H

#include "bscript/compiler/ast/Value.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;

class StringValue : public Value
{
public:
  StringValue( const SourceLocation& source_location, std::string value );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( std::string& ) const override;
  void describe_to( picojson::object& ) const override;
  std::string type() const override;

  const std::string value;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_STRINGVALUE_H
