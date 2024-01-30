#ifndef POLSERVER_FUNCTIONREFERENCE_H
#define POLSERVER_FUNCTIONREFERENCE_H

#include "bscript/compiler/ast/Value.h"

namespace Pol::Bscript::Compiler
{
class FunctionLink;

class FunctionReference : public Value
{
public:
  FunctionReference( const SourceLocation&, std::string name, std::shared_ptr<FunctionLink> );

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;
  std::string type() const override;

  const std::string name;
  const std::shared_ptr<FunctionLink> function_link;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_FUNCTIONREFERENCE_H
