#ifndef POLSERVER_FUNCTIONCALL_H
#define POLSERVER_FUNCTIONCALL_H

#include "compiler/ast/Expression.h"

namespace Pol::Bscript::Compiler
{
class Argument;
class FunctionLink;
class ModuleFunctionDeclaration;
class FunctionParameterDeclaration;
class UserFunction;

class FunctionCall : public Expression
{
public:
  FunctionCall( const SourceLocation&, std::string scope, std::string name,
                std::vector<std::unique_ptr<Argument>> arguments );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( fmt::Writer& ) const override;

  std::vector<std::unique_ptr<Argument>> take_arguments();
  [[nodiscard]] std::vector<std::reference_wrapper<FunctionParameterDeclaration>> parameters()
      const;

  const std::shared_ptr<FunctionLink> function_link;

  const std::string scope;
  const std::string method_name;
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_FUNCTIONCALL_H
