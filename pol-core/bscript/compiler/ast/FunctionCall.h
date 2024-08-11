#ifndef POLSERVER_FUNCTIONCALL_H
#define POLSERVER_FUNCTIONCALL_H

#include "bscript/compiler/ast/Expression.h"
#include "bscript/compiler/model/ScopableName.h"

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
  // A compile-time, statically linked function call
  FunctionCall( const SourceLocation&, std::string calling_scope, ScopableName scoped_name,
                std::vector<std::unique_ptr<Argument>> arguments );
  // A run-time, dynamically executed expression-as-callee function call
  FunctionCall( const SourceLocation&, std::string calling_scope, std::unique_ptr<Node> callee,
                std::vector<std::unique_ptr<Argument>> arguments );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( std::string& ) const override;

  std::vector<std::unique_ptr<Argument>> take_arguments();
  [[nodiscard]] std::vector<std::reference_wrapper<FunctionParameterDeclaration>> parameters()
      const;

  const std::shared_ptr<FunctionLink> function_link;

  // Used in FunctionResolver. If the function does not exist under
  // `method_name`, check `calling_scope::method_name` (if calling scope
  // exists).
  const std::string calling_scope;

  // nullptr in expression-as-callees function calls
  std::unique_ptr<ScopableName> scoped_name;

  // Eg. `foo()` or `Animal::foo()`
  std::string maybe_scoped_name() const;
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_FUNCTIONCALL_H
